#include "mbed.h"
#include "C12832.h"
#include "QEI.h"


C12832 lcd(D11, D13, D12, D7, D10); 
QEI leftWheel(PC_4, PB_1, NC, 624, QEI::X4_ENCODING);  
QEI rightWheel(PC_2, PC_5, NC, 624, QEI::X4_ENCODING); 

const float sampling_time = 0.1; 
const float pulses_per_rev = 624.0; 

void displayEncoderData() {
    int leftPulses = leftWheel.getPulses();
    int rightPulses = rightWheel.getPulses();
    
    float leftRPM = (leftPulses / pulses_per_rev) * (60.0 / sampling_time);
    float rightRPM = (rightPulses / pulses_per_rev) * (60.0 / sampling_time);
    
    lcd.cls();
    lcd.locate(0, 20);
    lcd.printf("L RPM: %.2f  R RPM: %.2f", leftRPM, rightRPM);
    
    lcd.locate(60, 20);
    lcd.printf("L Pulses: %d  R Pulses: %d", leftPulses, rightPulses);
    
    leftWheel.reset(); 
    rightWheel.reset();
}


int main() {
    lcd.cls(); 

    while (1) {
        displayEncoderData(); 
        wait(sampling_time); 
    }
}
