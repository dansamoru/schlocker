/*
 * PROJECT SCHLOCKER
 *
 * Developer: Samoilov Daniil © 2020
 * VK: @dansamoru
 */



//  ===INCLUDE SETTINGS===

//For Arduino IDE
#include <Wire.h>
#include <SPI.h>
#include <Servo.h>
#include <Adafruit_PN532.h>




//  ===PROJECT SETTINGS===
#define CELL_QUANTITY 3  //  Quantity of cells

//  ===CIRCUIT SETTINGS===
#define CELL_START_PIN 2  //  First pin for cells
#define SCANNER_PIN 7  //  Pin for RFID-scanner
#define OPEN_BUTTON_PIN 8  //  Pin for "Green" button
#define DELETE_BUTTON_PIN 9  //  Pin for "Red" button
#define GREEN_LED_PIN 20
#define YELLOW_LED_PIN 20
#define RED_LED_PIN 20

//  ===HARDWARE SETTINGS===
#define DEBUG true  //  Switch on debug (boolean)
#define OPEN_ANGLE 170  //  Cell opening angle
#define CLOSE_ANGLE 10  //  Cell closing angle (Default for first start)
#define SCANNER_WAIT_TIME 10000  //  Total delay for scanning
#define LOOP_DELAY 100  //  Delay in "loop"
#define SERIAL_SPEED 9600


//  ===CODE===

//  Class for debugging messages
class Debugger {
    bool lastEnds = true;  //  Had last flag "end"
public:

    //  Free RAM on the board
#if DEBUG

    int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

#endif

    //  First message
    void loaded() {
#if DEBUG
        if (DEBUG) {
            Serial.println("###LOADED###");
            Serial.println("#TECHNICAL INFORMATION#");
            Serial.print("Free RAM: ");
            Serial.println(freeRam());
            Serial.println("###END###\n");
        }
#endif
    }

    //  Acr logs
    void act(String object, String act, int number = -1, bool ending = true) {
#if DEBUG
        if (lastEnds) {
            Serial.println("#ACT#");
            Serial.print("Object: ");
            Serial.print(object);
            if (number != -1) {
                Serial.print(" (");
                Serial.print(number);
                Serial.print(")");
            }
            Serial.print("\n");
        }
        Serial.print("Act: ");
        Serial.println(act);
        lastEnds = false;
        if (ending) {
            lastEnds = true;
            Serial.println("#END#\n");
        }
#endif
    }

    //  Error logs
    void error(String object, String error, int number = -1) {
#if DEBUG
        Serial.println("#ERROR#");
        Serial.print("Object: ");
        Serial.print(object);
        if (number != -1) {
            Serial.print(" (");
            Serial.print(number);
            Serial.print(")");
        }
        Serial.print("\n");
        Serial.print("Error: ");
        Serial.println(error);
        Serial.println("#END#\n");
#endif
    }
};

Debugger debugger;

//  Basic class for all components
class Port {
protected:
    short number;

    Port(short _number) {
        number = _number;
    }
};

//  Class for ports with "INPUT" mode
class PortIn : protected Port {
protected:
    PortIn(short _number) : Port(_number) {
        pinMode(number, INPUT);
    }
};

//  Class for ports with "OUTPUT" mode
class PortOut : protected Port {
protected:
    PortOut(short _number) : Port(_number) {
        pinMode(number, OUTPUT);
    }

    void set(void value) {
        digitalWrite(number, value);
    }
};

//  Class for ports with "INPUT_PULLUP" mode
class PortButton : protected Port {
protected:
    PortButton(short _number) : Port(_number) {
        pinMode(number, INPUT_PULLUP);
        digitalWrite(number, HIGH);
    }

    bool read() {
        return digitalRead(number);
    }
};

//  Class for ports with servos
class PortServo : protected Port {
    Servo *servo;
protected:
    PortServo(short _number) : Port(_number) {

        servo = new Servo();
        servo->attach(number);
        servo->write(CLOSE_ANGLE);
    }

    //  Read current position of servo
    short read() {
        return servo->read();
    }

    //  Write new position for servo
    void write(short angle) {
        servo->write(angle);
    }
};

// Class for ports with RFID-scanner
class PortScanner : protected Port {
    Adafruit_PN532 *scanner;
    uint8_t card;
    uint8_t uid[8];
    uint8_t uidLength;
protected:
    PortScanner(short _number) : Port(_number) {
        scanner = new Adafruit_PN532(number, 100);
        scanner->begin();
        if (!scanner->getFirmwareVersion()) {
            debugger.error("Scanner", "Didn't find");
        }
        scanner->SAMConfig();
    }

    bool isAvailable() {
        card = scanner->readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        return card;
    }

    long get_uid() {
        return *(long *) uid;
    }
};

//  Class for user with identity key
class User {
    long identity;
public:
    User(int _identity) {
        identity = _identity;
    }

    //  Get unique id
    long get_identity() {
        return identity;
    }
};

