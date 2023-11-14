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
    enum Mode: uint8_t {Standard, Hold, Monodic};
} // namespace PianoMode


class MIDITouchpadPiano : public MIDIOutputElement {
public:
    MIDITouchpadPiano(const pin_t sclPin, const pin_t sdaPin, MIDIAddress baseAddress, PianoMode::Mode mode = PianoMode::Standard)
        : sclPin(sclPin), sdaPin(sdaPin), baseAddress(baseAddress), updateTimer(UPDATE_RATE), mode(mode), monodicNote(-1) {}

public:
    void begin() final override {
        /* Configure the clock and data pins */
        pinMode(sclPin, OUTPUT);
        pinMode(sdaPin, INPUT);
    }

    void update() final override {
        // Too high rate reading is no working
        if (!updateTimer) return;

        uint8_t newKeysState[NB_NOTES];
        int8_t newMonodicNote = -1;

        readData(newKeysState, &newMonodicNote);
        
        switch (mode) {
            case PianoMode::Standard:
                handleStandard(newKeysState);
                break;
            case PianoMode::Hold:
                handleHold(newKeysState);
                break;
            case PianoMode::Monodic:
                handleMonodic(newMonodicNote);
                break;
        }

        updateTimer.beginNextPeriod();
    }

    void setMode(PianoMode::Mode mode) {
        // Leaving Hold mode requires to stop notes
        if (this->mode == PianoMode::Hold)
        {
            for (uint8_t i = 0; i < NB_NOTES; i++)
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

    void readData(uint8_t newKeysState[NB_NOTES], int8_t* newMonodicNote)
    {
        for (uint8_t i = 0; i < NB_NOTES; i++)
        {
            digitalWrite(sclPin, LOW);
            newKeysState[NoteMap[i]] = !digitalRead(sdaPin);
            if (newKeysState[NoteMap[i]])
            {
                holdLastTouchMs = millis();
                if (NoteMap[i] > *newMonodicNote)
                    *newMonodicNote = NoteMap[i];
            }
            digitalWrite(sclPin, HIGH);
        }
    }

    void handleStandard(uint8_t newKeysState[NB_NOTES])
    {
        for (uint8_t i = 0; i < NB_NOTES; i++)
        {
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
                    sender.sendOff(baseAddress + i);
                    keysState[i] = 0;
                }
            }
#ifdef DEBUG_VERBOSE
            Serial.print(keysState[i]);
#endif
        }
#ifdef DEBUG_VERBOSE
        Serial.println();
#endif
    }

    void handleHold(uint8_t newKeysState[NB_NOTES])
    {
        uint64_t holdPrevTouch = holdLastTouchMs;

        for (uint8_t i = 0; i < NB_NOTES; i++)
        {
            if ((newKeysState[i] & keysState[i]) == 1)
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
                    if ((holdLastTouchMs - holdPrevTouch) > HOLD_THRESHOLD_MS)
                    {
                        sender.sendOff(baseAddress + i);
                        keysState[i] = 0;
                    }
                }
            }
#ifdef DEBUG_VERBOSE
            Serial.print(keysState[i]);
#endif
        }
#ifdef DEBUG_VERBOSE
        Serial.println();
#endif
    }

    void handleMonodic(int8_t newMonodicNote)
    {
        if (newMonodicNote != monodicNote)
        {
            sender.sendOff(baseAddress + monodicNote);
            if (newMonodicNote != -1)
                sender.sendOn(baseAddress + newMonodicNote);
            monodicNote = newMonodicNote;
        }
#ifdef DEBUG_VERBOSE
        Serial.println(monodicNote);
#endif
    }

private:
    const pin_t sclPin;
    const pin_t sdaPin;
    const MIDIAddress baseAddress;
    const uint8_t NoteMap[NB_NOTES] = {1,3,6,8,10,0,2,4,5,7,9,11};
    uint8_t keysState[NB_NOTES] = {0};
    uint64_t holdLastTouchMs = 0;
    AH::Timer<millis> updateTimer;
    PianoMode::Mode mode;
    int8_t monodicNote;

public:
    DigitalNoteSender sender;
};

END_CS_NAMESPACE