
#include <FastLED.h>
#define membersof(x) (sizeof(x) / sizeof(x[0]))

const int NUM_LEDS = 60;
const int DATA_PIN = 5;
CRGB leds[NUM_LEDS];

struct car {
  int pos;
  boolean alive;
  int distanceTraveld;
};

car LED_cars[10];


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
    LED_cars[i] = {0, false, 0};
  }
  LED_cars[0] = {0, true, 0};

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
          if ((LED_cars[i].pos == (place >= NUM_LEDS ? place - NUM_LEDS : place) || LED_cars[i].pos == (place+1 >= NUM_LEDS ? place + 1 - NUM_LEDS : place +1)) && LED_cars[j].alive) {
            allowedToSpawn = false;
            break;
          }

          if (allowedToSpawn)
            LED_cars[i].pos = place;
          LED_cars[i].alive = true;
        }
      }
    }
    else { // when it is alive
      boolean allowedToMove = true;
      for (int j = 0; j < membersof(LED_cars); j++) {
        if (LED_cars[j].pos == (LED_cars[i].pos + 2 >= NUM_LEDS ? LED_cars[i].pos + 2 - NUM_LEDS : LED_cars[i].pos + 2) && LED_cars[j].alive) {
          allowedToMove = false;
          break;
        }
      }

      // move it forward one more
      if (allowedToMove) {
        LED_cars[i].pos += 1;
        if (LED_cars[i].pos >= NUM_LEDS) LED_cars[i].pos = 0;
      }

      // show a LED
      leds[LED_cars[i].pos] = CRGB::Red;
      leds[(LED_cars[i].pos + 1 >= NUM_LEDS ? 0 : LED_cars[i].pos + 1)] = CRGB::White;


      // there is a chance for it to die
      if (int(random(0, 10)) == 0) {
        LED_cars[i].alive = false;
      }
    }
  }

  FastLED.show();

  delay(500);
}
