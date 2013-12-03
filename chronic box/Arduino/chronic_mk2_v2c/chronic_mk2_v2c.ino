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
//       make stop after 5 restart attempts and require button press to try another five
//       fix settleIn coming after information already incorporated into average
//       motor jitters when seized and this reads as valid RPMs
//       motor haltingly starts when powered by USB alone (shouldn't try to run at all -- just set limit on voltage)
//       really should be averaging period, not RPMs
//       power, voltage, current all screwy
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  #include <Servo.h>      // RC-style servo for Phoenix motor controller
  #include <LiquidCrystal.h>

  Servo myservo;          // RC-style servo for Phoenix motor controller
  const int servoOutputPin = 7;         // digital out for servo signal to Phoenix
  const int minServoPulseWidth = 75;
  const int maxServoPulseWidth = 180;
  const int servoPulseWidthKeyboardDelta = 10;
  
  LiquidCrystal lcd (3, 4, 5, 8, 9, 10);         // RS, EN, DB4, DB5, DB6, DB7
    
  const int powerVoltageHighPin = A0;     // analog input for battery side of the current sense resistor
  const int powerVoltageLowPin = A1;    // analog input for Phoenix side of the current sense resistor
  const float hiVoltageCalibration = 46.33;  // battery side ticks/V
  const float lowVoltageCalibration = 46.78; // Phoenix side ticks/V 
  float vhigh, vlow, current, power;    // voltages in volts, current in amps, power in watts
  float vlowAverage = 0, vhighAverage = 0, currentAverage = 0, powerAverage = 0, electricalN = 0;  // electricalN is the valid number of measurements to average 

  const int RPMMeasurePin = 2;          // digital in for RPM determination (not used in code, but here for completeness)
  const long validMotorSpeedLowerLimit = 15000;
  volatile unsigned long start = 0, period;    // measure period in usec for RPM determination; 3 signifies no valid information (may switch to 0 if possible)
  float motorSpeed, motorSpeedSum = 0, motorSpeedN = 0;                       // RPM data; motorSpeedN is the valid number of measurements to average
  float motorSpeedAverage = 0.0, oldMotorSpeedAverage = 0.0;

  const int RPMControlPin = A5;         // analog input for RMP up/down control switch
  const int lowMidThresh = 338;
  const int midHighThresh = 849;
  int RPMControlVoltage;
  const long RPMChangePeriod = 300;
  const float RPMSetPointDelta = 1000.0;
  unsigned long lastRPMChange;
  
  boolean wantedOn = true, motorSpinning = false;
  int servoPulseWidth = 120, oldServoPulseWidth = 120;            // initial servo pulse width in 10s of usec; motor starts running at about 70 and maxes out at 120 or so 

  const float proportionalGain = 0.65;    // 0.65
  const float integralGain = 0.02;         // 0.1
  const float integrationInterval = 2.0;  // 2.0
  const int cyclesToAverage = 10;  // put back to 10
  const float maxDeltaSPW = ((float) (maxServoPulseWidth - minServoPulseWidth)) / 2.0;
  const float minDeltaSPW = -maxDeltaSPW;
  float RPMSetPoint = 75000.0;
  float integratedError = 0.0;
  float RPMPerTick = 2999.0, deltaSPW = 0.0;
  
  const int startDelay = 2000;
  const int restartAttemptsLimit = 5;
  int restartAttempts = 0;
  unsigned long restartStartTime;
  boolean maxedRestarts = false, restartInProgress = false;   
  int settleIn = 0;                       // settleIn counts a period during which RPM determination is wrong and output is suppressed
  
  String inputString = "";                                                                       // a string to hold incoming data
  boolean stringComplete = false, understood = false;
  
  const int loopDelayTime = 10;
  const long baudRate = 9600;
  const unsigned int inputStringSize = 200;
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void setup () {
    
    myservo.attach (servoOutputPin);  // attaches the servo on pin 9 to the servo object 

    attachInterrupt (0, computeFrequency, FALLING);  // this calls computeFrequency on every falling transition on D2

    Serial.begin (baudRate);              
    inputString.reserve (inputStringSize);
    
    lcd.begin (16, 2);
    lcd.clear ();
   
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void loop () {
   
    if (!wantedOn)            
      myservo.write (0);      // set throttle to zero if desired state is off
    else
      if (!motorSpinning && !maxedRestarts)         // auto restart - if the desired state is on but there is no valid RPM data a restart is attempted
        restart ();  
      
    if (stringComplete)       // check for commands from a host PC over the serial connection (bluetooth or wired)
      processInputString (); 
    
    if ((long) (millis () - lastRPMChange) > RPMChangePeriod)
      readRPMControl ();
      
    computeRPM ();

    measureElectricals ();
    if (electricalN == cyclesToAverage) {
      sendData ();
      if ((wantedOn)  && (motorSpinning) && (!maxedRestarts)) // i had period != 3 as another condition but removed it to find what's killing the motor in water
        controlSpeed();
    }
      
    delay (loopDelayTime);              // takes a long time to start (like a minute) at 10, seems to work fine all the time at 15 
 
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void restart () {
    
    if (restartAttempts <= restartAttemptsLimit)
      if (restartInProgress) {
        if ((int) (millis () - restartStartTime) > startDelay) {
          Serial.println ("throttle back up");
          myservo.write (servoPulseWidth);      // set throttle to last desired setting
          delay (25);
          restartInProgress = false;
        }
      }
      else {
        myservo.write (0);                    // set throttle to zero
        restartStartTime = millis ();                         // wait two seconds
        restartInProgress = true;
        restartAttempts++;                    // track number of restart attempts, make sure to increment with throttle at zero
        Serial.print ("throttle to zero, restart attempt ");
        Serial.println (restartAttempts);
      }
    else {
      maxedRestarts = true;
      Serial.println ("Restart attempt limit exceeded; change RPM set point to reset"); 
      lcd.clear ();
      lcd.print ("maxed restarts");
      lcd.setCursor (0,1);
      lcd.print ("move setpoint");
    }
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
      servoPulseWidth += servoPulseWidthKeyboardDelta;
      if (servoPulseWidth > maxServoPulseWidth)
        servoPulseWidth = maxServoPulseWidth;
      myservo.write (servoPulseWidth);  
      Serial.print ("Servo pulse width: ");
      Serial.println (servoPulseWidth);
      understood = true;
    }

    if (inputString == "k") {
      servoPulseWidth -= servoPulseWidthKeyboardDelta;
      if (servoPulseWidth < minServoPulseWidth)
        servoPulseWidth = minServoPulseWidth;
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
    
    if (RPMControlVoltage > lowMidThresh) {
      
      if (restartAttempts > restartAttemptsLimit) {
        restartAttempts = 0;
        maxedRestarts = false;
      }
      
      if (RPMControlVoltage < midHighThresh) 
        RPMSetPoint -= RPMSetPointDelta;
      else
        RPMSetPoint += RPMSetPointDelta;
        
      lcd.setCursor (0, 1);              // (Column, Row)
      lcd.print ("Set: ");
      lcd.print (RPMSetPoint);
      lcd.print ("    ");
      lastRPMChange = millis ();
    }
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////  
  
  void computeRPM () {
    
    motorSpeed = 60000000.0 / (float) period;          // 1/period is rotations/usec; 1,000,000x to covert to rotations/sec; 60x to convert to RPM
    motorSpeedSum += motorSpeed;        
    motorSpeedN++;
    period = -1;
//    }
    
//    if (settleIn < 5) {                        // this is to suppress possibly erroneuous values after commanding motor on
//      motorSpeed = 0;                          // but it's wrong here since information has already been incorporated into average above
//      settleIn++;
//    }
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
  
  void controlSpeed () {
    
    if ((servoPulseWidth - oldServoPulseWidth) != 0)
      RPMPerTick = (motorSpeedAverage - oldMotorSpeedAverage) / (float) (servoPulseWidth - oldServoPulseWidth);
    if (RPMPerTick < 0.0)
      RPMPerTick *= -1.0;
    if (RPMPerTick > 3000.0)
      RPMPerTick = 3000.0;
    
    deltaSPW = proportionalGain * (RPMSetPoint - motorSpeedAverage) / RPMPerTick;
    if (deltaSPW > maxDeltaSPW)
      deltaSPW = maxDeltaSPW;
    if (deltaSPW < minDeltaSPW)
      deltaSPW = minDeltaSPW; 
    
    if ((deltaSPW < integrationInterval) && (deltaSPW > -integrationInterval)) {
      integratedError += RPMSetPoint - motorSpeedAverage;
      deltaSPW += integralGain * integratedError / RPMPerTick;
    }
    else
      integratedError = 0.0;
    
    if (servoPulseWidth + (int) deltaSPW > maxServoPulseWidth )
      deltaSPW = (float) (maxServoPulseWidth - servoPulseWidth);
    if (servoPulseWidth + (int) deltaSPW < minServoPulseWidth )
      deltaSPW = (float) (minServoPulseWidth - servoPulseWidth); 

    oldMotorSpeedAverage = motorSpeedAverage;
    oldServoPulseWidth = servoPulseWidth;
 
    servoPulseWidth += (int) deltaSPW;
    myservo.write (servoPulseWidth);
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
  
  void measureElectricals () {
    
    vlow = analogRead (powerVoltageLowPin) / lowVoltageCalibration;          // read voltage on battery side of current sense resistor
    vhigh = analogRead (powerVoltageHighPin) / hiVoltageCalibration;        // read voltage on Phoenix side of current sense resistor 
    current = vhigh - vlow;                        // calculate current into Phoenix
    power = vlow * current;                      // calculate power into Phoenix
    
    vlowAverage += vlow;                         // update electrical averages
    vhighAverage += vhigh;
    currentAverage += current;
    powerAverage += power;
    electricalN++;
    
  }
  
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  void sendData () {
    
    motorSpeedAverage = motorSpeedSum / motorSpeedN;
    currentAverage /= electricalN;
    vlowAverage /= electricalN; 
    powerAverage /= electricalN;
       
    motorSpinning = motorSpeedAverage > validMotorSpeedLowerLimit;     // track whether motor is spinning     
//      }
//      else {
//        motorSpeedSum = 0;
//        currentAverage = 0;
//        vlowAverage /= electricalN; 
//        powerAverage = 0; 
//      }
      
 //     if ((currentAverage < 0) || (powerAverage < 0)) {
 //       motorSpeedSum = 0;
 //       currentAverage = 0;
 //       powerAverage = 0; 
 //     }
      
      Serial.print (vhigh);
      Serial.print (" V;  ");
      Serial.print (vlowAverage);
      Serial.print (" V;  ");
      Serial.print (currentAverage);  
      Serial.print (" A;  ");
      Serial.print (powerAverage);  
      Serial.print (" W;  ");
      Serial.print ("T: ");
      Serial.print (period);
      Serial.print (" us");
//      Serial.print (wantedOn);
      Serial.print (";  SPW: ");
      Serial.print (servoPulseWidth);
      Serial.print (";  ");
      Serial.print (motorSpeedAverage);  
      Serial.print (" rpms;  deltaSPW: ");
      Serial.print (deltaSPW);
      Serial.print (";  Integrated error: ");
      Serial.println (integratedError);
      
      if (!maxedRestarts) {
        lcd.setCursor (0,0);             // (Column, Row)
        lcd.print ("RPM: ");
        lcd.print ((long) motorSpeedAverage);
        lcd.print ("                 ");
      }
      
      motorSpeedSum = 0;
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

    if (micros () - start > 100UL)
      period = micros () - start;
    start = micros ();
  
  }
