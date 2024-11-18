//
// Created by Sussex Racing on 01/11/2024.
//
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>
#include TSA.cpp
#include RTD.cpp

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,16,2);

//add tsa and rtd buttons pins
const int tsaButtonPin = 2;
const int rtdButtonPin = 5;

//ini kill switch
bool kill = false;

//variable for reading the TSA and RTD button states
int tsaButtonState = 0;
int rtdButtonState = 0;


void setup() {
    //sets TSA and APPS buttons as inputs
    pinMode(tsaButtonPin, INPUT);
    pinMode(rtdButtonPin, INPUT);


    //on/off of RTD and TSA 
    bool RTD = digitalRead(0);
    bool TSA = digitalRead(1);

    //---------LCD display-----------//
    // Set the cockpit display to the loading screen
    Serial.begin(115200);
    lcd.init();  // initialize the lcd
    delay(500);
    lcd.backlight();
    lcd.setCursor(0, 0);
    Serial.print(" Sussex  Racing ");
    lcd.setCursor(0, 1);
    lcd.print("===== V0.1 =====");

    delay(500);
    lcd.clear();
    //------------------------------------------------
}

//continuously loops checking whether any values have changed then making corresponding adjustments
void loop(){
    //checks the state of the TSA and RTD buttons
    tsaButtonState = digitalRead(tsaButtonPin);
    rtdButtonState = digitalRead(rtdButtonPin);

    // -----------------------------Outputs state of TSA 
    tsaStateCheck(tsaButtonState);

    // -----------------------------Outputs state of RTD 
    rtdStateCheck(rtdButtonState);
}
