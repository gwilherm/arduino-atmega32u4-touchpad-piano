#include <Control_Surface.h>

#include "MIDITouchpadPiano.hpp"

// Instantiate a MIDI over USB interface.
USBMIDI_Interface midi;

#define SCL_PIN 3
#define SDO_PIN 2

#define BTN_PIN 5

MIDITouchpadPiano piano(SCL_PIN, SDO_PIN, MIDI_Notes::C(4));

int btnState = LOW;
unsigned long lastDebounceTime = 0;

void setup()
{
  /* Initialise the serial interface */
  // Serial.begin(9600);
  pinMode(BTN_PIN, INPUT_PULLUP);

  Control_Surface.begin(); // Initialize Control Surface

  // while (!Serial);
}

void loop()
{
  int newBtnState = digitalRead(BTN_PIN);

  if ((millis() - lastDebounceTime) > 50)
  {
    if ((btnState == LOW) && (newBtnState == HIGH))
    {
      switch (piano.getMode())
      {
        case PianoMode::Standard:
          piano.setMode(PianoMode::Hold);
          break;
        case PianoMode::Hold:
          piano.setMode(PianoMode::Standard);
          break;
      }
    }
    btnState = newBtnState;
    lastDebounceTime = millis();
  }

  Control_Surface.loop(); // Update the Control Surface
}
