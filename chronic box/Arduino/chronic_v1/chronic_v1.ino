// V1 runs motor at full speed (supply voltage bypasses arduino)
// calculates and transmits rpm and supply voltages on either side of a current sense resistor
// no auto restart

  #include <Servo.h>

  Servo myservo;
  
  volatile unsigned long start = 0, period;
  int powerVoltageHiPin = A0;
  int powerVoltageLowPin = A1;
  int powerVoltageHi;
  int powerVoltageLow;

  void setup () {
    
    myservo.attach (7);  // attaches the servo on pin 9 to the servo object 
    myservo.write (0);
    delay (2000);

    attachInterrupt (0, computeFrequency, FALLING);

    Serial.begin(9600);  

  }
  
  void loop () {
   
    for (int i=100; i<140; i++) {
      myservo.write (180);  //73 seems to be the lowest that will work
      delay (5);
    }
   
  powerVoltageHi = analogRead (powerVoltageHiPin);
  powerVoltageLow = analogRead (powerVoltageLowPin);
  Serial.print (powerVoltageHi);
  Serial.print ("\t");
  Serial.print (powerVoltageLow);  
  Serial.print ("\t");
  Serial.println (60000000/period);
  period = -1;
    
  }
  
  void computeFrequency () {

    period = micros () - start;
    start = micros ();

}
