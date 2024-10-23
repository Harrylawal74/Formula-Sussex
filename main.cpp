#include <Arduino.h>
#include <CAN.h>
#include <LiquidCrystal_I2C.h>
#include <DigiPotX9Cxxx.h>

LiquidCrystal_I2C lcd(0x27,16,2);  // set the LCD address to 0x27 for a 16 chars and 2 line display
DigiPot coolFan(6, 4);

int sendDelay = 250;
int lastTime = millis();
int kill = false;

unsigned long TSAlastdown = 0;
bool TSAreleased = true;

unsigned long RTDlastdown = 0;
bool RTDreleased = true;

unsigned long lastMillis = millis();

//   Accel, brake, can, drive
enum S {ACC, BRK, CANN, DRV};
int STATUS[] = {-1, -1, -1, -1};

String getStatus();

void readCan(int);

float unpackFloat(const void *, int *);

void sendMsg();

void setup() {
    pinMode(2, OUTPUT);  // TSA relay
    pinMode(5, OUTPUT);  // APPS relay

    pinMode(A5, OUTPUT); // Buzzer
    pinMode(A6, OUTPUT); // Brake light
    Serial.begin(115200);
    lcd.init();  // initialize the lcd

    delay(500);
    lcd.backlight();
    lcd.setCursor(0, 0);
    Serial.print(" Sussex  Racing ");
    lcd.setCursor(0, 1);
    lcd.print("===== V0.1 =====");

    delay(500);

    coolFan.set(0);

    // start the CAN bus at 500 kbps
    if (!CAN.begin(500E3)) {
        Serial.println("Starting CAN failed!");
        STATUS[CANN] = 1;
    } else {
        Serial.println("Starting CAN success!");
        STATUS[CANN] = 0;
        CAN.onReceive(readCan);
        delay(1000);
    }

    STATUS[DRV] = 3;
}

void loop() {
    // Read sensors and data

    float APPS1 = analogRead(A0) / 204.8;
    float APPS2 = analogRead(A1) / 204.8;
    float BRAKE1 = analogRead(A2) / 85.33;
    float BRAKE2 = analogRead(A3) / 85.33;
    float VBATT = analogRead(A4) / 64.85;
    int DISP_POT = analogRead(A5);
    int DISP_BUT = analogRead(A6);

    bool RTD = digitalRead(0);
    bool TSA = digitalRead(1);

    // Perform checks and calculations
    float appsErr = abs((APPS2 * 1.515) - APPS1) * 20.0;
    if(appsErr > 5) STATUS[ACC] = 1;
    else STATUS[ACC] = 0;

    if(min(BRAKE1, BRAKE2) < 0.3) STATUS[BRK] = 1;
    else STATUS[BRK] = 0;

    // Send signals
    if(TSA == 1) {
        TSAlastdown = millis();
        TSAreleased = true;
    }
    else if (millis() - TSAlastdown > 1000 and TSAreleased) {
        if(STATUS[DRV] == 2 or STATUS[DRV] == 0) STATUS[DRV] = 3; else STATUS[DRV] = 2;
        TSAreleased = false;
    }


    if(RTD == 1) {
        RTDlastdown = millis();
        RTDreleased = true;
    }
    else if (millis() - RTDlastdown > 1000 and RTDreleased) {
        Serial.print("RTD switch");
        if(STATUS[DRV] == 0) STATUS[DRV] = 2; else if (STATUS[DRV] == 2) STATUS[DRV] = 0;
        RTDreleased = false;
    }

    if(STATUS[DRV] == 3 or STATUS[DRV] == 1 or kill) { // off or faulted
        digitalWrite(2, LOW); // TSA
        digitalWrite(5, LOW); // APPS
    } else if (STATUS[DRV] == 2) { // if RTD
        digitalWrite(2, HIGH); // TSA
        digitalWrite(5, LOW); // APPS
    } else if (STATUS[DRV] == 0) { // if on
        digitalWrite(2, HIGH); // TSA
        digitalWrite(5, HIGH); // APPS

        CAN.beginExtendedPacket(0x00000CFF);  // Send inverter drive enable signal
        CAN.write(0x01);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.endPacket();
    }

    // Update Display
    if(lastMillis + 500 < millis()) {
        /*lastMillis = millis();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("ACC BRK CAN DRV ");
        lcd.setCursor(0, 1);
        lcd.print(getStatus());
         */
    }
    delay(100);
}

