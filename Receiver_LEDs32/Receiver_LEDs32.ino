// Project module 8 Create data-physicalization
// The ESP connectivity in this code is rewritten for ESP32 by Frank Bosman from an code created and adapted by Ysbrand Burgstede to be able to receive years from the sender esp8266 and let the modules react to the given year.
// And it is extended upon by Frank Bosman for the LED simulation.
// The espnow- section of this code is largely inspired by https://github.com/bnbe-club/esp-now-examples-diy-62 which is under the creative commons license 'CC0 1.0 Universal'

// MAC Address: E0:E2:E6:0C:9F:98
// {0xE0,0xE2,0xE6,0x0C,0x9F,0x98}

#include <FastLED.h>

// ------------- ESP Now connectivity -------------
#include <WiFi.h>
#include <esp_now.h>

#define WIFI_CHANNEL 1
#define membersof(x) (sizeof(x) / sizeof(x[0]))

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
enum nodeStates whoAmI = LEDs;

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
// interval for updating LEDs
long actuateTime = 0;
int actuateInterval = 200;

// data:
int year = 0;
int oldYear = 0;

// -------------- LEDs --------------
const int NUM_LEDS = 95;
const int NUM_LEDS_HOUSES = 170;

CRGB leds[NUM_LEDS];
CRGB ledsHouse[NUM_LEDS_HOUSES];

// cars:
struct car
{
  float pos;
  int id;
  boolean alive;
  int distanceTraveld;
  float velocity;
  float baseVelocity;
};

car LED_cars[50];
const float maxSpeed = 1;
const float minSpeed = 0.1;

// dataset
// float years[] = {2000, 2001, 2002, 2003, 2004, 2005, 2006, 2007, 2008, 2009, 2010, 2011, 2012, 2013, 2014, 2015, 2016, 2017, 2018, 2019, 2020, 2021};
int carYearStart = 2000;
int carYearEnd = 2021;
float carsPerYear[] = {5.478888, 16.689393, 24.800169, 28.267820, 0.000000, 4.516175, 3.221977, 33.419927, 41.540107, 55.402483, 61.239891, 70.529669, 78.164378, 83.490455, 87.174393, 88.004279, 94.107344, 99.372296, 98.675240, 94.991301, 93.707683, 100.000000};
int carData = 10;

// houses data:
const int startYearHouses = 1970;
const int endYearHouses = 2019;
int housesPerYear[] = {1, 2, 4, 4, 5, 6, 7, 8, 9, 10, 12, 13, 16, 18, 21, 23, 25, 27, 31, 34, 37, 40, 43, 46, 47, 50, 51, 54, 57, 59, 62, 64, 66, 68, 69, 71, 74, 78, 81, 84, 86, 88, 89, 90, 92, 94, 96, 99, 100, 100};
int amountHouses = 10;

void updateYear()
{
  amountHouses = min(int(float(housesPerYear[max(min(int(year - startYearHouses), 49), 0)]) * float(NUM_LEDS_HOUSES) / 100), NUM_LEDS_HOUSES);
  carData = min(int(float(housesPerYear[max(min(int(year - carYearStart), 21), 0)]) * 50 / 100), NUM_LEDS);

  if (debugMode)
  {
    Serial.print("houses: ");
    Serial.println(amountHouses);
    Serial.print("year = ");
    Serial.println(year);
  }
}

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
  year = packet.year;
  selected = packet.selected;

  if (year != oldYear)
  {
    oldYear = year;
    updateYear();
  }
}

void setup()
{
  Serial.begin(115200);
  FastLED.addLeds<WS2811, 32, GRB>(leds, NUM_LEDS);             // cars
  FastLED.addLeds<WS2811, 19, GRB>(ledsHouse, NUM_LEDS_HOUSES); // houses

  // clear the leds
  FastLED.clear(); // clear all pixel data
  FastLED.show();
  FastLED.setBrightness(255); // brightness of the LEDS, max = 255

  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = CHSV(25, int(.75 * 255), int(.57 * 255));
    delay(int(500.0 / float(NUM_LEDS)));
    FastLED.show();
  }

  for (int i = 0; i < NUM_LEDS_HOUSES; i++)
  {
    ledsHouse[i] = CHSV(25, int(.75 * 255), int(.57 * 255));
    delay(int(500.0 / float(NUM_LEDS_HOUSES)));
    FastLED.show();
  }

  startTime = millis();
  actuateTime = millis();

  // ESP now connection REWRITTEN FOR ESP32!!
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(); // we do not want to connect to a WiFi network

  if (esp_now_init() != ESP_OK)
  {
    Serial.println("ESP-NOW initialization failed");
    year = 0;
    return;
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

  // init cars
  for (int i = 0; i <= membersof(LED_cars); i++)
  {
    LED_cars[i] = {0, 0, false, 0, float(int(random(maxSpeed * 10, minSpeed * 10))) / 10, float(int(random(maxSpeed * 10, minSpeed * 10))) / 10};
  }
  // clear everything
  delay(250);
  FastLED.clear(); // clear all pixel data
  FastLED.show();
}

