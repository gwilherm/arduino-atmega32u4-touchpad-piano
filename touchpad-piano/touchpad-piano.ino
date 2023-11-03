#include <USB-MIDI.h>

USBMIDI_CREATE_DEFAULT_INSTANCE();


#define SCL_PIN 3
#define SDO_PIN 2

#define MIDI_CHANNEL 1
#define NB_NOTES     12

byte NOTE_POS[NB_NOTES] = {1,3,6,8,10,0,2,4,5,7,9,11};
byte keys_state[NB_NOTES] = {0};

void setup()
{
  /* Initialise the serial interface */
  Serial.begin(9600);
  /* Configure the clock and data pins */
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDO_PIN, INPUT);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop()
{
  byte new_keys_state[NB_NOTES];
  byte i;

  // Read data
  for (i = 0; i < NB_NOTES; i++)
  {
    digitalWrite(SCL_PIN, LOW);
    new_keys_state[NOTE_POS[i]] = !digitalRead(SDO_PIN);
    digitalWrite(SCL_PIN, HIGH);
  }

  for (i = 0; i < NB_NOTES; i++)
  {
    // Display input data
    Serial.print(new_keys_state[i]);
    if (new_keys_state[i] != keys_state[i])
    {
      // Play / Stop the MIDI note
      if (new_keys_state[i] == 1)
        MIDI.sendNoteOn(60 + i, 64, MIDI_CHANNEL);
      else
        MIDI.sendNoteOff(60 + i, 64, MIDI_CHANNEL);
      
      // Save the new state
      keys_state[i] = new_keys_state[i];
    }
  }

  Serial.println();

  delay(100);
}
