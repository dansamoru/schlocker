/*
*
* PROJECT SCHLOCKER
* v.0.2.0.2
*
* Developer: Samoilov Daniil © 2020
* VK: @dansamoru
*/


//  ===INCLUDE SETTINGS===

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>


//  ===PROJECT SETTINGS===
#define CELL_QUANTITY 4

//  ===CIRCUIT SETTINGS===
#define LOCKER_START_PIN 9  //  First pin for cells
#define LOCKER_SENSOR_START_PIN 5  //  First pin for checkers
#define SCANNER_PIN 19  //  Pin for RFID-scanner
#define OPEN_BUTTON_PIN 14  //  Pin for "Green" button
#define DELETE_BUTTON_PIN 15  //  Pin for "Red" button
#define GREEN_LED_PIN 2
#define YELLOW_LED_PIN 3
#define RED_LED_PIN 4

//  ===HARDWARE SETTINGS===
#define DEBUG true  //  Switch on debug (boolean)
#define SCANNER_WAIT_TIME 1000  //  Total delay for scanning
#define LOOP_DELAY 100  //  Delay in "loop"
#define SERIAL_SPEED 9600
#define LOCKER_MAX_TIME 4000 //  Max time for locker opening in milliseconds
#define LOCKER_SENSOR_DEFAULT_VALUE false  //  Default value when locker is closed


//  ===CODE===

// ==SYSTEM==

class System{
public:
    //EEPROM here
};

//  Class for debugging messages
class Debugger {
    bool lastEnds = true;  //  Had last flag "end"
public:
    //  Free RAM on the board
#if DEBUG
    static int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }
#endif
    //  First message
    static void loaded() {
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
    //  Act logs
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
    static void error(String object, String error, int number = -1) {
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

//  ==PORTS==

//  Basic class for all components
class Port {
protected:
    unsigned short portNumber;
    explicit Port(unsigned short _portNumber) : portNumber(_portNumber) {}
};

//  Class for ports with "INPUT" mode
class InPort : protected Port {
protected:
    explicit InPort(unsigned short _portNumber) : Port(_portNumber) {
        pinMode(portNumber, INPUT);
    }
};

//  Class for ports with "OUTPUT" mode
class OutPort : protected Port {
protected:
    explicit OutPort(unsigned short _number) : Port(_number) {
        pinMode(portNumber, OUTPUT);
    }

    void set(bool value) {
        digitalWrite(portNumber, value);
    }
};

//  Class for ports with "INPUT_PULLUP" mode
class PullupPort : protected Port {
protected:
    explicit PullupPort(unsigned short _portNumber) : Port(_portNumber) {
        pinMode(portNumber, INPUT_PULLUP);
        digitalWrite(portNumber, HIGH);
    }

    bool read() {
        return digitalRead(portNumber);
    }
};

//  Class for lockers
class LockerPort :  protected OutPort, protected InPort{
public:
    explicit LockerPort(unsigned short _lockerPortNumber, unsigned short _sensorPortNumber) :
            OutPort(_lockerPortNumber),
            InPort(_sensorPortNumber){}

    //  Open locker
    void open(unsigned int milliseconds = 1000) {
        if(milliseconds > LOCKER_MAX_TIME){
            return;
        }
        digitalWrite(OutPort::portNumber, HIGH);
        delay(milliseconds); // TODO: переделать задержку
        digitalWrite(OutPort::portNumber, LOW);
    }

    //  Is locker opened
    bool is_open() {
        return (digitalRead(InPort::portNumber)!=LOCKER_SENSOR_DEFAULT_VALUE);  // Compare with default value
    }


};

// Class for ports with RFID-scanner
class PortScanner : protected Port {
    Adafruit_PN532 scanner = Adafruit_PN532(portNumber, 100);
    uint8_t card{};
    uint8_t uid[8]{};
    uint8_t uidLength{};
protected:
    explicit PortScanner(unsigned short _portNumber) : Port(_portNumber) {
        scanner.begin();
        if (!scanner.getFirmwareVersion()) {
            debugger.error("Scanner", "Didn't find");
        }
        scanner.SAMConfig();
    }

    bool isReadable() {
        card = scanner.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength);
        return card;
    }

    unsigned long get_uid() {
        return *(unsigned long *) uid;
    }
};

//  Class for user with identity key
class User {
    unsigned long uid;
public:
    unsigned long get_uid() const {
        return uid;
    }

    void set_uid(unsigned long _uid) {
        uid = _uid;
    }
};

/*
class Buzzer : protected OutPort {
public:
    explicit Buzzer(unsigned short _portNumber) : OutPort(_portNumber) {}

    void play(unsigned short mode) {
        debugger.act("Buzzer", "Play");
        switch (mode) {
            case 0:
                tone(BUZZER_TON_1, portNumber, 500);
                break;
        }
    }
};
Buzzer buzzer = Buzzer(BUZZER_PIN);
 */

//  Class for buttons
class Button : protected PullupPort {
public:
    explicit Button(unsigned short _portNumber) : PullupPort(_portNumber) {}

    //  Is button pressed in this moment
    bool isPushed() {
        return !read();
    }
};

//  Class for indication bulbs
class Led : protected OutPort {
public:
    explicit Led(unsigned short _portNumber) : OutPort(_portNumber) {}

    void turnOn() {
        set(HIGH);

    }

    void turnOff() {
        set(LOW);
    }
};

//  Component of section
class Cell : protected LockerPort {
    unsigned short identity;  //  Number of cell
    unsigned int lastOpenTime = 0;  //  Time when cell was opened last time
    unsigned int registrationTime = 0;  //  Time when cell was registered
    bool isOpen = false;
    User user;
public:
    explicit Cell(short _portNumber) : LockerPort(_portNumber + CELL_START_PIN) {
        identity = portNumber - CELL_START_PIN;
    }

    //  Get unique user id
    unsigned long userId() {
        return user.get_uid();
    }

    //  Open cell
    void open() {
        debugger.act("Cell", "Open", identity);
        isOpen = true;
        open();
    }

    //  Unregister recorded user
    void unreg(bool doOpen = true) {
        debugger.act("Cell", "Unregistration", identity, false);
        user.set_uid(0);
        if (doOpen) {
            open();
        }
    }

    //  Register new user for cell
    void reg(unsigned long userIdentity, bool doOpen = true) {
        debugger.act("Cell", "Registration", identity, false);
        user.set_uid(userIdentity);
        if (doOpen) {
            open();
        }
    }

    bool get_isOpen() {
        return isOpen;
    }
};

//  Class for RFID-scanner
class Scanner : protected PortScanner {
public:
    Scanner(unsigned short _portNumber) : PortScanner(_portNumber) {}

    bool isAvailable() {
        return isReadable();
    }

    //  Read current value
    unsigned long read() {
        return get_uid();
    }

    //  The cycle of scanning
    unsigned long scan() {
        buzzer.play(0);
        debugger.act("Scanner", "Start scanning", -1, false);
        unsigned long startTime = millis();  //  Variable for timer
        delay(1);
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                unsigned long uid = read();
                debugger.act("Scanner", "End scanning");
                return uid;
            }
            //delay(LOOP_DELAY / 10);
        }
        debugger.act("Scanner", "End scanning (timeout)");
        return 0;
    }

};

