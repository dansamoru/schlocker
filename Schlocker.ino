/*
 *
 * PROJECT SCHLOCKER!
 *
 * Developer: Dneprov Stepan © 2020
 * VK: @pegaster
 *
*/


//  ===SETTINGS===

//  ==INCLUDES==
//  =SYSTEM LIBRARIES=
#include <EEPROM.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
//  =USER LIBRARIES=
#include "src/schlockerFunc.h"

//  ===USE===
void setup(){
    Serial.begin(SERIAL_SPEED);
    pinModes();
    scannerSetup();
    cellSetup();
    memorySetup();
}
void loop(){
    update();
    delay(LOOP_DELAY);
}

void update(){
    unsigned short status = update_status();
    unsigned short cell_number;
    indicate(status);
    unsigned long userId;
    if(digitalRead(greenBtn) == LOW && digitalRead(redBtn) == HIGH){
        userId = scan();
        if(userId != NULL){
            cell_number = findCellNumber(userId);
            if(cell_number == NULL && status < 2){
                cell_number = regUser(userId);
            }
            if(cell_number != NULL){
                open_cell(cell_number);
                rewriteLastOpenTime(cell_number);
            }
        }
    }
    else if(digitalRead(redBtn) == LOW && digitalRead(greenBtn) == HIGH){
        userId = scan();
        cell_number = findCellNumber(userId);
        if(cell_number != NULL){
            unregUser(cell_number);
            open_cell(cell_number);
        }
    }
}
