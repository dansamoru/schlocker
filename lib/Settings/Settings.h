//
// Created by dansamoru on 07.02.2021.
//

#ifndef SCHLOCKER_SETTINGS_H
#define SCHLOCKER_SETTINGS_H

//  =DEVELOPMENT=
/*
 * Set debugging
 *     0 - off
 */
#define DEBUG 0

/*
 * Set the level of input
 *
 * Input levels:
 *     0 - Input nothing
 *     1 - Input user only
 *     2 - Input everything
 */
#define INPUT_DEBUG 2

//  =PROJECT CONFIGURATIONS=
#define LOCKERS_QUANTITY 4
#define CARD_NUMBER_LENGTH 4
#define DELAY_TIME 100
#define LOCKER_DELAY 1000
#define INPUT_DELAY 10

// Pins
#define PIN_LOCKER_START 9
#define PIN_SENSOR_START 3

#define PIN_SCANNER 17

#define PIN_BUTTON_GREEN 15
#define PIN_BUTTON_RED 16


// === END OF SETTINGS===

#endif //SCHLOCKER_SETTINGS_H
