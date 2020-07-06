#pragma once
#include <Arduino.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>

class Scanner{
    Adafruit_PN532 rfid(30, 100); //не трогать, костыль
    uint8_t uid[8]{};
    uint8_t uidLength{};
    public:
        Scanner(unsigned int pin, unsigned long wait_time);
        unsigned long scan();
    private:
        bool isReadable();
        unsigned long read();
        unsigned long _wait_time;
};