//  Class for all led indication
class Indication {
    Bulb green = Bulb(GREEN_LED_PIN);
    Bulb yellow = Bulb(YELLOW_LED_PIN);
    Bulb red = Bulb(RED_LED_PIN);
    unsigned short mode = 0;
public:

    //  Set indication mode
    void set_mode(unsigned short value) {
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
            case 4:
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
    Scanner scanner = Scanner(SCANNER_PIN);
    Indication indication;
    Button greenButton = Button(OPEN_BUTTON_PIN);
    Button redButton = Button(DELETE_BUTTON_PIN);
    Button closeButton = Button(CLOSE_BUTTON_PIN);
    unsigned short stat = 0;  //  Section status: 0 - green, 1 - yellow, 2 - red, 3 - green/yellow, 4 - red/yellow
public:
    Schlocker() {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            cells[i] = new Cell(i);
        }
        indication.refresh();
    }

    //  Search cell with userId
    unsigned short cellSearch(unsigned long userId) {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            if (cells[i]->userId() == userId) {
                return i + 1;
            }
        }
        return 0;
    }

    void updateStat() {
        bool isGreen = false;
        for (int i = 0; i < CELL_QUANTITY; i++) {
            if (cells[i]->userId() == 0) {
                isGreen = true;
            }
        }
        if (isGreen) {
            stat = 0;
        } else {
            stat = 2;
        }
        indication.set_mode(stat);
    }

    //  Refresh system status
    void refresh() {
        updateStat();
        if (greenButton.isPushed()) {
            debugger.act("Green button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId - 1]->open();
                } else {
                    cells[cellSearch(0) - 1]->reg(userId);

                }
            }
        }
        redButton.isPushed();
        if (redButton.isPushed()) {
            debugger.act("Red button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId - 1]->unreg();
                }
            }
        }
        if (closeButton.isPushed()) {
            debugger.act("Close button", "Pressed");
            for (int i = 0; i < CELL_QUANTITY; i++) {
                if (cells[i]->get_isOpen()) {
                    cells[i]->close();
                }
            }
        }
        indication.refresh();
    }

};

Schlocker schlocker;

void setup() {
    Serial.begin(SERIAL_SPEED);
    debugger.loaded();
}

void loop() {
    schlocker.refresh();
    delay(LOOP_DELAY);
}