/*
 * ESP32-OS Hardware Abstraction Layer Header
 * Hardware interface and peripheral management
 */

#ifndef HAL_H
#define HAL_H

#include <Arduino.h>
#include "../config/config.h"

// GPIO pin definitions
#define HAL_LED_PIN LED_BUILTIN_PIN
#define HAL_BUTTON_PIN 0  // Boot button on ESP32-DevKitC

class HAL {
private:
    bool initialized;
    bool ledState;
    unsigned long lastButtonPress;
    
    // Hardware monitoring
    float temperature;
    uint32_t vccVoltage;
    
    void initGPIO();
    void initADC();
    void initPWM();
    
public:
    HAL();
    ~HAL();
    
    bool init();
    void shutdown();
    
    // LED control
    void setLED(bool state);
    bool getLED() const { return ledState; }
    void toggleLED();
    void blinkLED(uint16_t onTime, uint16_t offTime, uint8_t count = 1);
    
    // Button input
    bool isButtonPressed();
    bool wasButtonPressed(); // Debounced
    
    // Analog input
    uint16_t readAnalog(uint8_t pin);
    float readVoltage(uint8_t pin);
    
    // PWM output
    void setPWM(uint8_t pin, uint8_t channel, uint16_t frequency, uint8_t dutyCycle);
    void stopPWM(uint8_t channel);
    
    // System monitoring
    float getTemperature();
    uint32_t getVccVoltage();
    void updateSensors();
    
    // Power management
    void enterLightSleep(uint64_t sleepTimeUs);
    void enterDeepSleep(uint64_t sleepTimeUs);
    void wakeupFromSleep();
    
    // System control
    void enableWatchdog(uint32_t timeoutMs);
    void disableWatchdog();
    void feedWatchdog();
    
    // Hardware info
    void printHardwareInfo();
    bool isHardwareHealthy();
};

#endif // HAL_H
