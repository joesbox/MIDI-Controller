// MIDI Controller for Teensy LC
// Various sources used including Arduino examples, forums, notesandvolts.com etc.
// 5 direct digital buttons
// 5 rotary encoders
// 5 encoder buttons via a 74HC4051 multiplexer chip
// 5 potentiometers (analog inputs)

#include <Bounce.h>
#include <Encoder.h>

// The MIDI channel number to send messages
const int channel = 1;

// The MIDI continuous controller for each analog input
const int controllerA0 = 20;
const int controllerA1 = 21;
const int controllerA2 = 22;
const int controllerA3 = 23;
const int controllerA4 = 24;

// Store previously sent analog values, to detect changes
int previousA0 = -1;
int previousA1 = -1;
int previousA2 = -1;
int previousA3 = -1;
int previousA4 = -1;

// Store the elapsed milliseconds for analog and multiplexed input smoothing/de-bouncing
elapsedMillis msec = 0;
long msecMux[5] = {0, 0, 0, 0, 0};

// Create Bounce objects for each button
Bounce button0 = Bounce(14, 5);
Bounce button1 = Bounce(15, 5);  // 5 = 5 ms debounce time
Bounce button2 = Bounce(21, 5);  // which is appropriate for good
Bounce button3 = Bounce(22, 5);  // quality mechanical pushbuttons
Bounce button4 = Bounce(23, 5);

// Create encoder objects with pin numbers
Encoder enc1(3, 2);
Encoder enc2(5, 4);
Encoder enc3(7, 6);
Encoder enc4(9, 8);
Encoder enc5(11, 10);

// Setup encoder inputs. Current readings and previous readings
int current_value1;
int previous_value1;

int current_value2;
int previous_value2;

int current_value3;
int previous_value3;

int current_value4;
int previous_value4;

int current_value5;
int previous_value5;

// Setup mux chip pins
const int selectPins[3] = {0, 1, 12}; // S0~0, S1~1, S2~12
const int zOutput = 5;
const int zInput = 19; // Connect common (Z) to 19 (digital input)
const int muxNotes[5] = {65, 66, 67, 68, 69}; // Notes assinged to encoder buttons

// Mux button states for de-bouncing
int buttonState[5] = { -1, -1, -1, -1, -1};
int buttonStatePrev[5] = { -1, -1, -1, -1, -1};

