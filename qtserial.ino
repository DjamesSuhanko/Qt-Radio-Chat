char LED[2] = {'0\n'};

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  pinMode(13,OUTPUT);
  digitalWrite(13,LOW);
}

void loop() {
  int i = 0;
  while (Serial.available()){
    LED[i] = Serial.read();
    i++;
  }
  delay(500);
  if (LED[0] == '1'){
    digitalWrite(13,HIGH);
  }
  else if (LED[0] == '0'){
    digitalWrite(13,LOW);
  }
  delay(500);
  long mil     = millis();
  char buf[10] = {0};
  String msg   = String(mil);
  msg.toCharArray(buf,5); 
  Serial.write(buf);

}
