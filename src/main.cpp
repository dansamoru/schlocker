#include <Arduino.h>

#include <Settings.h>
#include <Constants.h>
#include <Models.h>

#include <ArduinoLog.h>

Cell cells[LOCKERS_QUANTITY];
Scanner scanner(PIN_SCANNER, 100);
uint8_t vacant_lockers_quantity = LOCKERS_QUANTITY;

void smartdelay(unsigned long ms) {
    unsigned long realtime = millis();
    while ((millis() - realtime) < ms) {}
}

void pinsSetup() {
    pinMode(PIN_BUTTON_GREEN, INPUT_PULLUP);
    pinMode(PIN_BUTTON_RED, INPUT_PULLUP);
    pinMode(PIN_LED_GREEN, OUTPUT);
    pinMode(PIN_LED_RED, OUTPUT);
}

void cellsSetup() {
    Log.noticeln("\tPin setup started...");
    // Set up lockers
    Log.traceln("\tSet up lockers:");
    for (short i = 0; i < LOCKERS_QUANTITY; ++i) {
        cells[i].id = i;
        cells[i].locker_pin = PIN_LOCKER_START + i;
        cells[i].sensor_pin = PIN_SENSOR_START + i;

        pinMode(cells[i].locker_pin, OUTPUT);
        pinMode(cells[i].sensor_pin, INPUT_PULLUP);

        Log.traceln("\t\tLocker: num=%d, id=%d, lpin=%d, spin=%d",
                    i, cells[i].id, cells[i].locker_pin, cells[i].sensor_pin);
    }
    Log.infoln("\tPin setup successful");
}

void scannerSetup() {
#if INPUT_DEBUG == 0
    Log.noticeln("\tScanner setup started...");
    scanner.begin();
    if (!scanner.getFirmwareVersion()) {
        Log.fatalln("\t\tScanner setup failed");
        return;
    }
    scanner.SAMConfig();
    Log.infoln("\tScanner setup successful");
#endif
}

void updateVacantLockersQuantity(bool add) {
    Log.noticeln("Change system status (%d): %d", vacant_lockers_quantity, add ? 1 : -1);
    if (vacant_lockers_quantity == 0) {
        if (add) {
            vacant_lockers_quantity = 1;
            digitalWrite(PIN_LED_GREEN, HIGH);
            digitalWrite(PIN_LED_RED, LOW);
        }
    } else if (vacant_lockers_quantity == 1 && !add) {
        vacant_lockers_quantity = 0;
        digitalWrite(PIN_LED_GREEN, LOW);
        digitalWrite(PIN_LED_RED, HIGH);
    } else {
        vacant_lockers_quantity += add ? 1 : -1;
    }
}

ScannerAnswer get_scannerAnswer() {
    ScannerAnswer scannerAnswer;
    uint8_t uid[CARD_NUMBER_LENGTH];
#if INPUT_DEBUG == 0
    Log.traceln("Scanning started");
    scannerAnswer.success = scanner.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &scannerAnswer.uidLength);
#elif INPUT_DEBUG >= 1
    Log.verboseln("Enter uid, len=%d:", CARD_NUMBER_LENGTH);
    for (int i = 0; i < CARD_NUMBER_LENGTH; ++i) {
        Log.verboseln("\tuid[%d]?", i);
        while (!Serial.available()) {
            smartdelay(INPUT_DELAY);
        }
        uid[i] = Serial.read() - '0';
    }
    scannerAnswer.success = true;
    scannerAnswer.uidLength = CARD_NUMBER_LENGTH;
#endif
    if (scannerAnswer.success && scannerAnswer.uidLength == CARD_NUMBER_LENGTH) {
        for (int i = 0; i < scannerAnswer.uidLength; ++i) {
            scannerAnswer.uid[i] = uid[i];
        }
    } else {
        scannerAnswer.success = false;
    }
    return scannerAnswer;
}

void openLocker(uint8_t pin) {
    Log.noticeln("Locker open");
    Log.traceln("\tpin = %d", pin);
    digitalWrite(pin, HIGH);
    smartdelay(LOCKER_DELAY);
    digitalWrite(pin, LOW);
}

