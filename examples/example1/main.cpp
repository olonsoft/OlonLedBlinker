#include <Arduino.h>
#include <LedBlinker.h>

Olon::LedBlinker* led1 = nullptr;

uint8_t step = 0;
uint8_t lastStep = UINT8_MAX;
uint32_t lastMillis = 0;

void setup() {
  led1 = new Olon::LedBlinker(LED_BUILTIN, false);
  led1->setPattern(Olon::LedBlinker::Pattern::WIFI_ERROR);
}

void loop() {
  led1->loop();

  if (millis() - lastMillis > 5000) {
    lastMillis = millis();
    if (++step > 9) step = 0;
  }

  if (step == lastStep) {
    return;
  }

  lastStep = step;

  switch (step) {
    case 0: led1->setPattern(Olon::LedBlinker::Pattern::MQTT_CONNECTING); break;
    case 1: led1->setPattern(Olon::LedBlinker::Pattern::MQTT_ERROR); break;
    case 2: led1->setPattern(Olon::LedBlinker::Pattern::SPEED_FAST); break;
    case 3: led1->setPattern(Olon::LedBlinker::Pattern::SPEED_MEDIUM); break;
    case 4: led1->setPattern(Olon::LedBlinker::Pattern::SPEED_SLOW); break;
    case 5: led1->setPattern(Olon::LedBlinker::Pattern::SPEED_VERY_SLOW); break;
    case 6: led1->setPattern(Olon::LedBlinker::Pattern::WIFI_CONNECTING); break;
    case 7: led1->setPattern(Olon::LedBlinker::Pattern::WIFI_ERROR); break;
    case 8: led1->setPattern(0x01, 2, 50); break;  // led flash very fast
    case 9: led1->setPattern(0b1, 1, 100, 1); break;
    default: break;
  }
};