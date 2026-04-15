#pragma once
#include <Arduino.h>

class IBusPico
{
public:
    static constexpr uint8_t CHANNEL_COUNT = 14;
    static constexpr uint8_t FRAME_SIZE    = 32;

    void begin(HardwareSerial& serial)
    {
        _serial = &serial;
        _frameIndex = 0;
        _frameValid = false;
        for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {
            _channels[i] = 1500; // midpoint default
        }
    }

    // Call frequently (in loop)
    void loop()
    {
        while (_serial && _serial->available()) {
            uint8_t b = _serial->read();
            processByte(b);
        }
    }

    // Channel index: 0–13, returns 1000–2000
    int readChannel(uint8_t ch) const
    {
        if (ch >= CHANNEL_COUNT) return 1500;
        return _channels[ch];
    }

    bool isFrameValid() const
    {
        return _frameValid;
    }

private:
    HardwareSerial* _serial = nullptr;
    uint8_t  _frame[FRAME_SIZE];
    uint8_t  _frameIndex = 0;
    uint16_t _channels[CHANNEL_COUNT];
    bool     _frameValid = false;

    void processByte(uint8_t b)
    {
        // Wait for header
        if (_frameIndex == 0 && b != 0x20) return;
        if (_frameIndex == 1 && b != 0x40) {
            _frameIndex = 0;
            return;
        }

        _frame[_frameIndex++] = b;

        if (_frameIndex == FRAME_SIZE) {
            _frameIndex = 0;
            parseFrame();
        }
    }

    void parseFrame()
    {
        uint16_t checksum = 0xFFFF;
        for (uint8_t i = 0; i < FRAME_SIZE - 2; i++) {
            checksum -= _frame[i];
        }

        uint16_t rxChecksum =
            (uint16_t)_frame[FRAME_SIZE - 2] |
            ((uint16_t)_frame[FRAME_SIZE - 1] << 8);

        if (checksum != rxChecksum) {
            _frameValid = false;
            return;
        }

        for (uint8_t i = 0; i < CHANNEL_COUNT; i++) {
            uint8_t idx = 2 + i * 2;
            _channels[i] =
                (uint16_t)_frame[idx] |
                ((uint16_t)_frame[idx + 1] << 8);
        }

        _frameValid = true;
    }
};