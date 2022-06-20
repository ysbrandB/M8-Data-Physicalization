//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'


//ESP Now connectivity
#include<ESP8266WiFi.h>
#include<espnow.h>

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
  Serial.print("year: ");
  Serial.println(packet.year);
  Serial.print("selected: ");
  Serial.println(packet.selected);
  if (year != packet.year) {
    year = packet.year;
  }
}

void setup() {
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
}

void doActuate() {
  //PUT CODE HERE FOR YOUR ACTUATOR!
}
