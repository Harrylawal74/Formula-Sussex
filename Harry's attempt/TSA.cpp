
void tsaStateCheck(tsaButtonState){
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
}
