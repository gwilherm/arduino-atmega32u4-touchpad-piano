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
    enum Mode: byte {Standard, Hold, Monodic};
} // namespace PianoMode


class MIDITouchpadPiano : public MIDIOutputElement {
public:
    MIDITouchpadPiano(const pin_t sclPin, const pin_t sdoPin, MIDIAddress baseAddress, PianoMode::Mode mode = PianoMode::Standard)
        : sclPin(sclPin), sdoPin(sdoPin), baseAddress(baseAddress), updateTimer(UPDATE_RATE), mode(mode), monodicNote(-1) {}

public:
    void begin() final override {
        /* Configure the clock and data pins */
        pinMode(sclPin, OUTPUT);
        pinMode(sdoPin, INPUT);
    }

    void update() final override {
        // Too high rate reading is no working
        if (!updateTimer) return;

        byte newKeysState[NB_NOTES];
        int newMonodicNote = -1;
        byte i;

        uint64_t holdPrevTouch = holdLastTouchMs;

        // Read data
        for (i = 0; i < NB_NOTES; i++)
        {
            digitalWrite(sclPin, LOW);
            newKeysState[NoteMap[i]] = !digitalRead(sdoPin);
            if (newKeysState[NoteMap[i]])
            {
                holdLastTouchMs = millis();
                if (NoteMap[i] > newMonodicNote)
                    newMonodicNote = NoteMap[i];
            }
            digitalWrite(sclPin, HIGH);
        }
        
        if (mode == PianoMode::Monodic)
        {
            if (newMonodicNote != monodicNote)
            {
                sender.sendOff(baseAddress + monodicNote);
                if (newMonodicNote != -1)
                    sender.sendOn(baseAddress + newMonodicNote);
                monodicNote = newMonodicNote;
            }
            Serial.println(monodicNote);
        }
        else
        {
            for (i = 0; i < NB_NOTES; i++)
            {
                if ((mode == PianoMode::Hold) && ((newKeysState[i] & keysState[i]) == 1))
                {
                    // Re-trigger the note if it was already ON
                    if ((holdLastTouchMs - holdPrevTouch) > HOLD_THRESHOLD_MS)
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
                        if ((mode != PianoMode::Hold) || ((holdLastTouchMs - holdPrevTouch) > HOLD_THRESHOLD_MS))
                        {
                            sender.sendOff(baseAddress + i);
                            keysState[i] = 0;
                        }
                    }
                }
                Serial.print(keysState[i]);
            }
            Serial.println();
        }
        updateTimer.beginNextPeriod();
    }

    void setMode(PianoMode::Mode mode) {
        // Leaving Hold mode requires to stop notes
        if (this->mode == PianoMode::Hold)
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
    const pin_t sclPin;
    const pin_t sdoPin;
    const MIDIAddress baseAddress;
    const byte NoteMap[NB_NOTES] = {1,3,6,8,10,0,2,4,5,7,9,11};
    byte keysState[NB_NOTES] = {0};
    uint64_t holdLastTouchMs = 0;
    AH::Timer<millis> updateTimer;
    PianoMode::Mode mode;
    int monodicNote;

public:
    DigitalNoteSender sender;
};

END_CS_NAMESPACE