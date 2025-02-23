#include "mbed.h"
#include "C12832.h"
#include "QEI.h"

// Configuration constants
#define VDD 3.3f
#define SAMPLING_FREQUENCY 10.0f
#define PULSES_PER_REV 1024
#define LCD_UPDATE_MS 100

// LCD position helper structure
struct Point { int x; int y; };
const Point LEFT_POT_POS = {0, 0};
const Point RIGHT_POT_POS = {60, 0};
const Point LEFT_ENC_POS = {0, 15};
const Point RIGHT_ENC_POS = {60, 15};

class Potentiometer {
private:
    AnalogIn inputSignal;
    const float vdd;
    volatile float currentSampleNorm;
    volatile float currentSampleVolts;

public:
    Potentiometer(PinName pin, float v) : 
        inputSignal(pin), vdd(v), 
        currentSampleNorm(0.0f), currentSampleVolts(0.0f) {}

    void sample() {
        currentSampleNorm = inputSignal.read();
        currentSampleVolts = currentSampleNorm * vdd;
    }

    float getCurrentSampleVolts() const { return currentSampleVolts; }
    float getCurrentSampleNorm() const { return currentSampleNorm; }
};

class SamplingPotentiometer : public Potentiometer {
private:
    Ticker sampler;
    float samplingPeriod;

public:
    SamplingPotentiometer(PinName pin, float v, float fs) : 
        Potentiometer(pin, v) {
        if(fs <= 0) fs = 1.0f;  // Basic input validation
        samplingPeriod = 1.0f/fs;
        sampler.attach(this, &SamplingPotentiometer::sample, samplingPeriod);
    }
};

// Hardware resources
C12832 lcd(D11, D13, D12, D7, D10);
DigitalOut enable(PC_3);
PwmOut motorL(PB_7);
PwmOut motorR(PA_15);

// Motor control pins
DigitalOut bipolar(PC_11);
DigitalOut direction(PC_10);
DigitalOut bipolar2(PA_13);
DigitalOut direction2(PA_14);

// Encoders
QEI leftEncoder(PB_3, PA_10, NC, PULSES_PER_REV, QEI::X2_ENCODING);
QEI rightEncoder(PB_5, PB_4, NC, PULSES_PER_REV, QEI::X2_ENCODING);

void updateDisplay(float leftPot, float rightPot, int leftCount, int rightCount) {
    lcd.cls();
    
    // Potentiometer values
    lcd.locate(LEFT_POT_POS.x, LEFT_POT_POS.y);
    lcd.printf("L:%.2f", leftPot);
    lcd.locate(RIGHT_POT_POS.x, RIGHT_POT_POS.y);
    lcd.printf("R:%.2f", rightPot);
    
    // Encoder counts
    lcd.locate(LEFT_ENC_POS.x, LEFT_ENC_POS.y);
    lcd.printf("L:%5d", leftCount);
    lcd.locate(RIGHT_ENC_POS.x, RIGHT_ENC_POS.y);
    lcd.printf("R:%5d", rightCount);
}

int main() {
    // Initialize motor control pins
    direction = 0;
    bipolar = 0;
    direction2 = 0;
    bipolar2 = 0;
    enable = 1;

    // Initialize peripherals
    SamplingPotentiometer pot1(A0, VDD, SAMPLING_FREQUENCY);
    SamplingPotentiometer pot2(A1, VDD, SAMPLING_FREQUENCY);
    leftEncoder.reset();
    rightEncoder.reset();

    while(1) {
        // Read sensor values
        float leftVal = pot1.getCurrentSampleNorm();
        float rightVal = pot2.getCurrentSampleNorm();
        
        // Update motors
        motorL = 1.0f - leftVal;
        motorR = 1.0f - rightVal;
        
        // Read encoders
        int leftCount = leftEncoder.getPulses();
        int rightCount = rightEncoder.getPulses();
        leftEncoder.reset();
        rightEncoder.reset();
        
        // Update display
        updateDisplay(leftVal, rightVal, leftCount, rightCount);
        
        // Timing control
        wait_ms(LCD_UPDATE_MS);
    }
}
