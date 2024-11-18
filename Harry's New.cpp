//
// Created by Sussex Racing on 01/11/2024.
//
#include <Arduino.h>
#include <LiquidCrystal_I2C.h>

// set the LCD address to 0x27 for a 16 chars and 2 line display
LiquidCrystal_I2C lcd(0x27,16,2);

//add tsa and rtd buttons pins
const int tsaButtonPin = 2;
const int rtdButtonPin = 5;


//variable for reading the TSA and RTD button states
int tsaButtonState = 0;
int rtdButtonState = 0;


void setup() {
    //sets TSA and APPS buttons as inputs
    pinMode(tsaButtonPin, INPUT);
    pinMode(rtdButtonPin, INPUT);

    //---------LCD display-----------//
    // Set the cockpit display to the loading screen
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

    // -----------------------------Outputs states of buttons
    //if TSA is on then output "TSA on" to screen
    if(tsaButtonState == HIGH) {
        lcd.clear();
        lcd.print("TSA on");
        delay(500);
    }

    //if TSA is off then output "TSA off" to screen
    else {
        lcd.clear();
        lcd.print("TSA off");
        delay(500);
    }

    //if RTD is on then output "RTD on" to screen
    if(rtdButtonState == HIGH) {
        lcd.clear();
        lcd.print("RTD on");
        delay(500);
    }

    //if RTD is off then output "RTD off" to screen
    else {
        lcd.clear();
        lcd.print("RTD off");
        delay(500);
    }
    //--------------------------------------------------------------
}
