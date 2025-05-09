#pragma once
#include <Arduino.h>

#define OLON_LED_BLINKER_VERSION "1.1.1"
namespace Olon {
class LedBlinker {
 public:
  // Predefined 20-bit patterns for different states
  enum Pattern : uint32_t {
    SPEED_VERY_SLOW = 0b00000000001111111111,
    SPEED_SLOW      = 0b00000111110000011111,  // captive portal running
    SPEED_MEDIUM    = 0b00110011001100110011,
    SPEED_FAST      = 0b01010101010101010101,  // Error/Reset
    WIFI_CONNECTING = 0b00000000000000010001,
    MQTT_CONNECTING = 0b00000000000100010001,
    WIFI_ERROR      = 0b00000000111100010001,
    MQTT_ERROR      = 0b00001111000100010001
  };

  LedBlinker(uint8_t led_pin = LED_BUILTIN, bool active_state = HIGH) : _ledPin{led_pin}, _activeState{active_state} {
    pinMode(_ledPin, OUTPUT);
    digitalWrite(_ledPin, !_activeState);  // turn off LED
  }

  // Set a new blinking pattern
  void setPattern(uint32_t pattern, uint8_t patternLength = 20, uint32_t msecsPerBit = 100,
                  uint8_t repetitions = 0) {
    if (_locked || patternLength > 32 || patternLength == 0 || msecsPerBit == 0) {
      return;
    }
    if (_pattern != pattern || _patternLength != patternLength ||
        _msecsPerBit != msecsPerBit || repetitions > 0 || _patternStatus == Finished) {
      // start new pattern
      _pattern        = pattern;
      _patternLength = patternLength;
      _msecsPerBit  = msecsPerBit;
      _repetitions    = repetitions;
      _patternStatus = StartNew;
    }
  }

  void setOn() {
    if (_locked)
      return;
    if (_ledState == LOW) {
      _ledState = HIGH;
      digitalWrite(_ledPin, _activeState);
    }
    _patternStatus = Finished;
  }

  void setOff() {
    if (_locked) return;
    if (_ledState == HIGH) {
      _ledState = LOW;
      digitalWrite(_ledPin, !_activeState);
    }
    _patternStatus = Finished;
  }

  // Check if a pattern is currently active
  bool isBusy() {
    return (_patternStatus != Finished);
  }

  void lock(bool locked) {
    _locked = locked;
  }

  // Pause the current pattern
  void pause() {
    if (_patternStatus == Running) {
      _patternStatus = Paused;
    }
  }

  // Resume a paused pattern
  void resume() {
    if (_patternStatus == Paused) {
      _patternStatus = Running;
    }
  }

  void loop() {
    static uint32_t lastTime          = 0;
    static uint8_t  currentBit        = 0;
    static uint8_t  currentRepetition = 0;
    static bool     waitForNextBit  = false;

    if (_patternStatus == StartNew) {
      _patternStatus    = Running;
      waitForNextBit  = false;
      lastTime          = millis(); // was 0;
      currentBit        = 0;
      currentRepetition = 0;
    } else if (_patternStatus != Running) {
      return; // Skip if finished or paused
    }

    if (!waitForNextBit) {
      bool bit_state = bitRead(_pattern, currentBit);
      if (bit_state != _ledState) {
        _ledState = bit_state;
        digitalWrite(_ledPin, _ledState ? _activeState : !_activeState);
      }
      lastTime = millis();
      waitForNextBit = true;
    } else if (millis() - lastTime > _msecsPerBit) {
        // ready to process the next bit
        currentBit++;
        waitForNextBit = false;
        // if finished the pattern
        if (currentBit >= _patternLength) {
          currentBit = 0;
          if (_repetitions > 0 && ++currentRepetition >= _repetitions) {
            _patternStatus = Finished;
            setOff();
          }
        }
      }
  }

 private:
  uint8_t  _ledPin        = LED_BUILTIN;
  bool     _activeState   = LOW;
  uint32_t _pattern       = 0b0;    // 0 = all off
  uint8_t  _patternLength = 20;     // bits of pattern
  uint32_t _msecsPerBit   = 100;    // how long a bit will be on or off
  uint8_t  _repetitions   = 0;      // 0 = repeat forever
  bool     _locked        = false;  // if locked it won't accept changes to patterns
  bool     _ledState      = 0;     // current led state (On / Off)
  enum PatternStatus {
    StartNew,
    Running,
    Paused,
    Finished
  };
  PatternStatus _patternStatus = Finished;
};

}; // namespace olon