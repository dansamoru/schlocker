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
#define DEBUG 1

/*
 * Set the level of input
 *
 * Input levels:
 *     0 - Input nothing
 *     1 - Input user only
 *     2 - Input everything
 */
#define INPUT_DEBUG 0

//  =PROJECT CONFIGURATIONS=
#define LOCKERS_QUANTITY 1
#define CARD_NUMBER_LENGTH 4
#define DELAY_TIME 100
#define LOCKER_DELAY 1000
#define INPUT_DELAY 10

// Pins
#define PIN_LOCKER_START 9
#define PIN_SENSOR_START 5

#define PIN_SCANNER 4

#define PIN_BUTTON_GREEN A2
#define PIN_BUTTON_RED A3

#define PIN_LED_GREEN A0
#define PIN_LED_RED A1


// === END OF SETTINGS===

#endif //SCHLOCKER_SETTINGS_H