bool userRegister(const uint8_t *uid) {
    for (auto &cell: cells) {
        if (cell.is_vacant) {
            Log.noticeln("User registration...");
            Log.traceln("\tuid=");
            cell.is_vacant = false;
            for (int j = 0; j < CARD_NUMBER_LENGTH; ++j) {
                cell.card_uid[j] = uid[j];
                Log.traceln("\t\t%d", uid[j]);
            }
            openLocker(cell.locker_pin);
            return true;
        }
    }
    return false;
}

void openCell(uint8_t *uid, bool unregister = false) {
    for (auto &cell: cells) {
        if (!cell.is_vacant) {
            bool currentRegistration = true;
            for (int j = 0; j < CARD_NUMBER_LENGTH; ++j) {
                if (uid[j] != cell.card_uid[j]) {
                    currentRegistration = false;
                    break;
                }
            }
            if (currentRegistration) {
                openLocker(cell.locker_pin);
                if (unregister) {
                    Log.noticeln("User delete");
                    Log.traceln("\tLocker:\n\t\tid=%d\n\t\tlpin=%d", cell.id, cell.locker_pin);
                    Log.traceln("\tuid=");
                    for (unsigned char j: cell.card_uid) {
                        Log.traceln("\t\t%d", j);
                    }
                    cell.is_vacant = true;
                    updateVacantLockersQuantity(true);
                }
                return;
            }
        }
    }
    if (userRegister(uid)) {
        Log.infoln("User registered");
        updateVacantLockersQuantity(false);
    } else {
        Log.errorln("User registration failed");
    }
}

State getCurrentState() {
    State currentState;
#if INPUT_DEBUG == 2
    Log.verboseln("Input state:");
    Log.verboseln("\tgreenButton? (0 - pressed, 1)");
    while (!Serial.available()) { delay(INPUT_DELAY); }
    currentState.greenButton = (Serial.read() == '0' ? LOW : HIGH);
    Log.verboseln("\tredButton? (0 - pressed, 1)");
    while (!Serial.available()) { delay(INPUT_DELAY); }
    currentState.redButton = (Serial.read() == '0' ? LOW : HIGH);
#else
    currentState.greenButton = digitalRead(PIN_BUTTON_GREEN);
    currentState.redButton = digitalRead(PIN_BUTTON_RED);
#endif
    return currentState;
}

void update() {
    State currentState = getCurrentState();
    if (currentState.greenButton == LOW && currentState.redButton == HIGH) {
        Log.traceln("Only greenButton is pressed");
        ScannerAnswer scannerAnswer = get_scannerAnswer();
        if (scannerAnswer.success) {
            // TODO: убрать повторения
            Log.traceln("Got successful scannerAnswer");
            openCell(scannerAnswer.uid);
        } else {
            Log.warningln("Got wrong scannerAnswer");
        }
    } else if (currentState.greenButton == HIGH && currentState.redButton == LOW) {
        Log.traceln("Only redButton is pressed");
        ScannerAnswer scannerAnswer = get_scannerAnswer();
        if (scannerAnswer.success) {
            Log.traceln("Got successful scannerAnswer");
            openCell(scannerAnswer.uid, true);
        } else {
            Log.warningln("Got wrong scannerAnswer");
        }
    } else if (currentState.greenButton == LOW && currentState.redButton == LOW) {
        Log.traceln("Both buttons are pressed");
    } else {
        Log.traceln("None buttons are pressed");
    }
}

void setup() {
    // Turn on Serial for debugging
#if DEBUG != 0 || INPUT_DEBUG != 0
    Serial.begin(9600);
    while (!Serial && !Serial.available()) {}
    Log.begin(LOG_LEVEL_VERBOSE, &Serial);
#endif
    Log.infoln("\nSetup started...");

    pinsSetup();
    cellsSetup();
    scannerSetup();

    digitalWrite(PIN_LED_GREEN, HIGH);
    digitalWrite(PIN_LED_RED, LOW);
}

void loop() {
    update();
    smartdelay(DELAY_TIME);
}