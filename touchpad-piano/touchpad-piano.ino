#include <USB-MIDI.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();


#define SCL_PIN 3
#define SDO_PIN 2

#define MIDI_CHANNEL 1
#define NB_NOTES     12

#define HOLD_THRESHOLD_MS 500
#define C4                 60
#define VELOCITY           64

byte NOTE_POS[NB_NOTES] = {1,3,6,8,10,0,2,4,5,7,9,11};
byte keys_state[NB_NOTES] = {0};

uint64_t lastTouchMs = 0;

void setup()
{
  /* Initialise the serial interface */
  Serial.begin(9600);
  /* Configure the clock and data pins */
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDO_PIN, INPUT);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);

  // while (!Serial);
}

void loop()
{
  byte new_keys_state[NB_NOTES];
  byte i;

  uint64_t prevTouch = lastTouchMs;
  // Read data
  for (i = 0; i < NB_NOTES; i++)
  {
    digitalWrite(SCL_PIN, LOW);
    new_keys_state[NOTE_POS[i]] = !digitalRead(SDO_PIN);
    if (new_keys_state[NOTE_POS[i]]) lastTouchMs = millis();
    digitalWrite(SCL_PIN, HIGH);
  }

  
  for (i = 0; i < NB_NOTES; i++)
  {

    // Re-trigger the note if it was already ON
    if ((new_keys_state[i] & keys_state[i]) == 1)
    {
        if ((lastTouchMs - prevTouch) > HOLD_THRESHOLD_MS)
        {
          MIDI.sendNoteOff(C4 + i, VELOCITY, MIDI_CHANNEL);
          MIDI.sendNoteOn(C4 + i, VELOCITY, MIDI_CHANNEL);
        }
    }

    if (new_keys_state[i] != keys_state[i])
    {
      if (new_keys_state[i] == 1)
      {
        MIDI.sendNoteOn(C4 + i, VELOCITY, MIDI_CHANNEL);
        keys_state[i] = 1;
      }
      else
      {
        if ((lastTouchMs - prevTouch) > HOLD_THRESHOLD_MS)
        {
          MIDI.sendNoteOff(C4 + i, VELOCITY, MIDI_CHANNEL);
          keys_state[i] = 0;
        }
      }
    }

    Serial.print(keys_state[i]);
  }

  Serial.println();

  delay(100);
}
