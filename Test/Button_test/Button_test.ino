bool button=false;
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(2, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(digitalRead(2)){
  button=false;
  }
  else{button=true;
  }
  Serial.println(button);
}
