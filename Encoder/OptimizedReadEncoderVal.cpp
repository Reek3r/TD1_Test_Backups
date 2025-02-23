#include "mbed.h"
#include "C12832.h"
#include "QEI.h"

// Constants
#define VDD 3.3f
#define SAMPLING_FREQUENCY 10.0f
#define PULSES_PER_REV 1024
#define DISPLAY_UPDATE_PERIOD 0.1f

/**
 * @brief Class to handle analog input from a potentiometer
 */
class Potentiometer {
protected:
    AnalogIn inputSignal;
    float vdd;
    float currentSampleNorm;
    float currentSampleVolts;

public:
    Potentiometer(PinName pin, float v) : inputSignal(pin), vdd(v) {
        currentSampleNorm = 0.0f;
        currentSampleVolts = 0.0f;
        sample(); // Initial sample
    }

    // Get voltage reading (0.0 to VDD)
    float amplitudeVolts() {
        return inputSignal.read() * vdd;
    }

    // Get normalized reading (0.0 to 1.0)
    float amplitudeNorm() {
        return inputSignal.read();
    }

    // Take a new sample
    virtual void sample() {
        currentSampleNorm = inputSignal.read();
        currentSampleVolts = currentSampleNorm * vdd;
    }

    // Getters for most recent samples
    float getCurrentSampleVolts() { return currentSampleVolts; }
    float getCurrentSampleNorm() { return currentSampleNorm; }
};

/**
 * @brief Extended potentiometer class with automatic sampling
 */
class SamplingPotentiometer : public Potentiometer {
private:
    Ticker sampler;
    float samplingPeriod;

public:
    SamplingPotentiometer(PinName pin, float v, float fs)
        : Potentiometer(pin, v) {
        samplingPeriod = 1.0f / fs;
        sampler.attach(callback(this, &SamplingPotentiometer::sample), samplingPeriod);
    }

    ~SamplingPotentiometer() {
        sampler.detach(); // Cleanup
    }
};

/**
 * @brief Class to manage motor control and feedback
 */
class MotorController {
private:
    PwmOut& motor;
    QEI& encoder;
    const char* name;
    float currentSpeed;

public:
    MotorController(PwmOut& pwm, QEI& enc, const char* motorName)
        : motor(pwm), encoder(enc), name(motorName) {
        currentSpeed = 0.0f;
        encoder.reset();
    }

    void setSpeed(float normalizedSpeed) {
        currentSpeed = 1.0f - normalizedSpeed; // Invert for correct direction
        motor.write(currentSpeed);
    }

    int getPulseCount() {
        return encoder.getPulses();
    }

    void resetEncoder() {
        encoder.reset();
    }

    const char* getName() {
        return name;
    }
};

/**
 * @brief Class to handle LCD display updates
 */
class DisplayManager {
private:
    C12832& lcd;

public:
    DisplayManager(C12832& display) : lcd(display) {}

    void updateDisplay(MotorController& leftMotor, MotorController& rightMotor,
                      float leftSpeed, float rightSpeed) {
        lcd.cls();
        
        // Display speeds
        lcd.locate(0, 0);
        lcd.printf("%f", leftSpeed);
        lcd.locate(60, 0);
        lcd.printf("%f", rightSpeed);

        // Display encoder counts
        lcd.locate(0, 15);
        lcd.printf("Left: %d", leftMotor.getPulseCount());
        lcd.locate(60, 15);
        lcd.printf("Right: %d", rightMotor.getPulseCount());
    }
};

int main() {
    // Initialize hardware
    C12832 lcd(D11, D13, D12, D7, D10);
    DisplayManager display(lcd);

    // Initialize motor control pins
    DigitalOut bipolar(PC_11);
    DigitalOut direction(PC_10);
    DigitalOut bipolar2(PA_13);
    DigitalOut direction2(PA_14);
    DigitalOut enable(PC_3);
    PwmOut motorL(PB_7);
    PwmOut motorR(PA_15);

    // Initialize encoders
    QEI leftEncoder(PB_3, PA_10, NC, PULSES_PER_REV, QEI::X2_ENCODING);
    QEI rightEncoder(PB_5, PB_4, NC, PULSES_PER_REV, QEI::X2_ENCODING);

    // Create motor controllers
    MotorController leftMotor(motorL, leftEncoder, "Left");
    MotorController rightMotor(motorR, rightEncoder, "Right");

    // Initialize potentiometers
    SamplingPotentiometer pot1(A0, VDD, SAMPLING_FREQUENCY);
    SamplingPotentiometer pot2(A1, VDD, SAMPLING_FREQUENCY);

    // Configure motor drivers
    direction.write(0);
    bipolar.write(0);
    direction2.write(0);
    bipolar2.write(0);
    enable.write(1);

    Timer displayTimer;
    displayTimer.start();

    while(1) {
        // Get potentiometer readings
        float leftSpeed = pot1.getCurrentSampleNorm();
        float rightSpeed = pot2.getCurrentSampleNorm();

        // Update motor speeds
        leftMotor.setSpeed(leftSpeed);
        rightMotor.setSpeed(rightSpeed);

        // Update display at fixed interval
        if (displayTimer.read() >= DISPLAY_UPDATE_PERIOD) {
            display.updateDisplay(leftMotor, rightMotor, leftSpeed, rightSpeed);
            leftMotor.resetEncoder();
            rightMotor.resetEncoder();
            displayTimer.reset();
        }
    }
}
