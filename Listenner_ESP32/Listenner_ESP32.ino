// Project module 8 Create data-physicalization
// The ESP connectivity in this code is rewritten for ESP32 by Frank Bosman from an code created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
// And it is extended upon by Frank Bosman for the LED simulation.
// The espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'

// MAC Address: E0:E2:E6:0C:9F:98
// {0xE0,0xE2,0xE6,0x0C,0x9F,0x98}

//#include <Arduino.h>

// ------------- ESP Now connectivity -------------
#include <WiFi.h>
#include <esp_now.h>

#define WIFI_CHANNEL 1
#define membersof(x) (sizeof(x) / sizeof(x[0]))
#define MAX_MESSAGE_LENGTH 12

// debugmode:
const boolean debugMode = false;

// BC:FF:4D:81:7D:DD
uint8_t receiverAddress[] = {0xBC, 0xFF, 0x4D, 0x81, 0x7D, 0xDD}; // CONTROLLER


// Peer info
esp_now_peer_info_t peerInfo;

enum nodeStates
{
  ALL,
  SMOKE,
  BUILDINGS,
  SOUND,
  LEDs,
  HOUSES
};

// DEFINES which commands the esp is listening to: by default it listens to ALL and itself(whoAmI).
// Change this to the purpose of your node! check the possibilities in the enum above!
enum nodeStates whoAmI = ALL;

// which node is selected by the controller
enum nodeStates selected = ALL;

struct __attribute__((packed)) dataPacket
{
  enum nodeStates whoAmI;
  int year;
  enum nodeStates selected;
};

// interval for sending
long startTime = 0;
int interval = 3000;

// -------------- ESP NOW --------------
void transmissionComplete(const uint8_t *receiver_mac, esp_now_send_status_t transmissionStatus)
{
  if (transmissionStatus == ESP_NOW_SEND_SUCCESS)
  {
    Serial.println("Data sent successfully");
  }
  else
  {
    Serial.print("Error code: ");
    Serial.println(transmissionStatus);
  }
}

IRAM_ATTR void dataReceived(const uint8_t *senderMac, const uint8_t *data, int dataLength)
{
  dataPacket packet;

  memcpy(&packet, data, sizeof(packet));

  Serial.print("whoIsSender: ");
  Serial.println(packet.whoAmI);
  Serial.print("packet year: ");
  Serial.println(packet.year);
  Serial.print("packet selected: ");
  Serial.println(packet.selected);
  selected = packet.selected;
  Serial.println("");
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
      int year = atoi(message);
      Serial.println(year);
      //Reset for the next message
      message_pos = 0;

      dataPacket packet;
      packet.selected = selected;
      packet.year = year;
      packet.whoAmI = whoAmI;
      esp_now_send(receiverAddress, (uint8_t *)&packet, sizeof(packet));
    }
  }
}


void setup()
{
  Serial.begin(115200);
  startTime = millis();

  // ESP now connection REWRITTEN FOR ESP32!!
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // we do not want to connect to a WiFi network

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    Serial.println("Retrying in 5 seconds");
    for(int i = 1; i<=10; i++){
      Serial.print(".");
      delay(500);
    }
    Serial.println("Restarting..");
    ESP.restart();
  }
  Serial.println("ESP-NOW Init Succesfull");

  // Resister peer
  memcpy(peerInfo.peer_addr, receiverAddress, 6);
  peerInfo.channel = WIFI_CHANNEL;
  peerInfo.encrypt = false;

  // esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
  esp_now_register_send_cb(transmissionComplete); // this function will get called once all data is sent
  esp_now_register_recv_cb(dataReceived);         // this function will get called whenever we receive data
  esp_now_add_peer(&peerInfo);
  Serial.println("ESP-NOW Initialized.");
}

void loop()
{

  updateSerial();

  if (millis() - startTime >= interval)
  {
    startTime = millis();

    // dataPacket packet;
    // packet.selected = selected;
    // packet.year = year;
    // packet.whoAmI = whoAmI;
    // esp_now_send(receiverAddress, (uint8_t *)&packet, sizeof(packet));
  }
}
