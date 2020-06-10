/*
 *
 * PROJECT SCHLOCKER!
 *
 * Developer: Samoilov Daniil © 2020
 * VK: @dansamoru
 *
*/

//  ===SETTINGS===

//  ==INCLUDE SETTINGS==

#include <Wire.h>
#include <SPI.h>
#include <Adafruit_PN532.h>
#include <EEPROM.h>

//  ==PROJECT SETTINGS==
#define CELL_QUANTITY 4

//  ==CIRCUIT SETTINGS==
#define LOCKER_START_PIN 9  //  First pin for cells
#define LOCKER_SENSOR_START_PIN 5  //  First pin for checkers
#define SCANNER_PIN 19  //  Pin for RFID-scanner
#define OPEN_BUTTON_PIN 14  //  Pin for "Green" button
#define DELETE_BUTTON_PIN 15  //  Pin for "Red" button
#define GREEN_LED_PIN 2
#define YELLOW_LED_PIN 3
#define RED_LED_PIN 4

//  ==HARDWARE SETTINGS==
#define DEBUG true  //  Switch on debug (boolean)
#define SCANNER_WAIT_TIME 1000  //  Total delay for scanning
#define LOOP_DELAY 100  //  Delay in "loop"
#define SERIAL_SPEED 9600
#define LOCKER_MAX_TIME 4000 //  Max time for locker opening in milliseconds
#define LOCKER_SENSOR_DEFAULT_VALUE false  //  Default value when locker is closed


//  ===CODE===


// ==SYSTEM==

class System {
public:
    //  Class for EEPROM
    class Memory {
    public:
        //  Initialize
        static bool begin();

        //  Get userId from EEPROM
        static unsigned long get_userId(unsigned short cellNumber);

        //  Set userId from EEPROM
        static void set_userId(unsigned short cellNumber, unsigned long userId);

        //  Delete userId from EEPROM
        static void del_userId(unsigned short cellNumber);
    };

#if DEBUG
    //  Reset EEPROM
    static void reset();
#endif

    //  Start EEPROM
    static void begin();
};

//  Class for debugging messages
static class Debugger {
    #if DEBUG
        //  Free RAM on the board
        static int freeRam();
    #endif
public:
    //  First message
    static void begin();

    //  Act logs
    void act(String object, String act, int number = -1);

    //  Error logs
    static void error(String object, String error, int number = -1);

    //  User interface
    static void checkConsole();
};


//  ==PORTS==

//  Basic class for any port
class Port {
protected:
    unsigned short portNumber;
    explicit Port(unsigned short _portNumber) : portNumber(_portNumber);
};

//  Class for port with "INPUT" mode
class InPort : protected Port {
protected:
    explicit InPort(unsigned short _portNumber) : Port(_portNumber);
};

//  Class for port with "OUTPUT" mode
class OutPort : protected Port {
protected:
    explicit OutPort(unsigned short _number) : Port(_number);

    //  Set HIGH or LOW
    void set(bool value);
};

//  Class for port with "INPUT_PULLUP" mode
class PullupPort : protected Port {
protected:
    explicit PullupPort(unsigned short _portNumber) : Port(_portNumber);

    //  Read value of port
    bool read();
};


// ==COMPONENTS==

//  Class for locker
class LockerPort : protected OutPort, protected InPort {
public:
    explicit LockerPort(unsigned short _lockerPortNumber, unsigned short _sensorPortNumber) :
            OutPort(_lockerPortNumber),
            InPort(_sensorPortNumber);

    //  Open locker
    void open(unsigned int milliseconds = 1000);

    //  Is locker opened
    bool is_open();


};

// Class for port with RFID-scanner
class PortScanner : protected Port {
    Adafruit_PN532 scanner = Adafruit_PN532(portNumber, 100);
    uint8_t card{};
    uint8_t uid[8]{};
    uint8_t uidLength{};
protected:
    explicit PortScanner(unsigned short _portNumber) : Port(_portNumber);

    //  Check for scanner readability
    bool isReadable();

    //  Get userId
    unsigned long get_uid();
};

//  Class for bugs
class Buzzer : protected OutPort {
public:
    explicit Buzzer(unsigned short _portNumber) : OutPort(_portNumber);

    //  Play Minecraft song
    void play(unsigned short mode);
};

//  Class for button
class Button :
        protected PullupPort {
public:
    explicit Button(unsigned short _portNumber) : PullupPort(_portNumber);

    //  Is button pressed
    bool isPushed();
};

//  Class for indication led
class Led :
        protected OutPort {
public:
    explicit Led(unsigned short _portNumber) : OutPort(_portNumber);

    void turnOn();

    void turnOff();
};

