#include <SPI.h>
#include <Servo.h> 

Servo myservo;  // create servo object to control a servo 

int start = 0, period, pressure;
const int slaveSelectPin = 6;    // set pin 10 as the slave select for the digital pot:

void setup () {
  
  pinMode (slaveSelectPin, OUTPUT);   // set the slaveSelectPin as an output:
  // initialize SPI:
  // SPI.setBitOrder (MSBFIRST);
  // SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider (SPI_CLOCK_DIV2); //check to see if this is necessary
  SPI.begin (); 

  myservo.attach (7);  // attaches the servo on pin 9 to the servo object 
  digitalPotWrite (3, 125);
  myservo.write (0);
  delay (500);
  myservo.write(180);
//  myservo.detach ();

  attachInterrupt (0, computeFrequency, FALLING);

  Serial.begin(9600);      // open the serial port at 9600 bps:   
}

// the loop routine runs over and over again forever:
void loop () {
  // go through the four channels of the digital pot:
  for (int channel = 0; channel < 1; channel++) { 

    //    myservo.write(180);              // tell servo to go to position in variable 'pos' 
    //    delay(3000);
    //    myservo.write(0);              // tell servo to go to position in variable 'pos' 

    // change the resistance on this channel from min to max:
    //    for (int level = 0; level < 256; level++) {
    //      digitalPotWrite(channel, level);
    //    delay(1);
    //    }

    for (int level = 125; level < 256; level += 10) {
      digitalPotWrite (3, 100);
      Serial.print ("************* New level of ");
      Serial.println (level);
      for (int i = 0; i < 20; i++) {
        Serial.println (period);
        period = -1;
        delay (1);
        
//        pressure = analogRead (A0);    // read the input pin
//        Serial.print ("Pressure: ");
//        Serial.println (pressure);             // debug value
        
      }
    }
    //   myservo.write(180);              // tell servo to go to position in variable 'pos' 
    //   delay(1000);
    //  myservo.write(0);              // tell servo to go to position in variable 'pos' 
    //  delay(1000);

    // change the resistance on this channel from max to min:
    //    for (int level = 0; level < 256; level++) {
    //      digitalPotWrite(channel, 255 - level);
    //   delay(1);
    //    }
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

void computeFrequency () {
  period = micros () - start;
  start = micros ();
}



