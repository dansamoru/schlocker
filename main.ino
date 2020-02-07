/*
 * PROJECT SCHLOCKER
 *
 * Developer: Samoilov Daniil © 2020
 * VK: @dansamoru
 */



//  ===INCLUDE SETTINGS===

// For CLion:

#include <Arduino.h>
#include <Servo/Servo.h>


//For Arduino IDE:

//#include <Servo.h>



//  ===PROJECT SETTINGS===
#define CELL_QUANTITY 3  //  Quantity of cells

//  ===CIRCUIT SETTINGS===
#define CELL_START_PIN 2  //  First pin for cells
#define SCANNER_PIN 7  //  Pin for RFID-scanner
#define OPEN_BUTTON_PIN 8  //  Pin for "Green" button
#define DELETE_BUTTON_PIN 9  //  Pin for "Red" button

//  ===HARDWARE SETTINGS===
#define DEBUG true  //  Switch on debug (boolean)
#define OPEN_ANGLE 170  //  Cell opening angle
#define CLOSE_ANGLE 10  //  Cell closing angle (Default for first start)
#define SCANNER_WAIT_TIME 10000  //  Total delay for scanning
#define LOOP_DELAY 100  //  Delay in "loop"



//  ===CODE===

//  Class for debugging messages
class Debugger {
    bool lastEnds = true;  //  Had last flag "end"
public:

    //  Free RAM on the board
    int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    //  First message
    void loaded() {
        if (DEBUG) {
            Serial.println("###LOADED###");
            Serial.println("#TECHNICAL INFORMATION#");
            Serial.print("Free RAM: ");
            Serial.println(freeRam());
            Serial.println("###END###\n");
        }
    }

    //  Log acts
    void act(String object, String act, int number = -1, bool ending = true) {
        if (DEBUG) {
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
        }
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
public:
    PortIn(short _number) : Port(_number) {
        pinMode(number, INPUT);
    }
};

//  Class for ports with "OUTPUT" mode
class PortOut : protected Port {
public:
    PortOut(short _number) : Port(_number) {
        pinMode(number, OUTPUT);
    }
};

//  Class for ports with "INPUT_PULLUP" mode
class PortButton : protected Port {
public:
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
public:
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

//  Class for user with identity key
class User {
    int identity;
public:
    User(int _identity) {
        identity = _identity;
    }

    //  Get unique id
    int get_identity() {
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
public:
    static bool isAvailable() {
        return Serial.available() > 0;
    }

    //  Read current value
    int read() {
        return Serial.parseInt();
    }

    //  The cycle of scanning
    int scan() {
        debugger.act("Scanner", "Start scanning", -1, false);
        long startTime = millis();  //  Variable for timer
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                int userId = read();
                debugger.act("Scanner", "End scanning");
                return userId;
            }
        }
        debugger.act("Scanner", "End scanning (timeout)");
        return -1;
    }

};

//  Class for all led indication
class Indication {

};

//  Main class for section
class Schlocker {
    Cell *cells[CELL_QUANTITY];
    Scanner scanner;
    Indication indication;
    Button *greenButton = new Button(OPEN_BUTTON_PIN);
    Button *redButton = new Button(DELETE_BUTTON_PIN);
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
    }
};

Schlocker *schlocker;  //  It should be with "*"

void setup() {
    Serial.begin(9600);
    schlocker = new Schlocker();
    debugger.loaded();
}

void loop() {
    schlocker->refresh();
    delay(LOOP_DELAY);
}
