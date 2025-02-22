#include "C12832.h"
#include "QEI.h"


C12832 lcd(D11, D13, D12, D7, D10);
QEI leftWheel(PC_4, PB_1, NC, 624, QEI::X4_ENCODING);     // pinName index?
QEI rightWheel(PC_2, PC_5, NC, 624, QEI::X4_ENCODING);    
// X4 - looks at the state every time a rising or falling edge occurs on channel A or channel B

int main()
{
    while(1)
    {
        wait(0.1);
        lcd.cls();
        lcd.locate(20,0);
        lcd.printf("Left wheel: %i\n", leftWheel.getPulses());


        lcd.locate(20,20);
        lcd.printf("Right wheel: %i\n", rightWheel.getPulses());
    }
}



// Start on encoder code ^

// Start on square code 

#include "mbed.h"

DigitalOut enable(PC_3);

PwmOut PWM1(PA_15);     // left
DigitalOut bipo1(PA_13);
DigitalOut d1(PA_14);

PwmOut PWM2(PB_7);      // right
DigitalOut bipo2(PC_11)    
DigitalOut d2(PC_10);      

void stopMotors(){    
    d1.write(1); // forward direction
    d2.write(1);

    PWM1.period(1.0f);
    PWM1.write(0.0f);
   
    PMW2.period(1.0f);
    PWM2.write(0.0f);
}


void moveForward(){
    d1.write(1); // forward direction
    d2.write(1);

    PWM1.period(19.0f); // period
    PWM1.write(0.8f)    // duty cycle

    PWM2.period(19.0f);
    PWM2.write(0.8f)

    stopMotors();
}


void turnLeft()
{
    d1.write(0); // forward direction
    d2.write(1);

    PWM1.period(19.0f);
    PWM1.write(0.4f);
    
    PWM2.period(19.0f);
    PWM2.write(0.8f);

    stopMotors();
}


void turnRight(){
    d1.write(1); // forward direction
    d2.write(0);

    PWM1.period(19.0f);
    PWM1.write(0.8f);

    PWM2.period(19.0f);
    PWM2.write(0.4f);

    stopMotors();
}




void uTurn()
{
    d1.write(0);
    d2.write(1);

    PWM1.period(19.0f);
    PWM1.write(0.8f);

    PWM2.period(19.0f);
    PWM2.write(0.8f);

    stopMotors();
}


int main()
{
    enable.write(1);
    bipo1.write(0);     // unipolar
    bipo2.write(0);


    stopMotors();


    for (int i = 0; i < 4; i++)
    {
        moveForward();
        wait(0.8);
        turnLeft();
        wait(0.4);
    }

    uTurn();
    wait(0.8);

    for (int x = 0; x < 4; x++)
    {
        moveForward();
        wait(0.8);
        turnRight();
        wait(0.4);
    }

    stopMotors();
}

