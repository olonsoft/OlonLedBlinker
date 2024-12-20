#pragma once
#include <Arduino.h>

#define OLON_LED_BLINKER_VERSION "1.0.0"
namespace Olon {
class LedBlinker {
 public:
  // Some general predefined 20 bit patterns
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

  LedBlinker(uint8_t led_pin, bool active_state) : _led_pin{led_pin}, _active_state{active_state} {
    pinMode(_led_pin, OUTPUT);
    digitalWrite(_led_pin, !_active_state);  // turn off led
  }

  void setPattern(uint32_t pattern, uint8_t pattern_length = 20, uint32_t msecs_per_bit = 100,
                  uint8_t repetitions = 0) {
    if (_locked)
      return;
    if (pattern_length > 32)
      return;
    if (_pattern != pattern || _pattern_length != pattern_length ||
        _msecs_per_bit != msecs_per_bit || repetitions > 0 || _pattern_status == finished) {
      // start new pattern
      _pattern        = pattern;
      _pattern_length = pattern_length;
      _msecs_per_bit  = msecs_per_bit;
      _repetitions    = repetitions;
      _pattern_status = startNew;
    }
  }

  void setOn() {
    if (_locked)
      return;
    if (_led_state == LOW) {
      _led_state = HIGH;
      digitalWrite(_led_pin, _active_state);
    }
    _pattern_status = finished;
  }

  void setOff() {
    if (_locked)
      return;
    if (_led_state == HIGH) {
      _led_state = LOW;
      digitalWrite(_led_pin, !_active_state);
    }
    _pattern_status = finished;
  }

  bool busy() {
    return (_pattern_status != finished);
  }

  void lock(bool locked) {
    _locked = locked;
  }

  void loop() {
    static bool     wait_for_next_bit  = false;
    static uint32_t last_time          = 0;
    static uint8_t  current_bit        = 0;
    static uint8_t  current_repetition = 0;

    if (_pattern_status == startNew) {
      _pattern_status    = processing;
      wait_for_next_bit  = false;
      last_time          = 0;
      current_bit        = 0;
      current_repetition = 0;
    } else if (_pattern_status == finished) {
      return;
    }

    if (!wait_for_next_bit) {
      // bool bit_state = (((_pattern) & (1ULL << _current_bit)) != 0);
      bool bit_state = bitRead(_pattern, current_bit);
      if (bit_state != _led_state) {
        _led_state = bit_state;
        digitalWrite(_led_pin, _led_state ? _active_state : !_active_state);
      }
      last_time         = millis();
      wait_for_next_bit = true;
    } else {  // waiting msecs for next bit
      if (millis() - last_time > _msecs_per_bit) {
        // ready to process the next bit
        current_bit++;
        wait_for_next_bit = false;
        // if finished the pattern
        if (current_bit >= _pattern_length) {
          current_bit = 0;

          if (_repetitions > 0) {
            if (++current_repetition >= _repetitions) {
              _pattern_status = finished;
              setOff();
            }
          }
        }
      }
    }
  }

 private:
  uint8_t  _led_pin        = LED_BUILTIN;
  bool     _active_state   = LOW;
  uint32_t _pattern        = 0b0;    // 0 = all off
  uint8_t  _pattern_length = 20;     // bits of pattern
  uint32_t _msecs_per_bit  = 100;    // how long a bit will be on or off
  uint8_t  _repetitions    = 0;      // 0 = repeat forever
  bool     _locked         = false;  // if locked it won't accept changes to patterns
  enum patten_status_t {
    startNew,
    processing,
    finished
  };
  patten_status_t _pattern_status    = finished;
  bool            _start_new_pattern = true;  //
  bool            _led_state         = 0;     // current led state (On / Off)
};

}; // namespace olon