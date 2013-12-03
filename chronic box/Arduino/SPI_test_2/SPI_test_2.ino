#include <SPI.h>

const int slaveSelectPin = 10;

void setup(){
  Serial.begin(115200);
  pinMode(slaveSelectPin, OUTPUT);
}

void loop(){
 for (byte channel = 0; channel < 1; channel++){
  for (byte level = 0; level < 255; level++){
   digitalPotWrite(channel, level);
   delay(10);
   Serial.print(0+channel);
   Serial.print("  ");
   Serial.println(0+level);
  }
  for (byte level = 0; level < 255; level++){
   digitalPotWrite(channel, 255 - level);
   delay(10);
   Serial.print(0+channel);
   Serial.print("  ");
   Serial.println(255-level);
  }  
  delay(500);
 } 
}

void digitalPotWrite(byte address, byte value){
  digitalWrite(slaveSelectPin, LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(slaveSelectPin, HIGH);
}