void setup() {
  // Configure the pins for input mode with pullup resistors on the digital inputs  
  pinMode(14, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(21, INPUT_PULLUP);
  pinMode(22, INPUT_PULLUP);
  pinMode(23, INPUT_PULLUP);
  pinMode(A2, INPUT);
  pinMode(A3, INPUT);
  pinMode(A4, INPUT);
  pinMode(A10, INPUT);
  pinMode(A11, INPUT);

  // LED Pin to output mode
  pinMode(13, OUTPUT);

  // Begin serial for debugging
  Serial.begin(38400);

  // Turn the LED on
  digitalWrite(13, HIGH);

  // Set up the select pins as outputs for the mux chip and raise them:
  for (int i = 0; i < 3; i++)
  {
    pinMode(selectPins[i], OUTPUT);
    digitalWrite(selectPins[i], HIGH);
  }

  // Set up mux Z as an input
  pinMode(zInput, INPUT_PULLUP);

}

void loop() {
  // Update all the buttons
  button0.update();
  button1.update();
  button2.update();
  button3.update();
  button4.update();

  // Check each button for "falling" edge.
  // falling = high (not pressed - voltage from pullup resistor)
  //           to low (pressed - button connects pin to ground)
  if (button0.fallingEdge()) {
    usbMIDI.sendNoteOn(60, 99, channel);  // 60 = C4
  }
  if (button1.fallingEdge()) {
    usbMIDI.sendNoteOn(61, 99, channel);  // 61 = C#4
  }
  if (button2.fallingEdge()) {
    usbMIDI.sendNoteOn(62, 99, channel);  // 62 = D4
  }
  if (button3.fallingEdge()) {
    usbMIDI.sendNoteOn(63, 99, channel);  // 63 = D#4
  }
  if (button4.fallingEdge()) {
    usbMIDI.sendNoteOn(64, 99, channel);  // 64 = E4
  }

  // Check each button for "rising" edge
  // Send a MIDI Note Off message when each button releases
  // rising = low (pressed - button connects pin to ground)
  //          to high (not pressed - voltage from pullup resistor)
  if (button0.risingEdge()) {
    usbMIDI.sendNoteOff(60, 0, channel);  // 60 = C4
  }
  if (button1.risingEdge()) {
    usbMIDI.sendNoteOff(61, 0, channel);  // 61 = C#4
  }
  if (button2.risingEdge()) {
    usbMIDI.sendNoteOff(62, 0, channel);  // 62 = D4
  }
  if (button3.risingEdge()) {
    usbMIDI.sendNoteOff(63, 0, channel);  // 63 = D#4
  }
  if (button4.risingEdge()) {
    usbMIDI.sendNoteOff(64, 0, channel);  // 64 = E4
  }

  // Only check the analog inputs 50 times per second,
  // to prevent a flood of MIDI messages
  if (msec >= 20) {
    msec = 0;
    int n0 = analogRead(A2) / 8;
    int n1 = analogRead(A3) / 8;
    int n2 = analogRead(A4) / 8;
    int n3 = analogRead(A10) / 8;
    int n4 = analogRead(A11) / 8;

    // only transmit MIDI messages if analog input changed
    // and change greater than 1 has occured
    // (Helps stop unwanted MIDI messages when the pot is teetering between two values)
    if (abs(n0 - previousA0) > 3) { // Found that the value from this pot is a bit jittery still
      usbMIDI.sendControlChange(controllerA0, n0, channel);
      previousA0 = n0;
    }
    if (abs(n1 - previousA1) > 1) {
      usbMIDI.sendControlChange(controllerA1, n1, channel);
      previousA1 = n1;
    }
    if (abs(n2 - previousA2) > 1) {
      usbMIDI.sendControlChange(controllerA2, n2, channel);
      previousA2 = n2;
    }
    if (abs(n3 - previousA3) > 1) {
      usbMIDI.sendControlChange(controllerA3, n3, channel);
      previousA3 = n3;
    }
    if (abs(n4 - previousA4) > 1) {
      usbMIDI.sendControlChange(controllerA4, n4, channel);
      previousA4 = n4;
    }
  }

  // Read current values from the encoders
  current_value1 = enc1.read();
  current_value2 = enc2.read();
  current_value3 = enc3.read();
  current_value4 = enc4.read();
  current_value5 = enc5.read();

  // Encoder 1
  if (current_value1 != previous_value1) { // compare to previous reading

    previous_value1 = current_value1; // store new value as previous reading

    if (current_value1 < 0) { // constrain for below 0
      current_value1 = 0; // constrain variable
      enc1.write(0); // constrain the encoder object
    }

    else if (current_value1 > 127) { // constrain for above 127
      current_value1 = 127; // constrain variable
      enc1.write(127); // constrain the encoder object
    }

    usbMIDI.sendControlChange(1, current_value1, channel); // send MIDI value

  }

  // Encoder 2
  if (current_value2 != previous_value2) { // compare to previous reading

    previous_value2 = current_value2; // store new value as previous reading

    if (current_value2 < 0) { // constrain for below 0
      current_value2 = 0; // constrain variable
      enc2.write(0); // constrain the encoder object
    }

    else if (current_value2 > 127) { // constrain for above 127
      current_value2 = 127; // constrain variable
      enc2.write(127); // constrain the encoder object
    }

    usbMIDI.sendControlChange(2, current_value2, channel); // send MIDI value

  }

  // Encoder 3
  if (current_value3 != previous_value3) { // compare to previous reading

    previous_value3 = current_value3; // store new value as previous reading

    if (current_value3 < 0) { // constrain for below 0
      current_value3 = 0; // constrain variable
      enc3.write(0); // constrain the encoder object
    }

    else if (current_value3 > 127) { // constrain for above 127
      current_value3 = 127; // constrain variable
      enc3.write(127); // constrain the encoder object
    }

    usbMIDI.sendControlChange(3, current_value3, channel); // send MIDI value

  }

  // Encoder 4
  if (current_value4 != previous_value4) { // compare to previous reading

    previous_value4 = current_value4; // store new value as previous reading

    if (current_value4 < 0) { // constrain for below 0
      current_value4 = 0; // constrain variable
      enc4.write(0); // constrain the encoder object
    }

    else if (current_value4 > 127) { // constrain for above 127
      current_value4 = 127; // constrain variable
      enc4.write(127); // constrain the encoder object
    }

    usbMIDI.sendControlChange(4, current_value4, channel); // send MIDI value

  }

  // Encoder 5
  if (current_value5 != previous_value5) { // compare to previous reading

    previous_value5 = current_value5; // store new value as previous reading

    if (current_value5 < 0) { // constrain for below 0
      current_value5 = 0; // constrain variable
      enc5.write(0); // constrain the encoder object
    }

    else if (current_value5 > 127) { // constrain for above 127
      current_value5 = 127; // constrain variable
      enc5.write(127); // constrain the encoder object
    }

    usbMIDI.sendControlChange(5, current_value5, channel); // send MIDI value

  }

  // Loop through only the first five mux pins (last three are not connected)
  for (byte pin = 0; pin <= 4; pin++)
  {    
    // Effectively a 5ms debounce
    if ((millis() - msecMux[pin]) >= 5) {
      selectMuxPin(pin); // Select one at a time
      buttonState[pin] = digitalRead(zInput); // and read Z
    
      if (buttonState[pin] != buttonStatePrev[pin]) {
        // If a button is pressed
        if (buttonState[pin] == 0) {
          usbMIDI.sendNoteOn(muxNotes[pin], 99, channel); // send MIDI value using the index in to the array of note values
        }
        else {
          usbMIDI.sendNoteOff(muxNotes[pin], 99, channel); // send MIDI value using the index in to the array of note values
        }
        msecMux[pin] = millis();
        buttonStatePrev[pin] = buttonState[pin];
      }

    }
  }

  // MIDI Controllers should discard incoming MIDI messages.
  // http://forum.pjrc.com/threads/24179-Teensy-3-Ableton-Analog-CC-causes-midi-crash
  while (usbMIDI.read()) {
    // ignore incoming messages
  }
  delay(10);
}

// The selectMuxPin function sets the S0, S1, and S2 pins
// accordingly, given a pin from 0-7.
void selectMuxPin(byte pin)
{
  for (int i = 0; i < 3; i++)
  {
    if (pin & (1 << i)){
      digitalWrite(selectPins[i], HIGH);
      delayMicroseconds(10);
    }
    else {
      digitalWrite(selectPins[i], LOW);
      delayMicroseconds(10);
    }
  }
}
