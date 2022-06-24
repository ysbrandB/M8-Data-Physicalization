void setup() {
  // put your setup code here, to run once:
Serial.begin(115200);
pinMode(4, INPUT);
pinMode(5, INPUT);
}

void loop() {
  Serial.print("D2:");
  Serial.print(digitalRead(5));
  Serial.print(", D3:");
   Serial.println(digitalRead(4));
  // put your main code here, to run repeatedly:

}