void sendMsg() {
    CAN.beginExtendedPacket(0x11FF);
    CAN.endPacket();
    Serial.print("Sent");
}


String getStatus() {
    String statusText = "";

    switch (STATUS[ACC]) {
        case -1: statusText += "--- "; break;
        case 0: statusText += "OK  "; break;
        case 1: statusText += "Err "; break;
    }

    switch (STATUS[BRK]) {
        case -1: statusText += "--- "; break;
        case 0: statusText += "OK  "; break;
        case 1: statusText += "Err "; break;
    }

    switch (STATUS[CANN]) {
        case -1: statusText += "--- "; break;
        case 0: statusText += "OK  "; break;
        case 1: statusText += "Err "; break;
    }

    switch (STATUS[DRV]) {
        case -1: statusText += "--- "; break;
        case 0: statusText += "On  "; break;
        case 1: statusText += "Err "; break;
        case 2: statusText += "rtd "; break;
        case 3: statusText += "off "; break;
    }
    return statusText;
}

float unpackFloat(const void *buf, int *i) {
    const auto *b = (const unsigned char *)buf;
    uint32_t value = 0;
    *i += 4;
    value = ((b[0] << 24) | (b[1] << 16) | (b[2] << 8) | b[3]);
    return *((float *) &value);
}


void readCan(int packetSize) {
    // ------------------------------------------------------------ GETTING CAN DATA ------------------------------------------------------------ //

    // received a packet
    Serial.print("Recv ");

    long id = CAN.packetId();
    Serial.print("id: ");
    Serial.print(id, HEX);

    Serial.print(" len: ");
    Serial.print(packetSize);

    // only print packet data for non-RTR packets
    Serial.print(" message: ");
    int k = 0;
    byte buffer[8];
    for(int j = 0; j < packetSize; j++) {
        byte part = CAN.read();
        if(k < 8) {
            buffer[k] = part;
            Serial.print(" ");
            Serial.print(part, HEX);
        }
        k++;
    }
    if(packetSize == 8) {
        int i = 0;
        float a = unpackFloat(&buffer, &i);
        float b = unpackFloat(&buffer, &i);
        Serial.print(" A: ");
        Serial.print(a);
        Serial.print(" B: ");
        Serial.print(b);
    }
}
/*
long id = CAN.packetId();
if (id == 0x200E) { // Handle inverter RPM packets

    byte data[8];
    int i = 0;
    while (CAN.available() && i < 8) {
        data[i] = CAN.read();
        i++;
    }
    // Convert 4 bytes to long
    long RPM = ((long)data[0] << 24) | ((long)data[1] << 16) | ((long)data[2] << 8) | (long)data[3];
    RPM /= 10;
    Serial.print("RPM: ");
    Serial.println(RPM);

} else { // Handle other packets
    Serial.print("id: ");
    Serial.print(id, HEX);

    if (CAN.packetRtr()) {
        Serial.print(" req len: ");
        Serial.println(CAN.packetDlc());
    } else {
        Serial.print(" len: ");
        Serial.println(packetSize);

        // only print packet data for non-RTR packets
        while (CAN.available()) {
            Serial.print(CAN.read(), HEX);
            Serial.print("");
        }
        Serial.println();
    }

        // ------------------------------------------------------------ SENDING CAN DATA ------------------------------------------------------------ //

    if(millis() > lastTime + sendDelay) {
        lastTime = millis();
        CAN.beginExtendedPacket(0x00000CFF);
        CAN.write(0x01);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.write(0xFF);
        CAN.endPacket();
    }
 */
