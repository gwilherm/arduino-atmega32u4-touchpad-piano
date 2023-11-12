#pragma once

#include <Def/Def.hpp>
#include <MIDI_Outputs/Abstract/MIDIOutputElement.hpp>
#include <MIDI_Senders/DigitalNoteSender.hpp>
#include <AH/Timing/MillisMicrosTimer.hpp>

BEGIN_CS_NAMESPACE

#define NB_NOTES           12
#define UPDATE_RATE        50
#define HOLD_THRESHOLD_MS 500

namespace PianoMode {
    enum Mode: byte {Standard, Hold};
} // namespace PianoMode


class MIDITouchpadPiano : public MIDIOutputElement {
public:
    MIDITouchpadPiano(const pin_t scl_pin, const pin_t sdo_pin, MIDIAddress baseAddress, PianoMode::Mode mode = PianoMode::Standard)
        : scl_pin(scl_pin), sdo_pin(sdo_pin), baseAddress(baseAddress), updateTimer(UPDATE_RATE), mode(mode) {}

public:
    void begin() final override {
        /* Configure the clock and data pins */
        pinMode(scl_pin, OUTPUT);
        pinMode(sdo_pin, INPUT);
    }

    void update() final override {
        // Too high rate reading is no working
        if (!updateTimer) return;

        byte newKeysState[NB_NOTES];
        byte i;

        uint64_t prevTouch = lastTouchMs;

        // Read data
        for (i = 0; i < NB_NOTES; i++)
        {
            digitalWrite(scl_pin, LOW);
            newKeysState[NotePos[i]] = !digitalRead(sdo_pin);
            if (newKeysState[NotePos[i]]) lastTouchMs = millis();
            digitalWrite(scl_pin, HIGH);
        }
        
        for (i = 0; i < NB_NOTES; i++)
        {
            if ((mode == PianoMode::Hold) && ((newKeysState[i] & keysState[i]) == 1))
            {
                // Re-trigger the note if it was already ON
                if ((lastTouchMs - prevTouch) > HOLD_THRESHOLD_MS)
                {
                    sender.sendOff(baseAddress + i);
                    sender.sendOn(baseAddress + i);
                }
            }

            // Key changed state
            if (newKeysState[i] != keysState[i])
            {
                if (newKeysState[i] == 1)
                {
                    sender.sendOn(baseAddress + i);
                    keysState[i] = 1;
                }
                else
                {
                    if ((mode != PianoMode::Hold) || ((lastTouchMs - prevTouch) > HOLD_THRESHOLD_MS))
                    {
                        sender.sendOff(baseAddress + i);
                        keysState[i] = 0;
                    }
                }
            }
            // Serial.print(keysState[i]);
        }
        // Serial.println();

        updateTimer.beginNextPeriod();
    }

    void setMode(PianoMode::Mode mode) {
        // Leaving Hold mode requires to stop notes
        if (mode == PianoMode::Hold)
        {
            for (byte i = 0; i < NB_NOTES; i++)
            {
                if (keysState[i] == 1)
                {
                    sender.sendOff(baseAddress + i);
                    keysState[i] = 0;
                }
            }
        }
        this->mode = mode;
    }

    PianoMode::Mode getMode() { return mode; }

private:
    const pin_t scl_pin;
    const pin_t sdo_pin;
    const MIDIAddress baseAddress;
    const byte NotePos[NB_NOTES] = {1,3,6,8,10,0,2,4,5,7,9,11};
    byte keysState[NB_NOTES] = {0};
    uint64_t lastTouchMs = 0;
    AH::Timer<millis> updateTimer;
    PianoMode::Mode mode;

public:
    DigitalNoteSender sender;
};

END_CS_NAMESPACE