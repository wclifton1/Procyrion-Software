#include <SPI.h>

int start = 0, period;
const int slaveSelectPin = 6;    // set pin 10 as the slave select for the digital pot:

void setup () {
  pinMode (slaveSelectPin, OUTPUT);   // set the slaveSelectPin as an output:
  pinMode (10, OUTPUT);   // set the slaveSelectPin as an output:
  // initialize SPI:
  // SPI.setBitOrder (MSBFIRST);
  // SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider (SPI_CLOCK_DIV2); //check to see if this is necessary
  SPI.begin (); 

  digitalPotWrite (0, 125);
}

// the loop routine runs over and over again forever:
void loop () {

  for (int level = 125; level < 256; level += 1) {
    digitalPotWrite (0, level);
    delay(1);
  }

}


void digitalPotWrite (int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}






