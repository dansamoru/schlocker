//#include <Arduino.h>
//#include <Servo/Servo.h>

#include <Servo.h>

//  Project settings:
#define CELL_QUANTITY 3

//  Scheme settings:
#define CELL_START_PIN 2
#define SCANNER_PIN 7
#define GREEN_BUTTON_PIN 8
#define RED_BUTTON_PIN 9

//  Hardware settings:
#define DEBUG true
#define OPEN_ANGLE 170
#define CLOSE_ANGLE 10
#define SCANNER_WAIT_TIME 10000
#define LOOP_DELAY 100


class Debugger {
    bool lastEnds = true;
public:
    int freeRam() {
        extern int __heap_start, *__brkval;
        int v;
        return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
    }

    void loaded() {
        if (DEBUG) {
            Serial.println("###LOADED###");
            Serial.println("#TECHNICAL INFORMATION#");
            Serial.print("Free RAM: ");
            Serial.println(freeRam());
            Serial.print("Serial: ");
            Serial.println(Serial?"YES":"NO");
            Serial.println("###END###\n");
        }
    }

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

class Port {
protected:
    short number;

    Port(short _number) {
        number = _number;
    }
};

class PortIn : protected Port {
public:
    PortIn(short _number) : Port(_number) {
        pinMode(number, INPUT);
    }
};

class PortOut : protected Port {
public:
    PortOut(short _number) : Port(_number) {
        pinMode(number, OUTPUT);
    }
};

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

class PortServo : protected Port {
    Servo *servo;
public:
    PortServo(short _number) : Port(_number) {

        servo = new Servo();
        servo->attach(number);
    }

        short read() {
        return servo->read();
    }

    void write(short angle) {
        Serial.println(number);
        Serial.println(read());
        Serial.println(angle);
        servo->write(angle);
        delay(1000);
        servo->write(10);

    }
};

class User {
    int identity;
public:
    User(int _identity) {
        identity = _identity;
    }

    int get_identity() {
        return identity;
    }
};

class Button : protected PortButton {
public:
    Button(short _number) : PortButton(_number) {}

    bool isPressed() {
        return !read();
    }
};

class Cell {
    int identity;
    int lastOpenTime;
    int registrationTime;
    bool isOpen;
    User *user;
    PortServo *port;
public:
    Cell(int _identity) {
        identity = _identity;
        user = new User(0);
        port = new PortServo(identity + CELL_START_PIN);
    }

    int userId() {
        return user->get_identity();
    }

    void close() {
        debugger.act("Cell", "Close", identity);
        isOpen = false;
        port->write(CLOSE_ANGLE);
    }

    void open() {
        debugger.act("Cell", "Open", identity);
        isOpen = true;
        port->write(OPEN_ANGLE);
    }

    void unreg(bool doOpen = true) {
        debugger.act("Cell", "Unregistration", identity, false);
        user = new User(0);
        if(doOpen){
            open();
        }
    }

    void reg(int userIdentity, bool doOpen = true) {
        debugger.act("Cell", "Registration", identity, false);
        user = new User(userIdentity);
        if(doOpen){
            open();
        }
    }
};

class Scanner {
public:
    static bool isAvailable() {
        return Serial.available() > 0;
    }

    int read() {
        return Serial.parseInt();
    }

    int scan() {
        debugger.act("Scanner", "Start scanning", -1, false);
        long startTime = millis();
        bool scanned = false;
        while (millis() - startTime <= SCANNER_WAIT_TIME) {
            if (isAvailable()) {
                int userId = read();
                debugger.act("Scanner", "End scanning");
                scanned = true;
                return userId;
                break;
            }
        }
        if (!scanned) {
            debugger.act("Scanner", "End scanning (timeout)");
        }
        return -1;
    }

};

class Indication {

};

class Schlocker {
    Cell *cells[CELL_QUANTITY];
    Scanner scanner;
    Indication indication;
    Button *greenButton = new Button(GREEN_BUTTON_PIN);
    Button *redButton = new Button(RED_BUTTON_PIN);
public:
    Schlocker() {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            cells[i] = new Cell(i);
        }
    }

    int cellSearch(int userId) {
        for (int i = 0; i < CELL_QUANTITY; ++i) {
            if (cells[i]->userId() == userId) {
                return i;
            }
        }
        return -1;
    }

    void refresh() {
        if (greenButton->isPressed()) {
          debugger.act("Green button", "Pressed");
            int userId = scanner.scan();
            if(userId != -1){
                int cellId = cellSearch(userId);
                if(cellId != -1){
                    cells[cellId]->open();
                }else{
                    cells[cellSearch(0)]->reg(userId);

                }
            }
        }
        if(redButton->isPressed()){
                    debugger.act("Red button", "Pressed");
            int userId = scanner.read();
            if(userId != -1){
                int cellId = cellSearch(userId);
                if(cellId != -1){
                    cells[cellId]->unreg();
                }
            }
        }
    }


};

Schlocker *schlocker;

void setup() {
    Serial.begin(9600);
    schlocker = new Schlocker();
    debugger.loaded();
}

void loop() {
    schlocker->refresh();
    delay(LOOP_DELAY);
}
