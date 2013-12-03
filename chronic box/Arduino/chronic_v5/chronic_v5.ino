// V1 runs motor at full speed (supply voltage bypasses arduino)
// calculates and transmits rpm and supply voltages on either side of a current sense resistor
// V2 adds auto restart and serial event handling with start/stop commands (7406 bytes)
// V2 also worked for bluetooth with no change
// V4 corrects current and voltage and power measures
// V5 adds interactive servo control and tries to make RPM work at less than 100% duty cycle

  #include <Servo.h>

  Servo myservo;
  
  volatile unsigned long start = 0, period = 3;
  volatile int settleIn = 0;
  
  int powerVoltageHiPin = A1;
  int powerVoltageLowPin = A0;
  int powerVoltageHi;
  int powerVoltageLow;
  int servoPulseWidth = 100;
  float vhi, vlow, current, power;
  long motorSpeed;
  
  String inputString = "";                             // a string to hold incoming data
  boolean stringComplete = false, wantedOn = true, understood = false;   // whether the string is complete

  void setup () {
    
    myservo.attach (7);  // attaches the servo on pin 9 to the servo object 
    myservo.write (0);
    delay (2000);

    attachInterrupt (0, computeFrequency, FALLING);

    Serial.begin(9600);
    inputString.reserve(200);

  }
  
  void loop () {
   
    delay (200);
    
    if (stringComplete) 
      processInputString (); 
      
    if (!wantedOn)
      myservo.write (0);
    
    if (wantedOn && (period == 3)) {    // auto restart - should maybe change to use millis()
      myservo.write (0); 
      delay (2000);
      myservo.write (servoPulseWidth);
    }
      
    if ((period == 3) || !wantedOn)
      motorSpeed = 0;
    else {
      motorSpeed = 60000000/period;
      period = 3;
    }  
    
    if (settleIn < 5) {
      motorSpeed = 0;
      settleIn++;
    }
     
    powerVoltageHi = analogRead (powerVoltageHiPin);
    powerVoltageLow = analogRead (powerVoltageLowPin);
    
    vhi = powerVoltageHi / 45.6;
    vlow = powerVoltageLow / 45.6;
    current = vhi - vlow;
    power = vlow * current;
    
    Serial.print (vlow);
    Serial.print (" volts; ");
    Serial.print (current);  
    Serial.print (" amps;  ");
    Serial.print (power);  
    Serial.print (" watts;  ");
    Serial.print (motorSpeed);  
    Serial.println (" rpms");
 
  }
  
  void processInputString () {
    
    inputString.trim();
    understood = false;
    
    if (inputString == "start" ) {
      wantedOn = true;
      Serial.println ("Motor should be on");
      settleIn = 0;
      understood  = true;
    }
    
    if (inputString == "stop" ) {
      wantedOn = false;
      Serial.println ("Motor should be off");
      understood = true;
    }
    
    if (inputString == "i") {
      servoPulseWidth += 5;
      if (servoPulseWidth > 180)
        servoPulseWidth = 180;
      myservo.write (servoPulseWidth);  
      Serial.print ("Servo pulse width: ");
      Serial.println (servoPulseWidth);
      understood = true;
    }

    if (inputString == "k") {
      servoPulseWidth -= 5;
      if (servoPulseWidth < 70)
        servoPulseWidth = 70;
      myservo.write (servoPulseWidth);  
      Serial.print ("Servo pulse width: ");
      Serial.println (servoPulseWidth);
      understood = true;
    }
      
    if (!understood)
      Serial.println ("Message not understood");
      
    inputString = "";   // clear the string:
    stringComplete = false;
    
  }
  
  void serialEvent() {
    
    while (Serial.available()) {
      char inChar = (char)Serial.read(); // get the new byte:
      if (inChar == '\n')
        stringComplete = true;    // if the incoming character is a newline, set a flag
      else
        inputString += inChar;     // add it to the inputString:
    }
    
  }
  
  void computeFrequency () {

    if (micros () - start > 100)
      period = micros () - start;
    start = micros ();
  
  }