//  Class for buttons
class Button : protected PortButton {
public:
    Button(short _number) : PortButton(_number) {}

    //  Is button pressed in this moment
    bool isPressed() {
        return !read();
    }
};

//  Class for indication bulbs
class Bulb : protected PortOut {
public:
    Bulb(_number) : PortOut(_number);

    void turnOn() {
        set(HIGH);

    }

    void turnOff() {
        set(LOW);
    }
}

//  Component of section
class Cell {
    int identity;  //  Number of cell
    int lastOpenTime;  //  Time when cell was opened last time
    int registrationTime;  //  Time when cell was registered
    bool isOpen;
    User *user;
    PortServo *port;
public:
    Cell(int _identity) {
        identity = _identity;
        user = new User(0);
        port = new PortServo(identity + CELL_START_PIN);
    }

    //  Get unique user id
    int userId() {
        return user->get_identity();
    }

    //  Close cell
    void close() {
        debugger.act("Cell", "Close", identity);
        isOpen = false;
        port->write(CLOSE_ANGLE);
    }

    //  Open cell
    void open() {
        debugger.act("Cell", "Open", identity);
        isOpen = true;
        port->write(OPEN_ANGLE);
    }

    //  Unregister recorded user
    void unreg(bool doOpen = true) {
        debugger.act("Cell", "Unregistration", identity, false);
        user = new User(0);
        if (doOpen) {
            open();
        }
    }

    //  Register new user for cell
    void reg(int userIdentity, bool doOpen = true) {
        debugger.act("Cell", "Registration", identity, false);
        user = new User(userIdentity);
        if (doOpen) {
            open();
        }
    }
};

//  Class for RFID-scanner
class Scanner {
    PortScanner *port;
public:
    Scanner() {
        port = new PortServo(SCANNER_PIN);
    }

    bool isAvailable() {
        return port->IsAvailable();
    }

    //  Read current value
    long read() {
        return port->get_uid();
    }

    //  The cycle of scanning
    long scan() {
        debugger.act("Scanner", "Start scanning", -1, false);
        long startTime = millis();  //  Variable for timer
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                long uid = read();
                debugger.act("Scanner", "End scanning");
                return uid;
            }
        }
        debugger.act("Scanner", "End scanning (timeout)");
        return -1;
    }

};

//  Class for all led indication
class Indication {
    Bulb *green;
    Bulb *yellow;
    Bulb *red;
    short mode;
public:
    Indication(short green_number, short yellow_number, short red_number) {
        green = new Bulb(GREEN_LED_PIN);
        yellow = new Bulb(YELLOW_LED_PIN);
        red = new Bulb(RED_LED_PIN);
    }

    //  Set indication mode
    void set_mode(short value) {
        mode = value;
    }

    void refresh() {
        switch (mode) {
            case 0:
                green.turnOn();
                yellow.turnOff();
                red.turnOff();
                break;
            case 1:
                green.turnOff();
                yellow.turnOn();
                red.turnOff();
                break;
            case 2:
                green.turnOff();
                yellow.turnOff();
                red.turnOn();
                break;
            case 3:
                green.turnOn();
                yellow.turnOn();
                red.turnOff();
                break;
            case 2:
                green.turnOff();
                yellow.turnOn();
                red.turnOn();
                break;
        }
    }
};

//  Main class for section
class Schlocker {
    Cell *cells[CELL_QUANTITY];
    Scanner scanner;
    Indication indication;
    Button *greenButton = new Button(OPEN_BUTTON_PIN);
    Button *redButton = new Button(DELETE_BUTTON_PIN);
    short stat;  //  Section status: 0 - green, 1 - yellow, 2 - red, 3 - green/yellow, 4 - red/yellow
public:
    Schlocker() {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            cells[i] = new Cell(i);
        }
    }

    //  Search cell with userId
    int cellSearch(int userId) {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            if (cells[i]->userId() == userId) {
                return i;
            }
        }
        return -1;
    }

    //  Refresh system status
    void refresh() {
        if (greenButton->isPressed()) {
            debugger.act("Green button", "Pressed");
            int userId = scanner.scan();
            if (userId != -1) {
                int cellId = cellSearch(userId);
                if (cellId != -1) {
                    cells[cellId]->open();
                } else {
                    cells[cellSearch(0)]->reg(userId);

                }
            }
        }
        if (redButton->isPressed()) {
            debugger.act("Red button", "Pressed");
            int userId = scanner.scan();
            if (userId != -1) {
                int cellId = cellSearch(userId);
                if (cellId != -1) {
                    cells[cellId]->unreg();
                }
            }
        }
        for (int i = 0; i < CELL_QUANTITY; i++) {

        }
        Indication.refresh();
    }

};

Schlocker *schlocker;  //  It should be with "*"

void setup() {
    Serial.begin(SERIAL_SPEED);
    schlocker = new Schlocker();
    debugger.loaded();
}

void loop() {
    schlocker->refresh();
    delay(LOOP_DELAY);
}
