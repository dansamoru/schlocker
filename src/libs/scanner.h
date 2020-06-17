#pragma once
#include <Arduino.h>
#include <Adafruit_PN532.h>

#define SCANNER_WAITTIME 1000

class Scanner{
    Adafruit_PN532 rfid;
    uint8_t uid[8]{};
    uint8_t uidLength{};
    public:
        Scanner(unsigned short pin);
        unsigned long scan();
    private:
        bool isReadable();
        unsigned long read();
};