void doActuateHouses()
{
  if (year == 0)
  { // party mode
    int rainbowBarf = int(millis() / 20 % 255);
    fill_rainbow(ledsHouse, NUM_LEDS_HOUSES, rainbowBarf);
  }
  else if (year <= endYearHouses && year >= startYearHouses)
  {
    fill_solid(ledsHouse, min(amountHouses, NUM_LEDS_HOUSES), CHSV(25, int(.75 * 255), int(.57 * 255)));
  }
  else
  {
    fill_solid(ledsHouse, NUM_LEDS_HOUSES, CRGB(0, 0, 0));
  }
}

// this function is called every actuate interval (200ms)
void doActuate()
{
  // loop trough and update all the cars
  if (year == 0)
  { // party mode
    int rainbowBarf = int(millis() / 5 % 255);
    fill_rainbow(leds, NUM_LEDS, rainbowBarf);
    
    return;
  }
  if (year < carYearStart || year > carYearEnd){
    return;
  }
  for (int i = 0; i < carData; i++)
  {
    if (debugMode)
    {
      Serial.print(LED_cars[i].pos);
      Serial.print(" ");
    }

    // test if the current car is alive or dead
    if (!LED_cars[i].alive)
    { // then it's dead, and has a change to respawn

      if (int(random(0, 5)) == 0)
      {                                       // change to respawn
        int place = int(random(0, NUM_LEDS)); // random place to respawn

        // test if the car is allowed to respawn on the random spot
        boolean allowedToSpawn = true;
        for (int j = 0; j < carData; j++)
        {
          if ((LED_cars[i].id == (place >= NUM_LEDS ? place - NUM_LEDS : place) || LED_cars[i].id == (place + 1 >= NUM_LEDS ? place + 1 - NUM_LEDS : place + 1) || LED_cars[i].id == (place - 1 >= NUM_LEDS ? place - 1 - NUM_LEDS : place - 1)) && LED_cars[j].alive)
          {
            allowedToSpawn = false;
            break;
          }

          // if it is allowed, reset al it's values
          if (allowedToSpawn)
          {
            LED_cars[i].pos = place;
            LED_cars[i].id = place;
            LED_cars[i].alive = true;
            LED_cars[i].baseVelocity = float(int(random(maxSpeed * 10, minSpeed * 10))) / 10; // (i == 0 ? (maxSpeed + minSpeed) / 2 : maxSpeed); //TODO CHANGE THIS TO A RANDOM SPEED
            LED_cars[i].velocity = LED_cars[i].baseVelocity;
            LED_cars[i].distanceTraveld = 0;
          }
        }
      }
    }
    else
    { // when the car is alive
      // test if it is allowed to move and test if there is a jam behind it.
      boolean allowedToMove = true;
      boolean travicJamBehind = false;
      for (int j = 0; j < carData; j++)
      {
        // test if car in front
        if (LED_cars[j].id == (LED_cars[i].id + 2 >= NUM_LEDS ? LED_cars[i].id + 2 - NUM_LEDS : LED_cars[i].id + 2) && LED_cars[j].alive)
        {
          allowedToMove = false;
        }
        // test if car behind
        if ((LED_cars[j].id == (LED_cars[i].id - 2 >= NUM_LEDS ? LED_cars[i].id - 2 - NUM_LEDS : LED_cars[i].id - 2) || LED_cars[j].id == (LED_cars[i].id - 3 >= NUM_LEDS ? LED_cars[i].id - 3 - NUM_LEDS : LED_cars[i].id - 3)) && LED_cars[j].alive)
        {
          travicJamBehind = true;
        }
      }

      //
      LED_cars[i].velocity = (travicJamBehind ? max(minSpeed, LED_cars[i].baseVelocity / 2) : LED_cars[i].baseVelocity);

      // move the car forwards
      if (allowedToMove)
      {
        LED_cars[i].pos += LED_cars[i].velocity;
        LED_cars[i].distanceTraveld += LED_cars[i].velocity;

        if (int(LED_cars[i].pos) >= NUM_LEDS)
          LED_cars[i].pos = LED_cars[i].pos - NUM_LEDS;
        LED_cars[i].id = int(LED_cars[i].pos);
      }

      // show a red and white LED on the cars place
      leds[LED_cars[i].id] = CRGB(50, 0, 0);
      leds[(LED_cars[i].id + 1 >= NUM_LEDS ? 0 : LED_cars[i].id + 1)] = CRGB(50, 50, 50);

      // there is a chance for it to die
      if (LED_cars[i].distanceTraveld >= NUM_LEDS * 0.7)
      { //- int(random(0, NUM_LEDS / 3))) {
        LED_cars[i].alive = false;
      }
    }
  }

  if (debugMode){
    Serial.println();
  }
}

void loop()
{
  if (millis() - startTime >= interval)
  {
    startTime = millis();

    dataPacket packet;
    packet.selected = selected;
    packet.year = year;
    packet.whoAmI = whoAmI;
    esp_now_send(receiverAddress, (uint8_t *)&packet, sizeof(packet));
  }
  if (millis() - actuateTime >= actuateInterval)
  {
    actuateTime = millis();
    FastLED.clear(); // clear all pixel data

    if (selected == whoAmI || selected == ALL)
    { // cars
      doActuate();
    }

    if (selected == HOUSES || selected == ALL)
    {
      doActuateHouses();
    }
    FastLED.show();
  }
}
