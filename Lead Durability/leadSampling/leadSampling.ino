/*
  AnalogReadSerial
  Reads an analog input on pin 0, prints the result to the serial monitor.
  Attach the center pin of a potentiometer to pin A0, and the outside pins to +5V and ground.
 
 This example code is in the public domain.
 */

// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
}

// the loop routine runs over and over again forever:
void loop() {
  // read the input on analog pin 0:
  Serial.print(analogRead(A0) > 1000);
  Serial.print(analogRead(A1) > 1000);
  Serial.print(analogRead(A2) > 1000);
  Serial.print(analogRead(A3) > 1000);
  Serial.print(analogRead(A4) > 1000);
  Serial.print(analogRead(A5) > 1000);
  Serial.print(analogRead(A6) > 1000);
  Serial.print(analogRead(A7) > 1000);
  Serial.print(analogRead(A8) > 1000);
  Serial.print(analogRead(A9) > 10000);
  Serial.print(analogRead(A10) > 1000);
  Serial.print(analogRead(A11) > 1000);
 Serial.print('\n');
//  Serial.println(analogRead(A0));
//  Serial.println(analogRead(A1));
//  Serial.println(analogRead(A2));
//  Serial.println(analogRead(A3));
//  Serial.println(analogRead(A4));
//  Serial.println(analogRead(A5));
//  Serial.println(analogRead(A6));
//  Serial.println(analogRead(A7));
//  Serial.println(analogRead(A8));
//  Serial.println(analogRead(A9));
//  Serial.println(analogRead(A10));
//  Serial.println(analogRead(A11));
  // print out the value you read:
  delay(1000);        // delay in between reads for stability
}
