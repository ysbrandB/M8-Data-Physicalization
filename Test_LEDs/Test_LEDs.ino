
#include <FastLED.h>
#define membersof(x) (sizeof(x) / sizeof(x[0]))

const int NUM_LEDS = 150;
const int DATA_PIN = 5;
CRGB leds[NUM_LEDS];

struct car {
  float pos;
  int id;
  boolean alive;
  int distanceTraveld;
  float velocity;
};

car LED_cars[10];
const float maxSpeed = 0.5;
const float minSpeed = 0.1;


void setup() {
  Serial.begin(115200);     // initialize serial port
  FastLED.addLeds<WS2811, 5, GRB>(leds, NUM_LEDS);

  FastLED.clear();  // clear all pixel data
  FastLED.show();

  for (int i = 0; i <= NUM_LEDS; i++) {
    leds[i] = CRGB::Red;
    delay(int(500.0 / float(NUM_LEDS)));
    FastLED.show();
  }

  for (int i = 0; i <= membersof(LED_cars); i++) {
    LED_cars[i] = {0, 0, false, 0, 0.5};
  }
  LED_cars[0] = {0, 0, true, 0, 0.1};

  delay(250);
  FastLED.clear();  // clear all pixel data
  FastLED.show();

}

int test = 0;

void loop() {
  FastLED.clear();  // clear all pixel data
  //  Serial.println("------------------");

  for (int i = membersof(LED_cars) - 1; i >= 0; i--) {
    if (!LED_cars[i].alive) { // then it's dead, and has a change to respawn
      if (int(random(0, 5)) == 0) {
        int place = int(random(0, NUM_LEDS));
        boolean allowedToSpawn = true;
        for (int j = 0; j < membersof(LED_cars); j++) {
          if ((LED_cars[i].id == (place >= NUM_LEDS ? place - NUM_LEDS : place) || LED_cars[i].id == (place + 1 >= NUM_LEDS ? place + 1 - NUM_LEDS : place + 1) || LED_cars[i].id == (place - 1 >= NUM_LEDS ? place - 1 - NUM_LEDS : place - 1)) && LED_cars[j].alive) {
            allowedToSpawn = false;
            break;
          }

          if (allowedToSpawn)
            LED_cars[i].pos = place;
          LED_cars[i].id = place;
          LED_cars[i].alive = true;
        }
      }
    }
    else { // when it is alive
      boolean allowedToMove = true;
      boolean travicJamBehind = false;
      for (int j = 0; j < membersof(LED_cars); j++) {
        if (LED_cars[j].id == (LED_cars[i].id + 2 >= NUM_LEDS ? LED_cars[i].id + 2 - NUM_LEDS : LED_cars[i].id + 2) && LED_cars[j].alive) {
          allowedToMove = false;
        }
        if ((LED_cars[j].id == (LED_cars[i].id - 2 >= NUM_LEDS ? LED_cars[i].id - 2 - NUM_LEDS : LED_cars[i].id - 2) || LED_cars[j].id == (LED_cars[i].id - 3 >= NUM_LEDS ? LED_cars[i].id - 3 - NUM_LEDS : LED_cars[i].id - 3)) && LED_cars[j].alive) {
          travicJamBehind = true;
        }
      }

      LED_cars[i].velocity = (travicJamBehind ? minSpeed : maxSpeed);

      // move it forward one more
      if (allowedToMove) {
        LED_cars[i].pos += LED_cars[i].velocity;
        LED_cars[i].distanceTraveld += LED_cars[i].velocity;

        if (int(LED_cars[i].pos) >= NUM_LEDS) LED_cars[i].pos = LED_cars[i].pos - NUM_LEDS;
        LED_cars[i].id = int(LED_cars[i].pos);

      }

      // show a LED
      leds[LED_cars[i].id] = CRGB::Red;
      leds[(LED_cars[i].id + 1 >= NUM_LEDS ? 0 : LED_cars[i].id + 1)] = CRGB::White;


      // there is a chance for it to die
      if (LED_cars[i].distanceTraveld >= NUM_LEDS - int(random(0, NUM_LEDS / 3))) {
        LED_cars[i].alive = false;
      }
    }
  }

  FastLED.show();

  delay(100);
}