//  Component of section
class Cell : protected LockerPort {
    unsigned short cellNumber;
//unsigned int lastOpenTime = 0;  //  Time when cell was opened last time
//unsigned int registrationTime = 0;  //  Time when cell was registered
public:
    explicit Cell(short _portNumber) : LockerPort(_portNumber + CELL_START_PIN, _portNumber + CELL_SENSOR_START_PIN);

    //  Get unique userId
    unsigned long get_userId();

    //  Open the cell
    void open();

    //  Unregister current user
    void unreg(bool doOpen = true);

    //  Register new user
    void reg(unsigned long userId, bool open = true);

    //  Is the cell opened
    bool isOpen();
};

//  Class for RFID-scanner
class Scanner :
        protected PortScanner {
public:
    Scanner(unsigned short _portNumber) : PortScanner(_portNumber);

    //  Check for scanner available
    bool isAvailable();

    //  Read current value
    unsigned long read();

    //  TODO: Чекнуть нужен ли цикл
    //  The cycle of scanning
    unsigned long scan();

};

//  Class for all led indication
class Indication {
    Led green(GREEN_LED_PIN);
    Led yellow(YELLOW_LED_PIN);
    Led red(RED_LED_PIN);
    unsigned short mode = 0;
public:

    //  Set indication mode
    void set_mode(unsigned short value);

    //  Update
    void refresh();
};

//  Main class for section
class Section {
    Cell cells[CELL_QUANTITY];
    Scanner scanner(SCANNER_PIN);
    Indication indication;
    Button greenButton(OPEN_BUTTON_PIN);
    Button redButton(DELETE_BUTTON_PIN);

    unsigned short stat = 0;  //  Section status: 0 - green, 1 - yellow, 2 - red, 3 - green/yellow, 4 - red/yellow
public:
    Section();

    //  Search cell with userId
    unsigned short cellSearch(unsigned long userId);

    void update_stat();

    //  Refresh system status
    void refresh();

};

Section section;

void setup() {
    System::begin();
}

void loop() {
    section.refresh();
    delay(LOOP_DELAY);
}


//  ===CODE===

class System {
public:
    class Memory {
    public:
        static bool begin() {
            switch (EEPROM.read(0)) {
                case 255:
                    EEPROM.update(0, CELL_QUANTITY);  //  Return true in next line
                case CELL_QUANTITY:
                    return true;
                default:
                    return false;
            }
        }

        static unsigned long get_userId(unsigned short cellNumber) {
            byte buffer[4];
            for (unsigned short i = 1; i < 5; i++) {
                buffer[i] = EEPROM.read(cellNumber * 4 + i);
            }
            return (unsigned long &) buffer;
        }

        static void set_userId(unsigned short cellNumber, unsigned long userId) {
            byte buffer[4];
            (unsigned long &) buffer = userId;
            for (unsigned short i = 1; i < 5; i++) {
                EEPROM.update(cellNumber * 4 + i, buffer[i]);
            }
        }

        static void del_userId(unsigned short cellNumber) {
            byte buffer[]{
                    255, 255, 255, 255
            };
            for (unsigned short i = 1; i < 5; i++) {
                EEPROM.update(cellNumber * 4 + i, buffer[i]);
            }
        }
    };

#if DEBUG
    static void reset() {
        for (int i = 0; i < EEPROM.length(); i++) {
            EEPROM.update(i, 255);
        }
    }
#endif

    static void begin() {
        System::Memory::begin();
        Serial.begin(SERIAL_SPEED);
        Debugger.begin();
    }
};

//  Class for debugging messages
static class Debugger {
    static bool lastEnds = true;  //  Had last flag "end"
public:

#if DEBUG

//  Free RAM on the board
    static int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

#endif

//  First message
    static void begin() {
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

        static void checkConsole() {
            if (Serial.available() > 0) {
                string tmp = Serial.readString()
                if (mp == "MEMORY_RESET") {
                    System.Memory.reset();
                } else {
                    Serial.println("Лол ты чо мне пишешь,F");
                }
            }
        }
    }
};

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

class LockerPort : protected OutPort, protected InPort {
public:
    explicit LockerPort(unsigned short _lockerPortNumber, unsigned short _sensorPortNumber) :
            OutPort(_lockerPortNumber),
            InPort(_sensorPortNumber) {}

    void open(unsigned int milliseconds = 1000) {
        if (milliseconds > LOCKER_MAX_TIME) {
            return;
        }
        digitalWrite(OutPort::portNumber, HIGH);
        delay(milliseconds); // TODO: переделать задержку
        digitalWrite(OutPort::portNumber, LOW);
    }

    bool is_open() {
        return (digitalRead(InPort::portNumber) != LOCKER_SENSOR_DEFAULT_VALUE);  // Compare with default value
    }


};

