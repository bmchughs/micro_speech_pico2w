# Micro Speech on Raspberry Pi Pico 2 W

This is a Raspberry Pi Pico 2 W port of the TensorFlow Lite Micro `micro_speech`
example. It listens to an I2S microphone, runs a tiny keyword-spotting model on
device, and reports the words `yes` and `no` over Serial.

The sketch is tuned for a HackerBox 118-style Pico 2 W build:

- `yes` turns the green LED on for 3 seconds.
- `no` turns the red LED on for 3 seconds, if your board or wiring provides a
  red LED pin.
- While listening, the green LED blinks.
- Serial output is intentionally quiet and only prints startup status plus real
  command detections.

## What It Recognizes

The included model has four output labels:

- `silence`
- `unknown`
- `yes`
- `no`

Only `yes` and `no` are reported as commands. `silence` and `unknown` are used
internally to avoid reacting to background audio.

## Hardware

- Raspberry Pi Pico 2 W
- I2S microphone
- Arduino IDE 2.x
- Earle Philhower Arduino-Pico board package
- Arduino_TensorFlowLite library

Default I2S microphone wiring:

| Microphone signal | Pico 2 W pin |
| --- | --- |
| SD / DIN | GP0 |
| SCK / BCLK | GP1 |
| WS / LRCLK | GP2 |
| 3V3 | 3V3 |
| GND | GND |

The Arduino-Pico I2S implementation derives LRCLK/word-select from the BCLK pin,
so setting BCLK to `GP1` uses `GP2` for LRCLK.

## Install

1. Install Arduino IDE.
2. Add the Arduino-Pico Boards Manager URL:

   ```text
   https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
   ```

3. Install the `Raspberry Pi Pico/RP2040/RP2350` board package.
4. Install the `Arduino_TensorFlowLite` library.
5. Apply the RP2040/RP2350 compatibility notes in
   [docs/arduino_tensorflowlite_rp2040_notes.md](docs/arduino_tensorflowlite_rp2040_notes.md).
6. Open [micro_speech/micro_speech.ino](micro_speech/micro_speech.ino).
7. Select board `Raspberry Pi Pico 2W`.
8. Compile and upload.

Arduino CLI equivalent:

```sh
arduino-cli compile --fqbn rp2040:rp2040:rpipico2w micro_speech
arduino-cli upload --fqbn rp2040:rp2040:rpipico2w -p /dev/cu.usbmodemXXXX micro_speech
```

## Expected Serial Output

Open Serial Monitor at `115200` baud. On boot you should see:

```text
Recognizer tuning: yes>60 near+10 no>150 startup>1000ms
Initialization complete
```

After that, output should be sparse:

```text
Heard yes (86) @20752ms
Heard no (188) @30112ms
```

If you see repeated `Scores:` or `#Scores:` lines, debug logging was re-enabled
in `recognize_commands.cpp`.

## Tuning Notes

The stock micro speech model recognizes `no` more strongly than `yes` with this
microphone setup. This port uses:

- A shorter recognizer smoothing window: `500 ms`
- A lower `yes` command threshold: `60`
- A stricter `no` command threshold: `150`
- A 1 second startup ignore window to suppress boot-time false detections
- Quiet Serial output so real detections are easy to spot

If your microphone is too quiet or too hot, start with `kMicGain` in
`micro_speech/arduino_audio_provider.cpp`.

## Project Layout

```text
micro_speech/
  micro_speech.ino                  Main Arduino sketch
  arduino_audio_provider.cpp        Pico I2S microphone capture
  arduino_command_responder.cpp     LED and Serial command response
  recognize_commands.cpp/.h         Keyword smoothing and thresholds
  micro_features_model.cpp/.h       Embedded TensorFlow Lite model
  data/                             Example yes/no wav files
docs/
  arduino_tensorflowlite_rp2040_notes.md
```

## Credits

This project is based on the TensorFlow Lite Micro `micro_speech` example and
keeps the original TensorFlow source headers and Apache 2.0 license notices.

Pico 2 W support, I2S microphone capture, LED behavior, quieter Serial output,
and recognizer threshold tuning were adapted for this HackerBox 118 Pico 2 W
build.

## References

- [TensorFlow Lite Micro](https://github.com/tensorflow/tflite-micro)
- [TensorFlow Lite Micro Speech example](https://android.googlesource.com/platform/external/tensorflow/+/60c6e0991e4/tensorflow/lite/experimental/micro/examples/micro_speech/)
- [Arduino-Pico installation](https://github.com/earlephilhower/arduino-pico/blob/master/docs/install.rst)
- [Arduino-Pico I2S documentation](https://arduino-pico.readthedocs.io/en/latest/i2s.html)
- [Raspberry Pi Pico 2 W documentation](https://www.raspberrypi.com/documentation/microcontrollers/raspberry-pi-pico.html)

## License

The TensorFlow-derived source files retain their original Apache 2.0 notices.
See the individual file headers for details.
