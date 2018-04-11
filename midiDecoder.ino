/** Author Willem Hillier
 *  https://willemhillier.wordpress.com
 *  willemhillier@gmail.com
 *  Licensed under GPL, see
 */ 

//Key map. Starts at MIDI note MidiOffset.
const byte PORTFMidi[] = {B11000111, B00000111, B00000000, B00111110, B00000000, B00000000, B00000000, B00000000,
                          B00000000, B00000000, B00000110, B00000000, B00000000, B00000000, B00000000, B00000000,
                          B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000110, B00000000,
                          B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000
                         };

const byte PORTKMidi[] = {B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
                          B00000000, B00000000, B00000000, B00000000, B00100000, B00000000, B00000000, B00000000,
                          B10000000, B10000000, B10000000, B10000000, B10000000, B10000000, B10000000, B10000000,
                          B10100000, B10000000, B10000000, B10000000, B10000010, B10001010, B10011010, B10011110
                         };

const byte PORTAMidi[] = {B10011010, B10011010, B10011010, B10011010, B00011010, B01011010, B00011010, B00011010,
                          B00011010, B00011010, B00011010, B00001010, B00001010, B00000010, B00001000, B00000000,
                          B00011010, B01011010, B00011010, B00011010, B00011010, B00011010, B00011010, B00001010,
                          B00001010, B00000010, B00001000, B00000000, B00000000, B00000000, B00000000, B00000000
                         };

const byte PORTCMidi[] = {B11111111, B11111111, B11111111, B11111111, B11111111, B11111111, B00111111, B00000111,
                          B00111000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000,
                          B11111111, B11111111, B00111111, B00000111, B00111000, B00000000, B00000000, B00000000,
                          B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000, B00000000
                         };
//variables setup

byte lastPORTK;
byte lastPORTF;
byte lastPORTA;
byte lastPORTC;
byte checkValue;

const int MidiOffset = 49;

const int PORTKTrigger = 3;
const int PORTFTrigger = 2;
const int PORTATrigger = 5;
const int PORTCTrigger = 4;

byte incomingByte;
byte note;
byte velocity;

int statusLed = 13;   // select the pin for the LED

int action = 2; //0 =note off ; 1=note on ; 2= nada

//setup: declaring iputs and outputs and begin serial
void setup() {
  pinMode(statusLed, OUTPUT);  // declare the LED's pin as output

  DDRF = B11111111; //Setup pins as outputs
  DDRK = B11111111;
  DDRA = B11111111;
  DDRC = B11111111;

  for (int n = 2; n <= 5; n++) { //trigger outputs
    pinMode(n, OUTPUT);
    digitalWrite(n, HIGH);
  }
  /*delay(500);
    for(int n=2;n<=5;n++){ //test trigger board
    digitalWrite(n,LOW);
    digitalWrite(n, HIGH);
    delay(1000);
    }
  */

  Serial.begin(38400); //start serial with midi baudrate 38400 to talk to hairless-midiserial
}

//loop: wait for serial data, and interpret the message
void loop () {
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // wait for as status-byte, channel 2, note on or off (channel 1 if zero-indexed)
    if (incomingByte == 145) { // note on message starting starting
      action = 1;
    } else if (incomingByte == 128) { // note off message starting
      action = 0;
    } else if (incomingByte == 208) { // aftertouch message starting
      //not implemented yet
    } else if (incomingByte == 160) { // polypressure message starting
      //not implemented yet
    } else if ( (action == 0) && (note == 0) ) { // if we received a "note off", we wait for which note (databyte)
      note = incomingByte;
      playNote(note, 0);
      note = 0;
      velocity = 0;
      action = 2;
    } else if ( (action == 1) && (note == 0) ) { // if we received a "note on", we wait for the note (databyte)
      note = incomingByte;
    } else if ( (action == 1) && (note != 0) ) { // ...and then the velocity
      velocity = incomingByte;
      playNote(note, velocity);
      note = 0;
      velocity = 0;
      action = 0;
    } else {
      //nada
    }
  }
}

void playNote(byte note, byte velocity) {
  if ((velocity > 10) && ((note - MidiOffset) < 32 && (note - MidiOffset) >= 0)) {
    digitalWrite(statusLed, HIGH);
    PORTK = PORTKMidi[note - MidiOffset]; //Solenoid lookup and write for PORTK
    PORTF = PORTFMidi[note - MidiOffset]; //Solenoid lookup and write for PORTF
    PORTA = PORTAMidi[note - MidiOffset]; //Solenoid lookup and write for PORTA
    PORTC = PORTCMidi[note - MidiOffset]; //Solenoid lookup and write for PORTC


    checkValue = ((~lastPORTK)&PORTKMidi[note + MidiOffset]); //Used to be ((PORTKMidi[note+MidiOffset]^lastPORTK)^lastPORTK)
    if (checkValue = !B0000000) { //has it changed from last loop? If so, we need to trigger PWM generator.
      digitalWrite(PORTKTrigger, LOW); //Trigger PWM timer
      digitalWrite(PORTKTrigger, HIGH);
    }

    checkValue = ((~lastPORTF)&PORTFMidi[note + MidiOffset]);
    if (checkValue = !B0000000) { //has it changed from last loop? If so, we need to trigger PWM generator.
      digitalWrite(PORTFTrigger, LOW); //Trigger PWM timer
      digitalWrite(PORTFTrigger, HIGH);
    }

    checkValue = ((~lastPORTA)&PORTAMidi[note + MidiOffset]);
    if (checkValue = !B0000000) { //has it changed from last loop? If so, we need to trigger PWM generator.
      digitalWrite(PORTATrigger, LOW); //Trigger PWM timer
      digitalWrite(PORTATrigger, HIGH);
    }

    checkValue = ((~lastPORTC)&PORTCMidi[note + MidiOffset]);
    if (checkValue = !B0000000) { //has it changed from last loop? If so, we need to trigger PWM generator.
      digitalWrite(PORTCTrigger, LOW); //Trigger PWM timer
      digitalWrite(PORTCTrigger, HIGH);
    }

    lastPORTK = PORTKMidi[note + MidiOffset];
    lastPORTF = PORTFMidi[note + MidiOffset];
    lastPORTA = PORTAMidi[note + MidiOffset];
    lastPORTC = PORTCMidi[note + MidiOffset];

  } else {
    //PORTK = 0; //Uncomment these if you want the sax to release keys on MIDI note-off command
    //PORTF = 0;
    //PORTA = 0;
    //PORTC = 0;
    lastPORTK = 0;
    lastPORTF = 0;
    lastPORTA = 0;
    lastPORTC = 0;
    digitalWrite(statusLed, LOW);
  }
}
