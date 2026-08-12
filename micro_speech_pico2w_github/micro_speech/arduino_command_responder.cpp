/* Copyright 2022 The TensorFlow Authors. All Rights Reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
==============================================================================*/

#include <Arduino.h>

#include "command_responder.h"
#include "tensorflow/lite/micro/micro_log.h"

namespace {

#if defined(LEDG)
constexpr int kGreenLedPin = LEDG;
#else
constexpr int kGreenLedPin = LED_BUILTIN;
#endif

#if defined(LEDR)
constexpr int kRedLedPin = LEDR;
#else
// Set this to your red LED's GP pin if it is not defined by the board package.
// Use -1 to disable red output.
constexpr int kRedLedPin = -1;
#endif

#if defined(LEDG) || defined(LEDR)
constexpr bool kRgbLedActiveLow = true;
#else
constexpr bool kRgbLedActiveLow = false;
#endif

void WriteLed(int pin, bool on) {
  if (pin < 0) {
    return;
  }
  digitalWrite(pin, (on ^ kRgbLedActiveLow) ? HIGH : LOW);
}

void SetStatusLeds(bool red_on, bool green_on) {
  WriteLed(kRedLedPin, red_on);
  WriteLed(kGreenLedPin, green_on);
}

}  // namespace

void RespondToCommand(int32_t current_time, const char* found_command,
                      uint8_t score, bool is_new_command) {
  static bool is_initialized = false;
  if (!is_initialized) {
    if (kRedLedPin >= 0) {
      pinMode(kRedLedPin, OUTPUT);
    }
    if (kGreenLedPin >= 0) {
      pinMode(kGreenLedPin, OUTPUT);
    }
    SetStatusLeds(false, false);
    is_initialized = true;
  }

  static int32_t last_command_time = 0;
  static int count = 0;
  static char active_command = 0;

  if (is_new_command) {
    MicroPrintf("Heard %s (%d) @%dms", found_command, score, current_time);

    if (found_command[0] == 'y') {
      SetStatusLeds(false, true);
      active_command = 'y';
      last_command_time = current_time;
    } else if (found_command[0] == 'n') {
      SetStatusLeds(true, false);
      active_command = 'n';
      last_command_time = current_time;
    }
  }

  if (last_command_time != 0 &&
      last_command_time < (current_time - 3000)) {
    last_command_time = 0;
    active_command = 0;
  }

  if (active_command == 0) {
    ++count;
    SetStatusLeds(false, (count & 1) != 0);
  }
}
