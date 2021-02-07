//
// Created by dansamoru on 07.02.2021.
//

#ifndef SCHLOCKER_SETTINGS_H
#define SCHLOCKER_SETTINGS_H

//  =DEVELOPMENT=
/*
 * Set the level of debugging
 *
 * Debug levels:
 *     0 - Print nothing [production]
 *     1 - Print errors only [errors]
 *     2 - Print main events [main]
 *     3 - Print everything [detail]
 */
#define DEBUG 3

/*
 * Set the level of input
 *
 * Input levels:
 *     0 - Input nothing
 *     1 - Input user
 *     2 - Input everything (not supported)
 */
#define INPUT_DEBUG 1

//  =PROJECT CONFIGURATIONS=
#define LOCKERS_QUANTITY 4
#define CARD_NUMBER_LENGTH 8
#define DELAY_TIME 100
#define LOCKER_DELAY 1000

// Pins
#define PIN_LOCKER_START 1
#define PIN_SENSOR_START 6

#define PIN_SCANNER 0

#define PIN_BUTTON_GREEN 0
#define PIN_BUTTON_RED 0


// === END OF SETTINGS===

// System debug configuration
#if DEBUG == 3
#define DEBUG_detail(val) Serial.print(val);
#define DEBUG_main(val) Serial.print(val);
#define DEBUG_error(val) Serial.print(val);
#elif DEBUG == 2
#define DEBUG_detail(val)
#define DEBUG_main(val) Serial.print(val);
#define DEBUG_error(val) Serial.print(val);
#elif DEBUG == 1
#define DEBUG_detail(val)
#define DEBUG_main(val)
#define DEBUG_error(val) Serial.print(val);
#else
#define DEBUG_detail(val)
#define DEBUG_main(val)
#define DEBUG_error(val)
#endif

#endif //SCHLOCKER_SETTINGS_H
