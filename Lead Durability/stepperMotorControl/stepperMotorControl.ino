// Arduino board pins
const int stepPin = 10;
const int dirPin = 4;
const int stepsPerRevolution = 400;  // change this to fit the number of steps per revolution

// Variables to control motor motion
float rpm;
float angle;
unsigned int stepsPerHalfCycle;
unsigned long timePerStepInMicros;
unsigned long stepCounter = 0;
unsigned long cycleCounter = 0;
unsigned long beginTime;
boolean leadIntact;
int lead;

// Variables to update RPM and ANGLE values
char firstC = 'x';
char secondC = 'x';
char thirdC = 'x';

void setup(){

  // Set up computer connection
  Serial.begin(9600);

  // Set up Arduino digital pins
  pinMode(stepPin, OUTPUT);
  pinMode(dirPin, OUTPUT);

  // Set initial RPM here
  rpm = 120;
  timePerStepInMicros = 1000000*(60/(stepsPerRevolution*rpm)); // for 200 steps/rev 60rpm = 5 ms
  Serial.print("timePerStepInMicros = ");
  Serial.println(timePerStepInMicros);

  // Set initial rotation angle (in degrees) here. So far actual rotation seems
  // less by upto 20-30 degrees
  angle = 180;  // Motor moves this much at a time
  stepsPerHalfCycle = stepsPerRevolution*(angle/360);
  Serial.print("stepsPerHalfCycle = ");
  Serial.println(stepsPerHalfCycle);
}

void loop(){ //run once per step
  stepCounter=stepCounter+1;
  // Rotation direction change
  if (stepCounter < stepsPerHalfCycle){
    digitalWrite(dirPin, HIGH);
  }
  else{
    digitalWrite(dirPin, LOW);
  }
  // Pulse signal to step motor
  beginTime = micros();

  digitalWrite(stepPin, HIGH);
  delayMicroseconds(100); //is this long enough?
  digitalWrite(stepPin, LOW);

  // Check for new RPM/ANGLE value
  //Serial.println("free time!"); //can check for serial less often
  //    if (analogRead(cycleCounter % 12) > 1000){ //each takes 100 µs
  //      //lead intact
  //    } 
  //    else{
  //      //lead fail
  //      Serial.print(cycleCounter % 12);
  //      Serial.println("fail");
  //    }

  updateVars();
  Serial.println(timePerStepInMicros-(micros()-beginTime));
  delayMicroseconds(timePerStepInMicros-(micros()-beginTime));
  if(stepCounter > stepsPerHalfCycle*2){
    stepCounter=0;
  }
}

void updateVars(){
  // Function takes user input and updates rotation angle and/or motor rpm

  // To use, open serial monitor and type (beside send button) 
  // in something to the effect of rpm = 60, angle = 10
  while (Serial.available()>0){
    firstC = secondC;
    secondC = thirdC;
    thirdC = (char)Serial.read();
    if (firstC =='r' || firstC=='R'){
      if (secondC=='p'||secondC=='P'){
        if (thirdC=='m'||thirdC=='M'){
          rpm = Serial.parseInt();
          timePerStepInMicros = 1000000*(60/(stepsPerRevolution*rpm));
          Serial.print("NEW RPM = ");
          Serial.println(rpm);
        }
      }
    }
    if (firstC =='a' || firstC=='A'){
      if (secondC=='n'||secondC=='N'){
        if (thirdC=='g'||thirdC=='G'){
          angle = Serial.parseInt();  // Motor rotates between -angle and +angle
          stepsPerHalfCycle = stepsPerRevolution*(angle/360);
          Serial.print("NEW ROTATION ANGLE = ");
          Serial.println(angle);
        }
      }
    }
  }
}
