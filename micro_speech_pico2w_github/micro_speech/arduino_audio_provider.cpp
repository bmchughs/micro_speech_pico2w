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

#if defined(ARDUINO_ARCH_RP2040)
#include <I2S.h>
#endif

#include <algorithm>

#include "audio_provider.h"
#include "micro_features_micro_model_settings.h"
#include "tensorflow/lite/micro/micro_log.h"

namespace {

#if defined(ARDUINO_ARCH_RP2040)

// Hackerbox118 I2S microphone wiring, matching ../echo/echo.ino:
//   SD  -> GP0
//   SCK -> GP1
//   WS  -> GP2
constexpr pin_size_t kI2sDataPin = 0;
constexpr pin_size_t kI2sBitClockPin = 1;
constexpr int kMicGain = 4;

I2S i2s(INPUT);
bool g_is_audio_initialized = false;

// Keep two seconds of 16 kHz PCM, enough for the micro_speech feature window
// while staying modest on Pico 2W RAM.
constexpr int kAudioCaptureBufferSize = kAudioSampleFrequency * 2;
int16_t g_audio_capture_buffer[kAudioCaptureBufferSize];
int16_t g_audio_output_buffer[kMaxAudioSampleSize];
volatile int32_t g_latest_audio_timestamp = 0;
uint32_t g_total_samples_captured = 0;

void CaptureAvailableSamples() {
  int16_t left = 0;
  int16_t right = 0;

  while (i2s.available() > 0) {
    if (!i2s.read16(&left, &right)) {
      break;
    }

    const int capture_index = g_total_samples_captured % kAudioCaptureBufferSize;
    int32_t amplified_sample = static_cast<int32_t>(right) * kMicGain;
    amplified_sample = std::max<int32_t>(-32768, std::min<int32_t>(32767, amplified_sample));
    g_audio_capture_buffer[capture_index] = static_cast<int16_t>(amplified_sample);
    ++g_total_samples_captured;
  }

  g_latest_audio_timestamp =
      (g_total_samples_captured * 1000) / kAudioSampleFrequency;
}

#endif  // defined(ARDUINO_ARCH_RP2040)

}  // namespace

TfLiteStatus InitAudioRecording() {
#if defined(ARDUINO_ARCH_RP2040)
  if (g_is_audio_initialized) {
    return kTfLiteOk;
  }

  i2s.setDATA(kI2sDataPin);
  i2s.setBCLK(kI2sBitClockPin);
  i2s.setBitsPerSample(16);
  i2s.setFrequency(kAudioSampleFrequency);

  if (!i2s.begin()) {
    MicroPrintf("I2S microphone failed to start");
    return kTfLiteError;
  }

  g_is_audio_initialized = true;

  while (g_latest_audio_timestamp < kFeatureSliceDurationMs) {
    CaptureAvailableSamples();
  }

  return kTfLiteOk;
#else
  MicroPrintf("Audio recording is not implemented for this board");
  return kTfLiteError;
#endif
}

TfLiteStatus GetAudioSamples(int start_ms, int duration_ms,
                             int* audio_samples_size, int16_t** audio_samples) {
#if defined(ARDUINO_ARCH_RP2040)
  CaptureAvailableSamples();

  const int start_offset = start_ms * (kAudioSampleFrequency / 1000);
  const int duration_sample_count =
      duration_ms * (kAudioSampleFrequency / 1000);

  if (duration_sample_count > kMaxAudioSampleSize) {
    return kTfLiteError;
  }

  for (int i = 0; i < duration_sample_count; ++i) {
    const int sample_index = start_offset + i;
    const int32_t oldest_available_sample =
        (g_total_samples_captured > kAudioCaptureBufferSize)
            ? static_cast<int32_t>(g_total_samples_captured -
                                   kAudioCaptureBufferSize)
            : 0;
    if (sample_index < oldest_available_sample) {
      g_audio_output_buffer[i] = 0;
      continue;
    }
    const int capture_index = sample_index % kAudioCaptureBufferSize;
    g_audio_output_buffer[i] = g_audio_capture_buffer[capture_index];
  }

  *audio_samples_size = duration_sample_count;
  *audio_samples = g_audio_output_buffer;
  return kTfLiteOk;
#else
  return kTfLiteError;
#endif
}

int32_t LatestAudioTimestamp() {
#if defined(ARDUINO_ARCH_RP2040)
  CaptureAvailableSamples();
  return g_latest_audio_timestamp;
#else
  return 0;
#endif
}
