//Project module 8 Create data-physicalization
//This code is created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
//And extended upon by Frank Bosman for the LED simulation.
//the espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'

#define membersof(x) (sizeof(x) / sizeof(x[0]))

//ESP Now connectivity -------------
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
  LEDs,
  HOUSES
};

//DEFINES which commands the esp is listening to: by default it listens to ALL and itself(whoAmI).
//Change this to the purpose of your node! check the possibilities in the enum above!
enum nodeStates whoAmI = LEDs;
//defines how often your actuate function gets called! 1000=once every second.
int actuateInterval = 100;

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
int oldYear = 0;


// LED simulation -------------
#include <FastLED.h>
const int NUM_LEDS = 150;
const int NUM_LEDS_HOUSES = 170;
const int NUM_LEDS_CANAL = 40;

CRGB leds[NUM_LEDS];
CRGB ledsHouse[NUM_LEDS_HOUSES];
CRGB ledsCanal[NUM_LEDS_CANAL];

struct car {
  float pos;
  int id;
  boolean alive;
  int distanceTraveld;
  float velocity;
  float baseVelocity;
};

car LED_cars[50];
const float maxSpeed = 0.5;
const float minSpeed = 0.1;

// dataset
float years[] = {2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021};
float values[] = { 5.478888,  16.689393,  24.800169,  28.267820,  0.000000,  4.516175,  3.221977,  33.419927,  41.540107,  55.402483,  61.239891,  70.529669,  78.164378,  83.490455,  87.174393,  88.004279,  94.107344,  99.372296,  98.675240,  94.991301,  93.707683,  100.000000};
int yearData = 10;

// houses
const int startYearHouses = 1970;
const int endYearHouses = 2019;
int housesPerYear[] = {1, 2, 4, 4, 5, 6, 7, 8, 9, 10, 12, 13, 16, 18, 21, 23, 25, 27, 31, 34, 37, 40, 43, 46, 47, 50, 51, 54, 57, 59, 62, 64, 66, 68, 69, 71, 74, 78, 81, 84, 86, 88, 89, 90, 92, 94, 96, 99, 100, 100};
int amountHouses = 10;
float rainbowBarf = 0;


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

  if (year != oldYear) {
    oldYear = year;
    updateYear(year);
  }
}

void setup() {
  Serial.begin(115200);     // initialize serial port

  // LED strips
  FastLED.addLeds<WS2811, 0, GRB>(leds, NUM_LEDS); // port D3
  FastLED.addLeds<WS2811, 5, GRB>(ledsHouse, NUM_LEDS_HOUSES); // port D1
  FastLED.addLeds<WS2811, 14, GRB>(ledsCanal, NUM_LEDS_CANAL); // port D5


  // clear the leds
  FastLED.clear();  // clear all pixel data
  FastLED.show();

  // startup animation
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    delay(int(500.0 / float(NUM_LEDS)));
    FastLED.show();
  }
  for (int i = 0; i < NUM_LEDS_HOUSES; i++) {
    ledsHouse[i] = CHSV(25.5, 75 * 255, 57 * 255);
    delay(int(500.0 / float(NUM_LEDS_HOUSES)));
    FastLED.show();
  }
  for (int i = 0; i < NUM_LEDS_CANAL; i++) {
    ledsCanal[i] = CHSV(25.5, 75 * 255, 57 * 255);
    delay(int(500.0 / float(NUM_LEDS_CANAL)));
    FastLED.show();
  }

  startTime = millis();
  actuateTime = millis();


  // Ysbrand ESP magic:
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


  // init the cars
  for (int i = 0; i <= membersof(LED_cars); i++) {
    LED_cars[i] = {0, 0, false, 0, float(int(random(maxSpeed * 10, minSpeed * 10))) / 10, float(int(random(maxSpeed * 10, minSpeed * 10))) / 10};
  }

  // clear everything
  delay(250);
  FastLED.clear();  // clear all pixel data
  FastLED.show();
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
    //    FastLED.clear();  // clear all pixel data

    // Show the Canal houses (grachten panten).
    fill_solid(ledsCanal, NUM_LEDS_CANAL, CHSV(25.5, 75 * 255, 57 * 255));

    if (selected == whoAmI || selected == ALL) {
      //      doActuate();

    }


    //    FastLED.show();
  }
  if (selected == HOUSES || selected == ALL) {
    doActuateHouses();
  }
  FastLED.show();
}

