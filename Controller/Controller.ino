//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to send the selected year to all subsystems and their esp8266.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'

#include<ESP8266WiFi.h>
#include<espnow.h>

#define WIFI_CHANNEL    1
#define MAX_MESSAGE_LENGTH 12
#define membersof(x) (sizeof(x) / sizeof(x[0]))

#include <SPI.h>
#include <MFRC522.h>
#define SS_PIN D8
#define RST_PIN D0
MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class

byte uids[][4] = {
  {224, 87, 84, 25},
  {10, 115, 204, 36},
  {10, 66, 118, 36},
  {186, 120, 6, 25},
  {186, 104, 31, 25}
};

enum nodeStates {
  ALL,
  SMOKE,
  BUILDINGS,
  SOUND,
  LEDs
};

String translation[5] = {
  "ALL",
  "SMOKE",
  "BUILDINGS",
  "SOUND",
  "LEDs"
};

enum nodeStates selected = ALL;
enum nodeStates whoAmI = ALL;
struct __attribute__((packed)) dataPacket {
  enum nodeStates whoAmI;
  int year;
  enum nodeStates selected;
};

uint8_t receiverAddresses[][8] =
{ {0xBC, 0xFF, 0x4D, 0x81, 0x8A, 0xCA},
  {0x7C, 0x87, 0xCE, 0x81, 0xB6, 0x54},
  {0xBC, 0xFF, 0x4D, 0x81, 0x8E, 0xF5},
  {0xBC, 0xFF, 0x4D, 0x81, 0x7D, 0x8D}
};

String receivedMacAdresses[membersof(receiverAddresses)];

float startTime;
float interval = 3000;
float loopTime;
float loopInterval = 3000;
int year = 1970;
int totalConnected = 0;


void transmissionComplete(uint8_t *receiver_mac, uint8_t transmissionStatus) {
  if (transmissionStatus == 0) {
    Serial.println("Data sent successfully");
    totalConnected += 1;
  } else {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

void dataReceived(uint8_t *senderMac, uint8_t *data, uint8_t dataLength) {
  char macStr[18];
  dataPacket packet;

  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x", senderMac[0], senderMac[1], senderMac[2], senderMac[3], senderMac[4], senderMac[5]);
  memcpy(&packet, data, sizeof(packet));
  Serial.println();
  Serial.print("Received data from |");
  Serial.print(translation[packet.whoAmI] + "| ");
  Serial.println(macStr);
  Serial.println();
}

void setup() {
  Serial.begin(115200);     // initialize serial port

  Serial.println();
  Serial.println();
  Serial.println("...initializing...");

  SPI.begin(); // Init SPI bus
  rfid.PCD_Init(); // Init MFRC522

  WiFi.mode(WIFI_STA);
  WiFi.disconnect();        // we do not want to connect to a WiFi network

  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW initialization failed");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(transmissionComplete);         // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);               // this function will get called whenever we receive data
  for (int i = 0; i < membersof(receiverAddresses); i++) {
    esp_now_add_peer(receiverAddresses[i], ESP_NOW_ROLE_COMBO, WIFI_CHANNEL, NULL, 0);
  }
  startTime = millis();
  
  Serial.println("Initialized.");
}

void loop() {
  if (millis() - startTime >= interval) {
    startTime = millis();

    Serial.println("--------------------------");
    Serial.println("Connected: " + String(totalConnected));
    Serial.println("year: " + String(year));
    Serial.println("selected: " + String(selected));
    
    dataPacket packet;
    packet.selected = selected;
    packet.year = year;
    
    totalConnected = 0;
    for (int i = 0; i < membersof(receiverAddresses); i++) {
      esp_now_send(receiverAddresses[i], (uint8_t *) &packet, sizeof(packet));
    }    
  }
  if (millis() - loopTime >= loopInterval) {
    loopTime = millis();
    year+=1;
    if(year>2030){
      year=1970;
     }
//     need to omit 1989 from the ring because Frank and Tristan did a fukkiewukkie
     if(year==1989){
      year=1990;
      }
  }
  updateSerial();

  if ( ! rfid.PICC_IsNewCardPresent())
    return;
  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  for (byte i = 0; i < 5; i++) {
    if (rfid.uid.uidByte[0] == uids[i][0] && rfid.uid.uidByte[1] == uids[i][1] && rfid.uid.uidByte[2] == uids[i][2] && rfid.uid.uidByte[3] == uids[i][3]) {
      selected=static_cast<nodeStates>(i);
    }
  }
  rfid.PICC_HaltA();
  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}

void updateSerial() {
  //Check to see if anything is available in the serial receive buffer
  while (Serial.available() > 0)
  {
    //Create a place to hold the incoming message
    static char message[MAX_MESSAGE_LENGTH];
    static unsigned int message_pos = 0;

    //Read the next available byte in the serial receive buffer
    char inByte = Serial.read();

    //Message coming in (check not terminating character) and guard for over message size
    if ( inByte != '\n' && (message_pos < MAX_MESSAGE_LENGTH - 1) )
    {
      //Add the incoming byte to our message
      message[message_pos] = inByte;
      message_pos++;
    }
    //Full message received...
    else
    {
      //Add null character to string
      message[message_pos] = '\0';
      year = atoi(message);
      Serial.println(year);
      //Reset for the next message
      message_pos = 0;
    }
  }
}
