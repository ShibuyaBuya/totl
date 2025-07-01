/*
 * ESP32-OS Hardware Abstraction Layer Implementation
 */

#include "hal.h"
#include <esp_system.h>
#include <esp_sleep.h>
#include <esp_task_wdt.h>
#include <driver/adc.h>

HAL::HAL() : initialized(false), ledState(false), lastButtonPress(0),
             temperature(0.0), vccVoltage(0) {
}

HAL::~HAL() {
    shutdown();
}

bool HAL::init() {
    if (initialized) {
        return true;
    }
    
    // Initialize GPIO
    initGPIO();
    
    // Initialize ADC
    initADC();
    
    // Initialize PWM
    initPWM();
    
    // Enable watchdog
    enableWatchdog(WATCHDOG_TIMEOUT_SECONDS * 1000);
    
    initialized = true;
    Serial.println("HAL: Hardware abstraction layer initialized");
    
    return true;
}

void HAL::shutdown() {
    if (!initialized) {
        return;
    }
    
    // Turn off LED
    setLED(false);
    
    // Disable watchdog
    disableWatchdog();
    
    initialized = false;
}

void HAL::initGPIO() {
    // Initialize LED pin
    pinMode(HAL_LED_PIN, OUTPUT);
    digitalWrite(HAL_LED_PIN, LOW);
    ledState = false;
    
    // Initialize button pin
    pinMode(HAL_BUTTON_PIN, INPUT_PULLUP);
}

void HAL::initADC() {
    // Configure ADC
    analogReadResolution(12); // 12-bit resolution (0-4095)
    analogSetAttenuation(ADC_11db); // Full range up to 3.3V
}

void HAL::initPWM() {
    // PWM channels will be configured as needed
}

void HAL::setLED(bool state) {
    if (!initialized) return;
    
    digitalWrite(HAL_LED_PIN, state ? HIGH : LOW);
    ledState = state;
}

void HAL::toggleLED() {
    if (!initialized) return;
    
    setLED(!ledState);
}

void HAL::blinkLED(uint16_t onTime, uint16_t offTime, uint8_t count) {
    if (!initialized) return;
    
    bool originalState = ledState;
    
    for (uint8_t i = 0; i < count; i++) {
        setLED(true);
        delay(onTime);
        setLED(false);
        delay(offTime);
    }
    
    setLED(originalState);
}

bool HAL::isButtonPressed() {
    if (!initialized) return false;
    
    return digitalRead(HAL_BUTTON_PIN) == LOW; // Active low
}

bool HAL::wasButtonPressed() {
    if (!initialized) return false;
    
    static bool lastState = false;
    bool currentState = isButtonPressed();
    
    // Simple debouncing
    if (currentState && !lastState) {
        unsigned long currentTime = millis();
        if (currentTime - lastButtonPress > 50) { // 50ms debounce
            lastButtonPress = currentTime;
            lastState = currentState;
            return true;
        }
    }
    
    lastState = currentState;
    return false;
}

uint16_t HAL::readAnalog(uint8_t pin) {
    if (!initialized) return 0;
    
    return analogRead(pin);
}

float HAL::readVoltage(uint8_t pin) {
    if (!initialized) return 0.0;
    
    uint16_t adcValue = readAnalog(pin);
    return (adcValue * 3.3) / 4095.0; // Convert to voltage
}

void HAL::setPWM(uint8_t pin, uint8_t channel, uint16_t frequency, uint8_t dutyCycle) {
    if (!initialized) return;
    
    // Configure PWM channel
    ledcSetup(channel, frequency, 8); // 8-bit resolution
    ledcAttachPin(pin, channel);
    
    // Set duty cycle (0-255 for 8-bit)
    uint16_t duty = (dutyCycle * 255) / 100;
    ledcWrite(channel, duty);
}

void HAL::stopPWM(uint8_t channel) {
    if (!initialized) return;
    
    ledcWrite(channel, 0);
    ledcDetachPin(channel);
}

float HAL::getTemperature() {
    return temperature;
}

uint32_t HAL::getVccVoltage() {
    return vccVoltage;
}

void HAL::updateSensors() {
    if (!initialized) return;
    
    // Read internal temperature (approximation)
    // ESP32 doesn't have a built-in temperature sensor
    // This is a simplified implementation
    temperature = 25.0 + (random(-5, 5)); // Simulate temperature reading
    
    // Read VCC voltage (approximation)
    vccVoltage = 3300; // 3.3V nominal
}

void HAL::enterLightSleep(uint64_t sleepTimeUs) {
    if (!initialized) return;
    
    Serial.println("HAL: Entering light sleep mode");
    Serial.flush();
    
    esp_sleep_enable_timer_wakeup(sleepTimeUs);
    esp_light_sleep_start();
    
    Serial.println("HAL: Woke up from light sleep");
}

void HAL::enterDeepSleep(uint64_t sleepTimeUs) {
    if (!initialized) return;
    
    Serial.println("HAL: Entering deep sleep mode");
    Serial.flush();
    
    esp_sleep_enable_timer_wakeup(sleepTimeUs);
    esp_deep_sleep_start();
    
    // This line will never be reached
}

void HAL::wakeupFromSleep() {
    Serial.println("HAL: System woke up from sleep");
    
    // Re-initialize hardware if needed
    if (!initialized) {
        init();
    }
}

void HAL::enableWatchdog(uint32_t timeoutMs) {
    esp_task_wdt_init(timeoutMs / 1000, true); // Panic on timeout
    esp_task_wdt_add(NULL); // Add current task to watchdog
}

void HAL::disableWatchdog() {
    esp_task_wdt_delete(NULL); // Remove current task
    esp_task_wdt_deinit();
}

void HAL::feedWatchdog() {
    esp_task_wdt_reset();
}

void HAL::printHardwareInfo() {
    Serial.println("Hardware Information:");
    Serial.println("====================");
    Serial.printf("Chip Model:      %s\n", ESP.getChipModel());
    Serial.printf("Chip Revision:   %d\n", ESP.getChipRevision());
    Serial.printf("CPU Cores:       %d\n", ESP.getChipCores());
    Serial.printf("CPU Frequency:   %d MHz\n", ESP.getCpuFreqMHz());
    Serial.printf("Flash Size:      %d bytes\n", ESP.getFlashChipSize());
    Serial.printf("Flash Speed:     %d Hz\n", ESP.getFlashChipSpeed());
    Serial.printf("PSRAM Size:      %d bytes\n", ESP.getPsramSize());
    Serial.printf("Free Heap:       %d bytes\n", ESP.getFreeHeap());
    Serial.printf("Min Free Heap:   %d bytes\n", ESP.getMinFreeHeap());
    
    // GPIO status
    Serial.printf("LED State:       %s\n", ledState ? "ON" : "OFF");
    Serial.printf("Button State:    %s\n", isButtonPressed() ? "PRESSED" : "RELEASED");
    
    // Sensor readings
    updateSensors();
    Serial.printf("Temperature:     %.1f°C\n", temperature);
    Serial.printf("VCC Voltage:     %d mV\n", vccVoltage);
}

bool HAL::isHardwareHealthy() {
    if (!initialized) return false;
    
    // Check various hardware health indicators
    uint32_t freeHeap = ESP.getFreeHeap();
    if (freeHeap < 10240) { // Less than 10KB free
        return false;
    }
    
    // Add more health checks as needed
    return true;
}
