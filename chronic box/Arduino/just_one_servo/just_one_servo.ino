  #include <Servo.h>

  Servo myservo;

  void setup () {
    myservo.attach (8);  // attaches the servo on pin 9 to the servo object 
    myservo.write (0);

    for (int i=0; i<180; i++) {
      myservo.write (73);  //73 seems to be the lowest that will work
      delay (25);
    }
    
    myservo.write (0);
  }
  
  void loop () {
  }
