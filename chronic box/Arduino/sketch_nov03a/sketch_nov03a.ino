char INBYTE;
int  LED = 13, i; // LED on pin 13

void setup() {
  Serial.begin(9600); 
  pinMode(LED, OUTPUT);
}

void loop() {
  
  jaceSerialEvent();

}

void jaceSerialEvent() {
      
  while (Serial.available()) {
    INBYTE = Serial.read();        // read next available byte
    if( INBYTE == '0' ) digitalWrite(LED, LOW);  // if it's a 0 (zero) tun LED off
    if( INBYTE == '1' ) digitalWrite(LED, HIGH); // if it's a 1 (one) turn LED on
    Serial.print("Press 1 to turn Arduino pin 13 LED ON or 0 to turn it OFF - received");
    Serial.println(INBYTE);
  }
  
}
