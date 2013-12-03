volatile int counter = 0;

void setup(){
  
//  // Set up computer connection
//  Serial.begin(9600);

// PORTD = PORTD | B01000000;
    // initialize Timer1
  cli();             // disable global interrupts
  TCCR1A = 0;        // set entire TCCR1A register to 0
  TCCR1B = 0;
   
  // set compare match register to desired timer count:
  OCR1A = 10;
  // enable timer compare interrupt:
  TIMSK1 |= (1 << OCIE1A);
  // Set CS10 bit so timer runs at clock speed:
  TCCR1B |= (1 << CS10);
  // Set to CTC mode
  TCCR1B |= (1 << WGM12);
  
  // enable global interrupts:
  sei();
}

ISR(TIMER1_COMPA_vect){
  if (counter == 0)
    PORTD = PORTD | B01000000;
  if (counter == 1)
    PORTD = PORTD & B10111111;
  counter++;
  if (counter==100)
    counter = 0;
}

void loop(){}
