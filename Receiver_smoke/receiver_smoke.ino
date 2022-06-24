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
digitalWrite(RELAY, HIGH);

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


void doActuate() {
  //PUT CODE HERE FOR YOUR ACTUATOR!

long value = -6.5517440569062585e-133 * pow(year,46) + 6.805656182938649e-130 * pow(year,45) + 2.341550209108304e-126 * pow(year,44) + 3.530974538531393e-123 * pow(year,43) + 2.399729319491314e-120 * pow(year,42) + -4.4609001974984246e-117 * pow(year,41) + -2.218596911241094e-113 * pow(year,40) + -5.646353780878323e-110 * pow(year,39) + -1.0893759951556788e-106 * pow(year,38) + -1.6525228668235932e-103  * pow(year,37) + -1.6953798292162856e-100 * pow(year,36) + 2.2665687148750585e-98 * pow(year,35) + 7.192003056934981e-94 * pow(year,34) + 2.484001292323127e-90 * pow(year,33) + 6.192897978992189e-87 * pow(year,32) + 1.288280438535514e-83 * pow(year,31) + 2.2983391503180903e-80 * pow(year,30) + 3.4066882963115613e-77  * pow(year,29) + 3.5527851557179534e-74 * pow(year,28) + -2.3454949398018666e-72 * pow(year,27) + -1.4811435993296021e-67 * pow(year,26) + -5.393683320108537e-64 * pow(year,25) + -1.4167959051681845e-60 * pow(year,24) + -3.1309793723599106e-57 * pow(year,23) + -6.028754295830673e-54 * pow(year,22) + -9.998791286046552e-51 * pow(year,21) + -1.32118154219509e-47 * pow(year,20) + -9.236985577038295e-45 * pow(year,19) + 1.8684686044932652e-41 * pow(year,18) + 1.0730402874275699e-37 * pow(year,17) + 3.2591762894928917e-34 * pow(year,16) + 7.830130724467377e-31 * pow(year,15) + 1.5969564665421713e-27 * pow(year,14) + 2.7542331220856135e-24 * pow(year,13) + 3.691454681046196e-21 * pow(year,12) + 2.3200056410821942e-18 * pow(year,11) + -6.832757231564678e-15 * pow(year,10) + -3.52569025091928e-11 * pow(year,9) + -1.0169359522579325e-07 * pow(year,8) + -0.00022359807875885738 * pow(year,7) + -0.3758739057297244 * pow(year,6) + -370.3511849305395 * pow(year,5) + 395573.5363661109 * pow(year,4) + 3193665975.170759 * pow(year,3) + 9066408055603.25 * pow(year,2) + 1.1366221113452164e+16 * pow(year,1) + -3.884265475935697e+19 * pow(year,0);
int mapValue = map(value, 4091445248, 11162517504, 500, 3000) 

// Normally Open configuration, send LOW signal to let current flow
  digitalWrite(RELAY, LOW);
  Serial.println("Current Flowing");
  delay(2000); / Normally Open configuration, send HIGH signal stop current flow
  digitalWrite(RELAY, HIGH);
  Serial.println("Current not Flowing");
  delay(1000);

  
}