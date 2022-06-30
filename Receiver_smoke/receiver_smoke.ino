//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'


//ESP Now connectivity
#include<ESP8266WiFi.h>
#include<espnow.h>

#define WIFI_CHANNEL    1

#define relaySmoke 14 //D5
#define relayFan 5  //D1

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
int interval = 6000;
//which node is selected by the controllerf
enum nodeStates selected = ALL;

struct __attribute__((packed)) dataPacket {
  enum nodeStates whoAmI;
  int year;
  enum nodeStates selected;
};


long startTime = 0;
long oldMillis = 0;
long oldTime = 0;
int year = 0;
long smokeTimer = 0;

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
//
//  //  Serial.print("whoIsSender: ");
//  //  Serial.println(packet.whoAmI);
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
  oldMillis = millis();
  oldTime = millis();
  
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

  pinMode(relaySmoke, OUTPUT);
  digitalWrite(relaySmoke, LOW);

  pinMode(relayFan, OUTPUT);
  digitalWrite(relayFan, LOW);

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

int oldYear = 999;
int arrayValue[] = {37, 36, 66, 81, 72, 57, 82, 68, 75, 100, 78, 54, 0, 3, 11, 14, 9, 21, 12, 37, 37, 52, 53, 55, 57, 60, 74, 62, 63, 53, 54, 62, 60, 65, 68, 62, 55, 55, 61, 54, 74, 54, 50, 51, 41, 53, 55, 53, 48, 41, 16};

bool notSmoking = true;

void loop() {
  if (millis() - startTime >= interval) {
    startTime = millis();
    dataPacket packet;

    packet.selected = selected;
    packet.year = year;
    packet.whoAmI = whoAmI;
    esp_now_send(receiverAddress, (uint8_t *) &packet, sizeof(packet));
//    year = random(1970, 2020);
  }

  if (year != oldYear && year >= 1970 && year <= 2020 && smokeTimer <= 0) {
    oldYear = year;

    int value = arrayValue[year - 1970];
//    Serial.print("value: ");
//    Serial.println(value);

    smokeTimer = map(value, 0, 100, 0, 1000);
    Serial.print("smoke timer: ");
    Serial.println(smokeTimer);
  }

  if (selected == whoAmI || selected == ALL) {
    startSmoke();
  }
}


void startSmoke() {
  if (year > 2020) return;
    Serial.println(smokeTimer);
  
  if (smokeTimer > 0) {
    smokeTimer -= (millis() - oldMillis);
    oldMillis = millis();
    digitalWrite(relaySmoke, HIGH);
    notSmoking = false;
  } else {
    smokeTimer = 0;
    digitalWrite(relaySmoke, LOW);
    notSmoking = true;
  }
}

void startFan() {
  if (notSmoking) {
    if ( millis() - oldTime >= 2000) {
      oldTime = millis();

      //  if (smokeTimer > 0) {
      //    smokeTimer -= (millis() - oldMillis);
      //    oldMillis = millis();
      digitalWrite(relayFan, LOW);
      Serial.println("Fan!");
    }
  } else {
    digitalWrite(relayFan, HIGH);
//    Serial.println("No Fan!!");
  }

}