// This method is called when the year is changed
void updateYear(int year) {
  for (int i = 0; i < membersof(years); i++) {
    if (year == years[i]) {
      yearData = int(map(values[i], 0, 100, 1, 50));
      break;
    }
  }

  amountHouses = min(housesPerYear[min(int(float(year - startYearHouses) * float(NUM_LEDS_HOUSES) / 100), 49)], NUM_LEDS_HOUSES);
  Serial.println(amountHouses + amountHouses);

  for (int j = 0; j < membersof(LED_cars); j++) {
    LED_cars[j].alive = false;
  }
}

// this function is called every actuate interval (200ms)
void doActuate() {
  // loop trough and update all the cars
  for (int i = 0; i < yearData; i++) {
    //    Serial.println(LED_cars[i].pos);

    // test if the current car is alive or dead
    if (!LED_cars[i].alive) { // then it's dead, and has a change to respawn

      if (int(random(0, 5)) == 0) { // change to respawn
        int place = int(random(0, NUM_LEDS)); //random place to respawn

        //test if the car is allowed to respawn on the random spot
        boolean allowedToSpawn = true;
        for (int j = 0; j < yearData; j++) {
          if ((LED_cars[i].id == (place >= NUM_LEDS ? place - NUM_LEDS : place) || LED_cars[i].id == (place + 1 >= NUM_LEDS ? place + 1 - NUM_LEDS : place + 1) || LED_cars[i].id == (place - 1 >= NUM_LEDS ? place - 1 - NUM_LEDS : place - 1)) && LED_cars[j].alive) {
            allowedToSpawn = false;
            break;
          }

          // if it is allowed, reset al it's values
          if (allowedToSpawn) {
            LED_cars[i].pos = place;
            LED_cars[i].id = place;
            LED_cars[i].alive = true;
            LED_cars[i].baseVelocity = float(int(random(maxSpeed * 10, minSpeed * 10))) / 10; // (i == 0 ? (maxSpeed + minSpeed) / 2 : maxSpeed); //TODO CHANGE THIS TO A RANDOM SPEED
            LED_cars[i].velocity = LED_cars[i].baseVelocity;
          }
        }
      }
    }
    else { // when the car is alive
      // test if it is allowed to move and test if there is a jam behind it.
      boolean allowedToMove = true;
      boolean travicJamBehind = false;
      for (int j = 0; j < yearData; j++) {
        // test if car in front
        if (LED_cars[j].id == (LED_cars[i].id + 2 >= NUM_LEDS ? LED_cars[i].id + 2 - NUM_LEDS : LED_cars[i].id + 2) && LED_cars[j].alive) {
          allowedToMove = false;
        }
        // test if car behind
        if ((LED_cars[j].id == (LED_cars[i].id - 2 >= NUM_LEDS ? LED_cars[i].id - 2 - NUM_LEDS : LED_cars[i].id - 2) || LED_cars[j].id == (LED_cars[i].id - 3 >= NUM_LEDS ? LED_cars[i].id - 3 - NUM_LEDS : LED_cars[i].id - 3)) && LED_cars[j].alive) {
          travicJamBehind = true;
        }
      }

      //
      LED_cars[i].velocity = (travicJamBehind ? max(minSpeed, LED_cars[i].baseVelocity / 2) : LED_cars[i].baseVelocity);

      // move the car forwards
      if (allowedToMove) {
        LED_cars[i].pos += LED_cars[i].velocity;
        LED_cars[i].distanceTraveld += LED_cars[i].velocity;

        if (int(LED_cars[i].pos) >= NUM_LEDS) LED_cars[i].pos = LED_cars[i].pos - NUM_LEDS;
        LED_cars[i].id = int(LED_cars[i].pos);
      }

      // show a red and white LED on the cars place
      leds[LED_cars[i].id] = CRGB::Red;
      leds[(LED_cars[i].id + 1 >= NUM_LEDS ? 0 : LED_cars[i].id + 1)] = CRGB::White;

      // there is a chance for it to die
      if (LED_cars[i].distanceTraveld >= NUM_LEDS * 0.7 - int(random(0, NUM_LEDS / 3))) {
        LED_cars[i].alive = false;
      }
    }

  }
}

// this function is called every actuate interval (200ms) and handles the houses lighting up
void doActuateHouses() {

  if (year != 0) {
    for (int i = 0; i < NUM_LEDS_HOUSES; i++) {
      if (i <= amountHouses) {
        ledsHouse[i] = CHSV(25.5, 75 * 255, 57 * 255);// CRGB::Blue;
      } else {
        ledsHouse[i] = CRGB(0, 0, 0);
      }
    }
  } else {
    rainbowBarf += 0.5;
    if (rainbowBarf > 255) {
      rainbowBarf = 0;
    }
    fill_rainbow(ledsHouse, NUM_LEDS_HOUSES, int(rainbowBarf));
  }
}
