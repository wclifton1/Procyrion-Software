#include <SPI.h>
#include <Servo.h> 

Servo motorServo[4];  // create servo object to control a servo 

volatile int motor0StartTime = 0, motor1StartTime = 0, motor2StartTime = 0, motor3StartTime = 0; 
volatile int motor0Period, motor1Period, motor2Period, motor3Period;
volatile int motor0CurrentPhase, motor1CurrentPhase, motor2CurrentPhase, motor3CurrentPhase;
volatile int motor0LastPhase = LOW, motor1LastPhase = LOW, motor2LastPhase = LOW, motor3LastPhase = LOW; 

const int motor0Phase = 5;    
const int motor1Phase = 3;
const int motor2Phase = 2;
const int motor3Phase = 4;

const int slaveSelectPin = 6;    // set pin 6 as the slave select for the digital pot:

void setup () {

  pinMode (slaveSelectPin, OUTPUT);   // set the slaveSelectPin as an output:
  
  pinMode (motor0Phase, INPUT);	   
  pinMode (motor1Phase, INPUT);
  pinMode (motor2Phase, INPUT);
  pinMode (motor3Phase, INPUT);

  SPI.setClockDivider (SPI_CLOCK_DIV2); //check to see if this is necessary
  SPI.begin (); 

  for (int i=0; i<3; i++) {
    motorServo[i].attach (i+7);
    motorServo[i].write (0);
    digitalPotWrite (i, 100);
  }
  
  delay (2000);
  
  //  attachInterrupt (0, computeFrequency, FALLING);
  InitialiseInterrupt();

  Serial.begin(9600);      // open the serial port at 9600 bps:   

  for (int channel = 0; channel < 4; channel++)
    motorServo[channel].write(180);
    
  delay (5000);  

  for (int channel = 0; channel < 4; channel++)
    motorServo[channel].write(0);
      
}


void loop () {
  
  for (int channel = 0; channel < 4; channel++) { 

    motorServo[2].write(180); //use this to turn on the motor

    // change the resistance on this channel from min to max:
        for (int level = 90; level < 256; level++) {
          digitalPotWrite(2, 120);
        delay(3);
        }
 
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

  digitalWrite(slaveSelectPin,LOW);
  SPI.transfer(address);
  SPI.transfer(value);
  digitalWrite(slaveSelectPin,HIGH); 

}

ISR (PCINT2_vect) {

  motor0CurrentPhase = digitalRead (motor0Phase);
  if ((motor0LastPhase == HIGH) && (motor0CurrentPhase == LOW)) {
    motor0Period = micros () - motor0StartTime;
    motor0StartTime = micros ();  
  }   
  motor0LastPhase = motor0CurrentPhase;

  motor1CurrentPhase = digitalRead (motor1Phase);
  if ((motor1LastPhase == HIGH) && (motor1CurrentPhase == LOW)) {
    motor1Period = micros () - motor1StartTime;
    motor1StartTime = micros ();  
  }   
  motor1LastPhase = motor1CurrentPhase;

  motor2CurrentPhase = digitalRead (motor2Phase);
  if ((motor2LastPhase == HIGH) && (motor2CurrentPhase == LOW)) {
    motor2Period = micros () - motor2StartTime;
    motor2StartTime = micros ();  
    Serial.println (motor2Period);
  }   
  motor2LastPhase = motor2CurrentPhase;

  motor3CurrentPhase = digitalRead (motor3Phase);
  if ((motor3LastPhase == HIGH) && (motor3CurrentPhase == LOW)) {
    motor3Period = micros () - motor3StartTime;
    motor3StartTime = micros ();  
  }   
  motor3LastPhase = motor3CurrentPhase;

}






