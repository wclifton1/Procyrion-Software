#include <SPI.h>

const int slaveSelectPin = 6, channel = 1;
byte level;

void setup() {

  pinMode (slaveSelectPin, OUTPUT);

  // initialize SPI:
//  SPI.setBitOrder (MSBFIRST);
 // SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider (SPI_CLOCK_DIV2);    // not sure this is necessary
  SPI.begin (); 
  
}

void loop() {

  Serial.println ("Hello");
  for (level = 0; level < 256; level++) {

//    Serial.println (level);
    digitalPotWrite (channel, level);

//    delay(1);

  }

  delay (3000);

  for (level = 0; level < 256; level++) {
    
    digitalPotWrite (channel, level);
    delay (1);

  }

 //   delay(3000);

}

void digitalPotWrite (int address, int value) {
  
  digitalWrite(slaveSelectPin,LOW);   // take the SS pin low to select the chip:
  
  SPI.transfer(address);     //  send in the address and value via SPI:
  SPI.transfer(value);
  
  digitalWrite(slaveSelectPin,HIGH);     // take the SS pin high to de-select the chip:
}
