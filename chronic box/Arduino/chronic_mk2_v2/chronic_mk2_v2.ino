//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// mk1v1 -runs motor at full speed (supply voltage bypasses arduino)
//       -calculates and transmits rpm and supply voltages on either side of a current sense resistor
// mk1v2 adds auto restart and serial event handling with start/stop commands (7406 bytes)
//       works for bluetooth with no change
// mk1v4 corrects current and voltage and power measures
// mk1v5 adds interactive servo control and tries to make RPM work at less than 100% duty cycle
// mk1v7 adds averaging and switched power resistor to have just phoenix on current sense and calibrates voltage measurement
// 
// mk2 will increase update rate, fix restart (by limiting it), add SD card support, add speed control, add the display, and 
// add local RPM controls
//
// mk2v1 basic function of mk1
// mk2v2 add LCD support and reading of local switch to control RPM set point
//
// todo: remove unnecessary test of period == 3; maybe use a specific RPM output to indicate no valid RPM measure on display
//       use h/w indication of unit number (with AI high or low) to set resistor caliration values
//       make output verbose again
//       make stop after 5 restart attempts and require button press to try another five
//       fix settleIn coming after information already incorporated into average
//       motor jitters when seized and this reads as valid RPMs
//       motor haltingly starts when powered by USB alone (shouldn't try to run at all -- just set limit on voltage
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  #include <Servo.h>      // RC-style servo for Phoenix motor controller
  #include <LiquidCrystal.h>

  Servo myservo;          // RC-style servo for Phoenix motor controller
  LiquidCrystal lcd (3, 4, 5, 8, 9, 10);         // RS, EN, DB4, DB5, DB6, DB7
    
  const int powerVoltageHiPin = A0;     // analog input for battery side of the current sense resistor
  const int powerVoltageLowPin = A1;    // analog input for Phoenix side of the current sense resistor
  const int RPMControlPin = A5;         // analog input for RMP up/down control switch
  const int RPMMeasurePin = 2;          // digital in for RPM determination (not used in code, but here for completeness)
  const int servoOutputPin = 7;         // digital out for servo signal to Phoenix

  const float hiVoltageCalibration = 46.77;  // battery side ticks/V
  const float lowVoltageCalibration = 47.23; // Phoenix side ticks/V 
  const int cyclesToAverage = 50;
  const int lowMidThresh = 338;
  const int midHighThresh = 849;
  const int RPMSetPointDelta = 1000;
  const int RPMChangePeriod = 300;
  
  int servoPulseWidth = 100;            // initial servo pulse width in 10s of usec; motor starts running at about 70 and maxes out at 120 or so 
  int RPMControlVoltage;
  long RPMSetPoint = 75000;
  long lastRPMChange;
  float vhigh, vlow, current, power;    // voltages in volts, current in amps, power in watts
  float vlowAverage = 0, currentAverage = 0, powerAverage = 0, electricalN = 0;  // electricalN is the valid number of measurements to average 
  volatile unsigned long start = 0, period = 3;    // measure period in usec for RPM determination; 3 signifies no valid information (may switch to 0 if possible)
  long motorSpeed, motorSpeedAverage = 0, motorSpeedN = 0;                       // RPM data; motorSpeedN is the valid number of measurements to average
  int restartAttempts = 0;
  int settleIn = 0;                       // settleIn counts a period during which RPM determination is wrong and output is suppressed
  
  String inputString = "";                                                                       // a string to hold incoming data
  boolean stringComplete = false, wantedOn = true, understood = false, motorSpinning = false;   // whether the string is complete

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void setup () {
    
    myservo.attach (servoOutputPin);  // attaches the servo on pin 9 to the servo object 
    myservo.write (0);                // set initial throttle value to zero
    delay (2000);                     // zero throttle for "some time" required to initialize the Phoenix (1500 ms only worked some of the time)

    attachInterrupt (0, computeFrequency, FALLING);  // this calls computeFrequency on every falling transition on D2

    Serial.begin (9600);              
    inputString.reserve (200);
    
    lcd.begin (16, 2);
    lcd.print ("hello, world!");
    lcd.setCursor (0, 1);              // (Column, Row)
    lcd.print ("This is chronic!");

  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void loop () {
   
    delay (25);              // takes a long time to start (like a minute) at 10, seems to work fine all the time at 25
    
    if (stringComplete)       // check for commands from a host PC over the serial connection (bluetooth or wired)
      processInputString (); 
    
    if (millis () - lastRPMChange > RPMChangePeriod)
      readRPMControl ();
      
    if (!wantedOn)            
      myservo.write (0);      // set throttle to zero if desired state is off
    
    if (wantedOn && (period == 3)) {        // auto restart - if the desired state is on but there is no valid RPM data a restart is attempted
      myservo.write (0);                    // set throttle to zero
      delay (2000);                         // wait two seconds
      myservo.write (servoPulseWidth);      // set throttle to last desired setting
      restartAttempts++;                    // track number of restart attempts
    }
      
    if ((period == 3) || !wantedOn)         
      motorSpeed = 0;                          // if the motor is commanded off or there is no valid RPM data, then show zero as motor RPM
    else {
      if (60000000/period < 200000) {          // values over 200k are erroneous - perhaps from beeps; valid zeroes make it through this
        motorSpeed = 60000000/period;          // 1/period is rotations/usec; 1,000,000x to covert to rotations/sec; 60x to convert to RPM
        motorSpeedAverage += motorSpeed;        
        motorSpeedN++;
        motorSpinning = motorSpeed > 5000;     // track whether motor is spinning (5000 allows for valid zeroes corrupted by odd beeps or such
      }
      period = 3;                              // set period = 3 as a flag that new period information is needed
    }  
    
    if (settleIn < 5) {                        // this is to suppress possibly erroneuous values after commanding motor on
      motorSpeed = 0;                          // but it's wrong here since information has already been incorporated into average above
      settleIn++;
    }
    
    measureElectricals ();
    
    if (electricalN == cyclesToAverage) 
      sendData ();
 
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

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
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void readRPMControl () {
    
    RPMControlVoltage = analogRead (RPMControlPin);
    
    if (RPMControlVoltage > lowMidThresh) 
      if (RPMControlVoltage < midHighThresh) 
        RPMSetPoint -= RPMSetPointDelta;
      else
        RPMSetPoint += RPMSetPointDelta;
        
    lcd.setCursor (0, 1);              // (Column, Row)
    lcd.print ("Set: ");
    lcd.print (RPMSetPoint);
    lastRPMChange = millis ();
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  
  void measureElectricals () {
    
    vlow = analogRead (powerVoltageHiPin) / lowVoltageCalibration;          // read voltage on battery side of current sense resistor
    vhigh = analogRead (powerVoltageLowPin) / hiVoltageCalibration;        // read voltage on Phoenix side of current sense resistor 
    current = vhigh - vlow;                        // calculate current into Phoenix
    power = vlow * current;                      // calculate power into Phoenix
    
    vlowAverage += vlow;                         // update electrical averages
    currentAverage += current;
    powerAverage += power;
    electricalN++;
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void sendData () {
    
    if (motorSpinning) {
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
      
      Serial.print (vlow);
      Serial.print (" volts; ");
      Serial.print (current);  
      Serial.print (" amps;  ");
      Serial.print (power);  
      Serial.print (" watts;  ");
      Serial.print (motorSpeed);  
      Serial.println (" rpms");
      
      lcd.clear ();             // (Column, Row)
      lcd.print ("RPM: ");
      lcd.print (motorSpeed);
      
      motorSpeedAverage = 0;
      motorSpeedN = 0;
      currentAverage = 0;
      vlowAverage = 0;
      powerAverage = 0;
      electricalN = 0;
      
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void serialEvent() {
    
    while (Serial.available()) {
      char inChar = (char)Serial.read(); // get the new byte:
      if (inChar == '\n')
        stringComplete = true;    // if the incoming character is a newline, set a flag
      else
        inputString += inChar;     // add it to the inputString:
    }
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void computeFrequency () {

    if (micros () - start > 100)
      period = micros () - start;
    start = micros ();
  
  }
