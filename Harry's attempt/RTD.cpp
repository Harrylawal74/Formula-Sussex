void rtdStateCheck(rtdButtonState){
    // -----------------------------Outputs state of RTD
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