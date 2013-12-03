// V1 runs motor at full speed (supply voltage bypasses arduino)
// calculates and transmits rpm and supply voltages on either side of a current sense resistor
// V2 adds auto restart and serial event handling with start/stop commands (7406 bytes)
// V3 moves communication to bluetooth -- no change

  #include <Servo.h>

  Servo myservo;
  
  volatile unsigned long start = 0, period = 3;
  volatile int settleIn = 0;
  
  int powerVoltageHiPin = A0;
  int powerVoltageLowPin = A1;
  int powerVoltageHi;
  int powerVoltageLow;
  long motorSpeed;
  
  String inputString = "";                             // a string to hold incoming data
  boolean stringComplete = false, wantedOn = false;   // whether the string is complete

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
      myservo.write (180);
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
    Serial.print (powerVoltageHi);
    Serial.print ("   ");
    Serial.print (powerVoltageLow);  
    Serial.print ("   ");
    Serial.println (motorSpeed);
 
  }
  
  void computeFrequency () {

    period = micros () - start;
    start = micros ();
  
  }
  
  void processInputString () {
    
    inputString.trim();
    
    if (inputString == "start" ) {
      wantedOn = true;
      Serial.println ("Motor should be on");
      settleIn = 0;
    }
    else if (inputString == "stop" ) {
      wantedOn = false;
      Serial.println ("Motor should be off");
    }
    else 
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
