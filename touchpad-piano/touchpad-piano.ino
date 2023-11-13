#include <Control_Surface.h>
#include <Adafruit_NeoPixel.h>
#include "MIDITouchpadPiano.hpp"

// Instantiate a MIDI over USB interface.
USBMIDI_Interface midi;

#define SDO_PIN 2
#define SCL_PIN 3
#define RGB_PIN 4
#define BTN_PIN 5
#define LED_COUNT 4

MIDITouchpadPiano piano(SCL_PIN, SDO_PIN, MIDI_Notes::C(4));

int btnState = LOW;
unsigned long lastDebounceTime = 0;

Adafruit_NeoPixel strip(LED_COUNT, RGB_PIN, NEO_GRB + NEO_KHZ800);

void colorFade(uint8_t r, uint8_t g, uint8_t b) {
  uint8_t curr_r, curr_g, curr_b;
  uint32_t curr_col = strip.getPixelColor(0); // get the current colour
  curr_b = curr_col & 0xFF; curr_g = (curr_col >> 8) & 0xFF; curr_r = (curr_col >> 16) & 0xFF; // separate into RGB components

  while ((curr_r != r) || (curr_g != g) || (curr_b != b)){  // while the curr color is not yet the target color
    if (curr_r < r) curr_r++; else if (curr_r > r) curr_r--;  // increment or decrement the old color values
    if (curr_g < g) curr_g++; else if (curr_g > g) curr_g--;
    if (curr_b < b) curr_b++; else if (curr_b > b) curr_b--;

    for(uint16_t i = 0; i < strip.numPixels(); i++)
      strip.setPixelColor(i, curr_r, curr_g, curr_b);  // set the color

    strip.show();
    delay(1);  // add a delay if its too fast
  }
}

void setup()
{
  /* Initialise the serial interface */
  Serial.begin(9600);
  pinMode(BTN_PIN, INPUT_PULLUP);

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(10);

  Control_Surface.begin(); // Initialize Control Surface

  while (!Serial);
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
          colorFade( 25, 210,  25); // fade into green
          break;
        case PianoMode::Hold:
          piano.setMode(PianoMode::Monodic);
          colorFade( 25,  25, 210); // fade into blue
          break;
        case PianoMode::Monodic:
          piano.setMode(PianoMode::Standard);
          colorFade(210,  25,  25); // fade into red
          break;
      }
    }
    btnState = newBtnState;
    lastDebounceTime = millis();
  }

  Control_Surface.loop(); // Update the Control Surface
}
