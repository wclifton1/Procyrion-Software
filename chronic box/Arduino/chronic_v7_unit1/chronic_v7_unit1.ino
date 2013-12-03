// V1 runs motor at full speed (supply voltage bypasses arduino)
// calculates and transmits rpm and supply voltages on either side of a current sense resistor
// V2 adds auto restart and serial event handling with start/stop commands (7406 bytes)
// V2 also worked for bluetooth with no change
// V4 corrects current and voltage and power measures
// V5 adds interactive servo control and tries to make RPM work at less than 100% duty cycle
// V7 adds averaging and switched power resistor to have just phoenix on current sense and calibrates voltage measurement

  #include <Servo.h>

  Servo myservo;
  
  volatile unsigned long start = 0, period = 3;
  volatile int settleIn = 0;
  
  int powerVoltageHiPin = A0;
  int powerVoltageLowPin = A1;
  int powerVoltageHi;
  int powerVoltageLow;
  int servoPulseWidth = 100;
  float vhi, vlow, current, power, vlowAverage = 0, currentAverage = 0, powerAverage = 0, electricalN = 0;
  long motorSpeed, motorSpeedAverage = 0, motorSpeedN = 0;
  
  String inputString = "";                             // a string to hold incoming data
  boolean stringComplete = false, wantedOn = true, understood = false, motorConnected = false;   // whether the string is complete

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
      if (60000000/period < 200000) {
        motorSpeed = 60000000/period;
        motorSpeedAverage += motorSpeed;
        motorSpeedN++;
        motorConnected = motorSpeed > 5000;
      }
      period = 3;
    }  
    
    if (settleIn < 5) {
      motorSpeed = 0;
      settleIn++;
    }
     
    powerVoltageHi = analogRead (powerVoltageHiPin);
    powerVoltageLow = analogRead (powerVoltageLowPin);
    
    vlow = powerVoltageLow / 47.23;
    
//    if ((period == 3) || !wantedOn) {
 //     current = 0;
 //     power = 0;
 //   }
 //   else {
      vhi = powerVoltageHi / 46.77;
      current = vhi - vlow;
      power = vlow * current;
 //   }
    
    vlowAverage += vlow;
    currentAverage += current;
    powerAverage += power;
    electricalN++;
    
    if (motorSpeedN == 25) {
      
      if (motorConnected) {
        motorSpeedAverage /= motorSpeedN;
        currentAverage /= electricalN;
        vlowAverage /= electricalN; 
        powerAverage /= electricalN; 
      }
      else {
        motorSpeedAverage = 0;
        currentAverage = 0;
        vlowAverage /= electricalN; 
        powerAverage = 0; 
      }
      
      if ((currentAverage < 0) || (powerAverage < 0)) {
        motorSpeedAverage = 0;
        currentAverage = 0;
        powerAverage = 0; 
      }
      
      Serial.print ("1,");
      Serial.print (motorSpeedAverage);  
      Serial.print (",");
      Serial.print (int(currentAverage*1000));  
      Serial.print (",");
      Serial.print (int(vlowAverage*1000));
      Serial.print (",");
      Serial.print (int(powerAverage*1000));  
      Serial.println (",");
      
      motorSpeedAverage = 0;
      motorSpeedN = 0;
      currentAverage = 0;
      vlowAverage = 0;
      powerAverage = 0;
      electricalN = 0;
      
    }
 
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
