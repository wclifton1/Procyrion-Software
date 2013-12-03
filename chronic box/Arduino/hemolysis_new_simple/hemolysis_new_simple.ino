#include <SPI.h>
#include <Servo.h> 

Servo motorServo[4];  // create servo object to control a servo 

volatile unsigned long motor1StartTime = 0, motor2StartTime = 0, motor3StartTime = 0, motor4StartTime = 0; 
volatile unsigned long motor1Period, motor2Period, motor3Period, motor4Period;
int motor1CurrentPhase, motor2CurrentPhase, motor3CurrentPhase, motor4CurrentPhase;
int motor1LastPhase = LOW, motor2LastPhase = LOW, motor3LastPhase = LOW, motor4LastPhase = LOW; 

int pressure;
unsigned long motor2PeriodAvg;
unsigned long motor2PeriodN;
int motor2Throttle = 80;
int motor2Adjustment = 0;

const int motor1Phase = 2;    
const int motor2Phase = 2;
const int motor3Phase = 4;
const int motor4Phase = 5;
const int slaveSelectPin = 6;    // set pin 6 as the slave select for the digital pot:

void setup () {

  pinMode (slaveSelectPin, OUTPUT);   // set the slaveSelectPin as an output:
  pinMode (motor1Phase, INPUT);	   
  pinMode (motor2Phase, INPUT);
  pinMode (motor3Phase, INPUT);
  pinMode (motor4Phase, INPUT);

  SPI.setClockDivider (SPI_CLOCK_DIV2); //check to see if this is necessary
  SPI.begin (); 

  for (int i=0; i<3; i++) {
    digitalPotWrite (i, 100);
    motorServo[i].attach (i+7);
    motorServo[i].write (0);
  }

  delay (2000);

  for (int i=0; i<3; i++) 
    digitalPotWrite (i, 100);

  Serial.begin(9600);      // open the serial port at 9600 bps:   

  for (int channel = 0; channel < 4; channel++)
    motorServo[channel].write(180);

  delay (5000);  

  for (int channel = 0; channel < 4; channel++)
    motorServo[channel].write(0);

  InitialiseInterrupt();
}


void loop () {

  for (int channel = 0; channel < 4; channel++) { 

    motorServo[2].write(0);
    digitalPotWrite(2, motor2Throttle);
    motor2PeriodAvg = 0;
    motor2PeriodN = 0;

    for (int i = 0; i < 250; i++) {
      if ((motor2Period > 300) && (motor2Period > 80)) {
        motor2PeriodAvg += motor2Period;
        motor2PeriodN++;
        motor2Period = -1;
      }
      delay(3);
    }

    if (motor2PeriodN < 50) {
      Serial.println ("Motor Stopped!");
      motor2Throttle = 80;
      motorServo[2].write(0);
      delay (1000);
      motorServo[2].write(180);
      delay (1000);
    }
    else {
      motor2PeriodAvg = motor2PeriodAvg / motor2PeriodN;
      motor2Throttle += (motor2PeriodAvg - 500) / 4;
      if (motor2Throttle < 70) motor2Throttle = 70;
      if (motor2Throttle > 180) motor2Throttle = 180;
    }

    Serial.print (motor2CurrentPhase);
    Serial.print ("     ");
    Serial.print (motor2PeriodAvg);
    Serial.print ("     ");
    Serial.print (motor2PeriodN);
    Serial.print ("     ");
    Serial.print (motor2PeriodAvg);
    Serial.print ("     ");
    Serial.println (motor2Throttle);

    motorServo[channel].write (0);
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

  motor2Period = micros () - motor2StartTime;
  motor2StartTime = micros ();

  /*  motor1CurrentPhase = digitalRead (motor1Phase);
   if ((motor1LastPhase == HIGH) && (motor1CurrentPhase == LOW)) {
   motor1Period = micros () - motor1StartTime;
   motor1StartTime = micros ();  
   }   
   motor1LastPhase = motor1CurrentPhase;
   */
  motor2CurrentPhase = digitalRead (motor2Phase);
//  if ((motor2LastPhase == HIGH) && (motor2CurrentPhase == LOW)) {
//    motor2Period = micros () - motor2StartTime;
//    motor2StartTime = micros ();  
//  }   
//  motor2LastPhase = motor2CurrentPhase;
  /*
  motor3CurrentPhase = digitalRead (motor3Phase);
   if ((motor3LastPhase == HIGH) && (motor3CurrentPhase == LOW)) {
   motor3Period = micros () - motor3StartTime;
   motor3StartTime = micros ();  
   }   
   motor3LastPhase = motor3CurrentPhase;
   
   motor4CurrentPhase = digitalRead (motor4Phase);
   if ((motor4LastPhase == HIGH) && (motor4CurrentPhase == LOW)) {
   motor4Period = micros () - motor4StartTime;
   motor4StartTime = micros ();  
   }   
   motor4LastPhase = motor4CurrentPhase;
   */
}







