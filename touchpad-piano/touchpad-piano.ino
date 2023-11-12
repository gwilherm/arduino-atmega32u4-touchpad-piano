#include <Control_Surface.h>
#include "MIDITouchpadPiano.hpp"

// Instantiate a MIDI over USB interface.
USBMIDI_Interface midi;

#define SCL_PIN 3
#define SDO_PIN 2

MIDITouchpadPiano piano(SCL_PIN, SDO_PIN, MIDI_Notes::C(4), PianoMode::Hold);

void setup()
{
  /* Initialise the serial interface */
  // Serial.begin(9600);
  
  Control_Surface.begin(); // Initialize Control Surface

  // while (!Serial);
}

void loop()
{
  Control_Surface.loop(); // Update the Control Surface
}
