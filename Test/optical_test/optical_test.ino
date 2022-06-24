// interrupt  example
//
// In this sketch a BOURNS ECW1J-series encoder is
// connected to Pins 2 and 3. Pinout of the encoder is
// 1    2    3
// A - GND - B
///////////////////////////
volatile  int position;
// hardware pins
int phaseA = 4;
int phaseB = 5;
int debounceInt = 1000;
long debounceTimer;
long lastDebounceTime;
void setup()
{
  Serial.begin(115200);
  attachInterrupt(digitalPinToInterrupt(phaseA), encoderA, CHANGE); // triggers on phaseA (port 2)
  attachInterrupt(digitalPinToInterrupt(phaseB), encoderB, CHANGE); // triggers on phaseB (port 3)
  lastDebounceTime=millis();
}

void loop()
{
  Serial.println(position);
  if (debounceTimer > 0) {
    debounceTimer = debounceTimer - (millis()- lastDebounceTime);
    lastDebounceTime = millis();
  }
  Serial.println(debounceTimer);
}

ICACHE_RAM_ATTR void encoderA() // encoder service routine
{
  if (debounceTimer <= 0) {
    debounceTimer = debounceInt;
    int A = digitalRead(phaseA);
    int B = digitalRead(phaseB);
    if ((A == 1 && B == 0) || (A == 0 && B == 1)) position++;
    if ((A == 1 && B == 1) || (A == 0 && B == 0)) position--;
  }

}

ICACHE_RAM_ATTR void encoderB() // encoder service routine
{
  if (debounceTimer <= 0) {
    debounceTimer = debounceInt;
    int A = digitalRead(phaseA);
    int B = digitalRead(phaseB);
    if ((A == 1 && B == 0) || (A == 0 && B == 1)) position--;
    if ((A == 1 && B == 1) || (A == 0 && B == 0)) position++;
  }
}