class PortScanner : protected Port {
    Adafruit_PN532 scanner = Adafruit_PN532(portNumber, 100);
    uint8_t card{};
    uint8_t uid[8]{};
    uint8_t uidLength{};
protected:
    explicit PortScanner(unsigned short _portNumber) : Port(_portNumber) {
        scanner.begin();
        if (!scanner.getFirmwareVersion()) {
            Debugger.error("Scanner", "Didn't find");
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

class Buzzer : protected OutPort {
public:
    explicit Buzzer(unsigned short _portNumber) : OutPort(_portNumber) {}

    void play(unsigned short mode) {
        Debugger.act("Buzzer", "Play");
        switch (mode) {
            case 0:
                tone(BUZZER_TON_1, portNumber, 500);
                break;
        }
    }
};

//  Class for buttons
class Button : protected PullupPort {
public:
    explicit Button(unsigned short _portNumber) : PullupPort(_portNumber) {}

    bool isPushed() {
        return !read();
    }
};


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

class Cell : protected LockerPort {
    unsigned short cellNumber;
//unsigned int lastOpenTime = 0;  //  Time when cell was opened last time
//unsigned int registrationTime = 0;  //  Time when cell was registered
public:
    explicit Cell(short _portNumber) : LockerPort(_portNumber + CELL_START_PIN, _portNumber + CELL_SENSOR_START_PIN) {
        cellNumber = portNumber - CELL_START_PIN;
    }

    unsigned long userId() {
        return System.Memory.get_userId(cellNumber);
    }

    void open() {
        Debugger.act("Cell", "Open", cellNumber);
        open();
    }

    void unreg(bool doOpen = true) {
        Debugger.act("Cell", "Unregistration", cellNumber, false);
        System.Memory.del_userId(cellNumber);
        if (doOpen) {
            open();
        }
    }

    void reg(unsigned long userId, bool open = true) {
        Debugger.act("Cell", "Registration", cellNumber, false);
        System.Memory.set_userId(cellNumber, userId);
        if (open) {
            open();
        }
    }

    bool get_isOpen() {
        return isOpen;
    }
};

class Scanner : protected PortScanner {
public:
    Scanner(unsigned short _portNumber) : PortScanner(_portNumber) {}

    bool isAvailable() {
        return isReadable();
    }

    unsigned long read() {
        return get_uid();
    }

    unsigned long scan() {
        buzzer.play(0);
        Debugger.act("Scanner", "Start scanning", -1, false);
        unsigned long startTime = millis();  //  Variable for timer
        delay(1);
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                unsigned long uid = read();
                Debugger.act("Scanner", "End scanning");
                return uid;
            }
//delay(LOOP_DELAY / 10);  //  ЧТО ЭТО
        }
        Debugger.act("Scanner", "End scanning (timeout)");
        return 0;
    }

};

class Indication {
    Led green(GREEN_LED_PIN);
    Led yellow(YELLOW_LED_PIN);
    Led red(RED_LED_PIN);
    unsigned short mode = 0;
public:
    void setMode(unsigned short value) {
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

class Section {
    Cell cells[CELL_QUANTITY];
    Scanner scanner(SCANNER_PIN);
    Indication indication;
    Button greenButton(OPEN_BUTTON_PIN);
    Button redButton(DELETE_BUTTON_PIN);
    Button closeButton(CLOSE_BUTTON_PIN);
    unsigned short stat = 0;  //  Section status: 0 - green, 1 - yellow, 2 - red, 3 - green/yellow, 4 - red/yellow
public:
    Section() {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            cells[i] = new Cell(i);
        }
//indication.refresh();  //  Эт зачем
    }

    unsigned short cellSearch(unsigned long userId) {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            if (cells[i].userId() == userId) {
                return i + 1;
            }
        }
        return 0;
    }

    void updateStat() {
        bool isGreen = false;
        for (int i = 0; i < CELL_QUANTITY; i++) {
            if (cells[i].userId() == 0) {
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

    void refresh() {
        updateStat();
        if (greenButton.isPushed()) {
            Debugger.act("Green button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId - 1].open();
                } else {
                    cells[cellSearch(0) - 1].reg(userId);
                }
            }
        }
        redButton.isPushed();
        if (redButton.isPushed()) {
            Debugger.act("Red button", "Pressed");
            unsigned long userId = scanner.scan();
            if (userId != 0) {
                unsigned short cellId = cellSearch(userId);
                if (cellId != 0) {
                    cells[cellId - 1].unreg();
                }
            }
        }
        if (closeButton.isPushed()) {
            Debugger.act("Close button", "Pressed");
            for (int i = 0; i < CELL_QUANTITY; i++) {
                if (cells[i].get_isOpen()) {
                    cells[i].close();
                }
            }
        }
        indication.refresh();  //  А здесь оно есть
    }
};

Section section;

void setup() {
    System::begin();
}

void loop() {
    section.refresh();
    delay(LOOP_DELAY);
}
