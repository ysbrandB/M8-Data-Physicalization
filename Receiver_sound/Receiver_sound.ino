

//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'


//ESP Now connectivity
#include<ESP8266WiFi.h>
#include<espnow.h>


#include<SoftwareSerial.h>
SoftwareSerial softSerial(1, 3);

#include <DFPlay.h>
DFPlay dfplay;

#define WIFI_CHANNEL    1

//BC:FF:4D:81:7D:DD
uint8_t receiverAddress[] = {0xBC, 0xFF, 0x4D, 0x81, 0x7D, 0xDD};   // CONTROLLER
enum nodeStates {
  ALL,
  SMOKE,
  BUILDINGS,
  SOUND,
  LEDs
};

//DEFINES which commands the esp is listening to: by default it listens to ALL and itself(whoAmI).
//Change this to the purpose of your node! check the possibilities in the enum above!
enum nodeStates whoAmI = SOUND;
//defines how often your actuate function gets called! 1000=once every second.
int actuateInterval = 1000;

//interval for sending
int interval = 3000;
//which node is selected by the controller
enum nodeStates selected = ALL;

struct __attribute__((packed)) dataPacket {
  enum nodeStates whoAmI;
  int year;
  enum nodeStates selected;
};


int startTime = 0;
int actuateTime = 0;
int year = 2021;

long timeNow = 0;

void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    Serial.println("Data sent successfully");
  } else {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  dataPacket packet;

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);

  Serial.println();
  Serial.print("Received data from: ");
  Serial.println(macStr);

  memcpy(&packet, data, sizeof(packet));

  Serial.print("whoIsSender: ");
  Serial.println(packet.whoAmI);
  Serial.print("packet year: ");
  Serial.println(packet.year);
  Serial.print("packet selected: ");
  Serial.println(packet.selected);
  year = packet.year;
  selected=packet.selected;
}

void setup() {
  //softSerial.begin(9600);
  //dfplay.begin(softSerial);

   Serial1.begin(9600);
   dfplay.begin(Serial1);      // Prepares DFPlay for execution
   dfplay.setVolume(5);     // Sets volume level to 10 (valid range = 0 to 30)
   Selection SDcard = {2,0,0,0,0}; // Selects all tracks on the SD card
   dfplay.play(SDcard);  
  
  pinMode(2, OUTPUT);
  pinMode(16, OUTPUT);
  Serial.begin(115200);     // initialize serial port

  startTime = millis();
  actuateTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    year = 0;
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(transmissionComplete);         // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);               // this function will get called whenever we receive data
  esp_now_add_peer(receiverAddress, ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  Serial.println("ESP-NOW Initialized.");

    

}

void loop() {
  if (millis() - startTime >= interval) {
    startTime = millis();
    dataPacket packet;

    packet.selected = selected;
    packet.year = year;
    packet.whoAmI = whoAmI;
    esp_now_send(receiverAddress, (uint8_t *) &packet, sizeof(packet));
  }
  if (millis() - actuateTime >= actuateInterval) {
    actuateTime = millis();
    if (selected == whoAmI || selected == ALL) {
      doActuate();
    }
  }
  timeNow = millis();
  dfplay.manageDevice();  
}

void doActuate() {
  //float frequency = -2.500722276023467e-12 * pow(year,7) + 1.6488853026647364e-08 * pow(year,6) + -2.4429201430871233e-05 * pow(year,5) + -0.043458198837792957 * pow(year,4) + 87.00640953800078 * pow(year,3) + 198455.11340918406 * pow(year,2) + -538435092.0758607 * pow(year,1) + 328869939032.7708 * pow(year,0);

  float minimum = 1.96; //2021
  float maximum = 6.75; //2016
  
  if (year >= 1998 && year <= 2021){
      float frequency = 1.1970171602529251e-120 * pow(year,40) + -9.61857722999715e-118 * pow(year,39) + 
      -4.002086824356516e-114 * pow(year,38) + -6.9589728888533e-111 * pow(year,37) + -7.063867437640753e-108 * 
      pow(year,36) + 1.7260531734570308e-105 * pow(year,35) + 2.996418127145569e-101 * pow(year,34) + 
      9.315402022704068e-98 * pow(year,33) + 2.0694717093151884e-94 * pow(year,32) + 3.721561115700868e-91 * 
      pow(year,31) + 5.326883607630093e-88 * pow(year,30) + 4.800381684977409e-85 * pow(year,29) + 
      -3.2268941823612903e-82 * pow(year,28) + -3.0286413988947407e-78 * pow(year,27) + -9.772700386303117e-75 * 
      pow(year,26) + -2.4014176645915433e-71 * pow(year,25) + -5.00939596070257e-68 * pow(year,24) + 
      -9.036172525452867e-65 * pow(year,23) + -1.3643185697997649e-61 * pow(year,22) + -1.473542260660355e-58 * 
      pow(year,21) + -6.188631598060209e-57 * pow(year,20) + 5.753525998119162e-52 * pow(year,19) + 
      2.2004251709973074e-48 * pow(year,18) + 5.907560758560234e-45 * pow(year,17) + 1.3306444235405137e-41 * 
      pow(year,16) + 2.598057881169742e-38 * pow(year,15) + 4.302617432976276e-35 * pow(year,14) + 
      5.487090372274661e-32 * pow(year,13) + 2.6446694881938504e-29 * pow(year,12) + -1.304332427518218e-25 * 
      pow(year,11) + -6.101478729818958e-22 * pow(year,10) + -1.7399880914374773e-18 * pow(year,9) + 
      -3.9455897940707805e-15 * pow(year,8) + -7.215334171730343e-12 * pow(year,7) + -9.444033804801445e-09 * 
      pow(year,6) + -2.4934070352588752e-06 * pow(year,5) + 0.03529821305321723 * pow(year,4) + 142.5484399349725 * 
      pow(year,3) + 332151.22527036007 * pow(year,2) + 321846280.36468387 * pow(year,1) + -1612233220132.9333 * 
      pow(year,0);
  
      

      float seconds = map(frequency, 1.96, 6.75, 20, 2);
      
      digitalWrite(16, !digitalRead(16));
      digitalWrite(2, !digitalRead(2));

      Serial.println(String(year) + ": " + String(frequency) + ", will run every " + String(seconds));

      //if (millis() >= time_now + seconds){
      //  time_now = millis();
     //   Serial.println("*Sound*");
     // }

  }
  else {
    Serial.println("OUTSIDE OF DATASET");
  }

}
