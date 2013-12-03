#include <SPI.h>
#include <Servo.h> 

Servo motorServo[4];  // create servo object to control a servo 

volatile unsigned long motorStartTime[4] = {0,0,0,0}, motor2FakeStartTime = 0; 
volatile unsigned long motorPeriod[4], motor2FakePeriod;
volatile int motorCurrentPhase[4];
volatile int motorLastPhase[4] = {LOW,LOW,LOW,LOW}; 

int pressure;
unsigned long motorPeriodAvg[4];
unsigned long motorPeriodN[4];
int motorThrottle[4] = {80,80,80,80};
int motorStoppedN[4] = {0,0,0,0};
int motorAdjustment[4];

int motorPhase[4] = {5,3,2,4};

const int slaveSelectPin = 6;    // set pin 6 as the slave select for the digital pot:

void setup () {

  pinMode (slaveSelectPin, OUTPUT);   // set the slaveSelectPin as an output:

  // initialize SPI:
  // SPI.setBitOrder (MSBFIRST);
  // SPI.setDataMode (SPI_MODE0);
  SPI.setClockDivider (SPI_CLOCK_DIV2); //check to see if this is necessary
  SPI.begin (); 

  for (int i=0; i<3; i++) {
    pinMode (motorPhase[i], INPUT);
    digitalPotWrite (i, 100);
    motorServo[i].attach (i+7);
    motorServo[i].write (0);
  }
  
  delay (2000);
  
  for (int i=0; i<3; i++) 
    digitalPotWrite (i, 100);
  
  //  attachInterrupt (0, computeFrequency, FALLING);

  Serial.begin(9600);      // open the serial port at 9600 bps:   

  for (int channel = 0; channel < 4; channel++)
    motorServo[channel].write(180);
    
  delay (5000);  

//  for (int channel = 0; channel < 4; channel++)
//    motorServo[channel].write(0);
    
  InitialiseInterrupt();
}

// the loop routine runs over and over again forever:
void loop () {
  // go through the four channels of the digital pot:
  
//  for (int level = 100; level < 256; level++) {  

 //   delay(10);
 // }

//  for (int channel = 0; channel < 4; channel++)
//    motorServo[channel].write(0);

  Serial.println ();  

  for (int channel = 1; channel < 3; channel++) { 

    //    myservo.write(180);              // tell servo to go to position in variable 'pos' 
    //    delay(3000);
    //    myservo.write(0);              // tell servo to go to position in variable 'pos' 
    
//        motorServo[channel].write(180);

        motorPeriodAvg[channel] = 0;
        motorPeriodN[channel] = 0;
//        delay(1000);
        
    // change the resistance on this channel from min to max:
        for (int i = 0; i < 250; i++) {
          if ((motorPeriod[channel] > 300) && (motorPeriod[channel] < 8000)) {
            motorPeriodAvg[channel] += motorPeriod[channel];
            motorPeriodN[channel]++;
            motorPeriod[channel] = -1;
          }
        delay(1);
        }
        
        if (motorPeriodN[channel] < 50) {
          Serial.print ("M");
          Serial.print (channel);
          Serial.print (": [stopped (");
          Serial.print (motorStoppedN[channel]++);
          Serial.print (")] ");
          motorServo[channel].write(0);
          if (motorStoppedN[channel] > 3) {
            motorThrottle[channel] = 80;
            motorServo[channel].write(180);
            motorStoppedN[channel] = 0;
          }
        }
        else {
          motorPeriodAvg[channel] = motorPeriodAvg[channel] / motorPeriodN[channel];
          motorAdjustment[channel] = (motorPeriodAvg[channel] - 900) / 8;  //8 is working well for 60krpm, expect 16 for 35krpm 
          motorThrottle[channel] += motorAdjustment[channel];
          if (motorThrottle[channel] < 70) motorThrottle[channel] = 60;
          if (motorThrottle[channel] > 180) motorThrottle[channel] = 180;
          digitalPotWrite(channel, motorThrottle[channel]);
     
        Serial.print ("M");
        Serial.print (channel);
        Serial.print (": [");
        Serial.print (motorPeriodAvg[channel]);
        Serial.print (", ");
        Serial.print (motorThrottle[channel]);
        Serial.print ("] ");
        }

 //   for (int level = 100; level < 256; level++) {  
 //     digitalPotWrite (channel, 0);
 //     delay(2);

      //     Serial.print ("************* New level of ");
      //     Serial.println (level);
//      for (int i = 0; i < 20; i++) {
//        Serial.print (motor1Period);
//        Serial.print ("     ");
//        Serial.print (motor2Period);
//        Serial.print ("     ");
//        Serial.print (motor3Period);
//        Serial.print ("     ");
//        Serial.println (motor4Period);
//        motor1Period = -1;
//        motor2Period = -1;
//        motor3Period = -1;
//        motor4Period = -1;
//        delay (1);

        //        pressure = analogRead (A0);    // read the input pin
        //        Serial.print ("Pressure: ");
        //        Serial.println (pressure);             // debug value

      
 //   }
    //   myservo.write(180);              // tell servo to go to position in variable 'pos' 
    //   delay(1000);
    //  myservo.write(0);              // tell servo to go to position in variable 'pos' 
    //  delay(1000);

    // change the resistance on this channel from max to min:
    //    for (int level = 0; level < 256; level++) {
    //      digitalPotWrite(channel, 255 - level);
    //   delay(1);
    //    }
    
//    motorServo[channel].write (0);
  }

}

void InitialiseInterrupt () {
  cli();		// switch interrupts off while messing with their settings  
  PCICR =0x04;          // Enable PCINT2 (Port D - PCINT[23:16]) interrupt
  PCMSK2 = 0b00111100;  // b2 is D2; b3 is D3; b4 is D4; b5 is D5
  sei();		// turn interrupts back on
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

ISR (PCINT2_vect) {

  motor2FakePeriod = micros () - motor2FakeStartTime;
  motor2FakeStartTime = micros ();
  
  for (int i=0; i<4; i++) {
    
    motorCurrentPhase[i] = digitalRead (motorPhase[i]);
    if ((motorLastPhase[i] == HIGH) && (motorCurrentPhase[i] == LOW)) {
      motorPeriod[i] = micros () - motorStartTime[i];
      motorStartTime[i] = micros ();  
    }   
    motorLastPhase[i] = motorCurrentPhase[i];

  }
}






