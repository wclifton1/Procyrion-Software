// inslude the SPI library:
#include <SPI.h>

/*
  SPI_test
  Tests SPI comms with an AD8403 digital pot
 */

// set pin 10 as the slave select for the digital pot:
const int slaveSelectPin = 10;

void setup() {
  // set the slaveSelectPin as an output:
  pinMode (slaveSelectPin, OUTPUT);
  // initialize SPI:
//  SPI.setBitOrder (MSBFIRST);
 // SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider (SPI_CLOCK_DIV2);
  SPI.begin (); 
}

// the loop routine runs over and over again forever:
void loop() {
  // go through the four channels of the digital pot:
  for (int channel = 0; channel < 1; channel++) { 
    // change the resistance on this channel from min to max:
    for (int level = 0; level < 256; level++) {
      digitalPotWrite(channel, level);
      delay(300);
    }
    // wait a second at the top:
    delay(3000);
    // change the resistance on this channel from max to min:
    for (byte level = 0; level < 256; level++) {
      digitalPotWrite(channel, 255 - level);
      delay(300);
    }
  }

}

void digitalPotWrite(int address, int value) {
  // take the SS pin low to select the chip:
  digitalWrite(slaveSelectPin,LOW);
  //  send in the address and value via SPI:
  SPI.transfer(address);
  SPI.transfer(value);
  // take the SS pin high to de-select the chip:
  digitalWrite(slaveSelectPin,HIGH); 
}
