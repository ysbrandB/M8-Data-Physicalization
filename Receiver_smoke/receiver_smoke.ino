//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'


//ESP Now connectivity
#include<ESP8266WiFi.h>
#include<espnow.h>

#define WIFI_CHANNEL    1

#define RELAY 14

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
enum nodeStates whoAmI = SMOKE;
//defines how often your actuate function gets called! 1000=once every second.
int actuateInterval = 3000;

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
int year = 0;


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
  selected = packet.selected;
}

void setup() {
  Serial.begin(115200);     // initialize serial port

  startTime = millis();
  actuateTime = millis();
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

//  pinMode(relay, OUTPUT);
pinMode(RELAY, OUTPUT);
//digitalWrite(RELAY, HIGH);

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

bool Switch = true;
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
//      Switch = !Switch;
    }
  }
}


int smokeTime = 0;
void doActuate() {
//  year = random(1970, 2020);
  year = 1969;
  if (year > 2020) return;
  
  long value = -1.7725037169689857e-120 * pow(year,40) + 2.691922781390831e-117 * pow(year,39) + 6.915587849485481e-114 * pow(year,38) + 7.351942178209149e-111 * pow(year,37) + -2.3046173656774446e-108 * pow(year,36) + -3.0768419146221935e-104 * pow(year,35) + -8.546394975866965e-101 * pow(year,34) + -1.6201824022196218e-97 * pow(year,33) + -2.20419206240899e-94 * pow(year,32) + -1.4160228915850454e-91 * pow(year,31) + 3.3675774870946775e-88 * pow(year,30) + 1.6882527900161895e-84 * pow(year,29) + 4.597919911280918e-81 * pow(year,28) + 9.711766071571068e-78 * pow(year,27) + 1.6704453004982314e-74 * pow(year,26) + 2.199008366025155e-71 * pow(year,25) + 1.3908445097492863e-68 * pow(year,24) + -3.5798630593631726e-65 * pow(year,23) + -1.8378838086785712e-61 * pow(year,22) + -5.247065704730748e-58 * pow(year,21) + -1.1798812837169494e-54 * pow(year,20) + -2.2113285686946442e-51 * pow(year,19) + -3.35892203902774e-48 * pow(year,18) + -3.4107302166127626e-45 * pow(year,17) + 1.035442206875168e-42 * pow(year,16) + 1.7617247645614831e-38 * pow(year,15) + 6.024025869542864e-35 * pow(year,14) + 1.4853625707674114e-31 * pow(year,13) + 2.9541439019403738e-28 * pow(year,12) + 4.646553734953138e-25 * pow(year,11) + 4.615942646555994e-22 * pow(year,10) + -2.7605406395181116e-19 * pow(year,9) + -2.9649720679057393e-15 * pow(year,8) + -9.508570018619388e-12 * pow(year,7) + -2.1075593646903026e-08 * pow(year,6) + -3.225614216839399e-05 * pow(year,5) + -0.015906093258578054 * pow(year,4) + 100.20446827526675 * pow(year,3) + 411729.9704703481 * pow(year,2) + 673447029.6980231 * pow(year,1) + -1643973335264.2969 * pow(year,0);
  int mapValue = map(value, 0, 100, 1000, 5000);
  Serial.print("year: ");
  Serial.println(year);

  Serial.print("value: ");
  Serial.println(value);

  Serial.print("mapValue: ");
  Serial.println(mapValue);

  int smokeInterval = mapValue;
//  int smokeInterval = 3000;
  if (millis() - smokeTime >= smokeInterval) {
    smokeTime = millis(); 
    // Normally Open configuration, send LOW signal to let current flow
    digitalWrite(RELAY, LOW);
    Serial.println("Current Flowing");
   }
//   digitalWrite(RELAY, HIGH);
//   Serial.println("Current not Flowing");
   
   int noSmokeInterval = 5000 - smokeInterval;
   if (millis() - smokeTime >= noSmokeInterval) {
   // Normally Open configuration, send HIGH signal stop current flow
   digitalWrite(RELAY, HIGH);
   Serial.println("Current not Flowing");
   }
    
}
