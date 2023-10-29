#define SCL_PIN 3
#define SDO_PIN 2

byte NOTE_POS[12] = {1,3,6,8,10,0,2,4,5,7,9,11};

void setup()
{
  /* Initialise the serial interface */
  Serial.begin(9600);
  /* Configure the clock and data pins */
  pinMode(SCL_PIN, OUTPUT);
  pinMode(SDO_PIN, INPUT);
  while (!Serial); // Leonardo: wait for Serial Monitor

}

void loop()
{
  byte key_table[12];
  byte i;

  // Read data
  for (i = 0; i < 12; i++)
  {
    digitalWrite(SCL_PIN, LOW);
    key_table[NOTE_POS[i]] = !digitalRead(SDO_PIN);
    digitalWrite(SCL_PIN, HIGH);
  }

  // Display data
  for (i = 0; i < 12; i++)
    Serial.print(key_table[i]);
  Serial.println();

  delay(100);
}
