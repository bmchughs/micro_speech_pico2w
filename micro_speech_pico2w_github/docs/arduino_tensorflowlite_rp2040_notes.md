# Arduino_TensorFlowLite RP2040/RP2350 Notes

The Arduino_TensorFlowLite library version used for this port was originally
oriented around Arduino Nano 33 BLE Sense support. On Pico 2 W, the library can
try to compile Nano-specific peripheral glue and fail with an unsupported board
error.

These are the compatibility changes used locally to compile this sketch for:

```text
rp2040:rp2040:rpipico2w
```

## Library Files Touched

Installed library root:

```text
~/Documents/Arduino/libraries/Arduino_TensorFlowLite/src
```

Changes applied locally:

- `TensorFlowLite.h`
  - Removed the unconditional include of `peripherals/peripherals.h`.
- `peripherals/button_arduino.cpp`
- `peripherals/i2c_arduino.cpp`
- `peripherals/i2s_nrf52840.cpp`
- `peripherals/led_arduino.cpp`
- `peripherals/wm8960.cpp`
- `peripherals/ws_wm8960_audio_hat_nrf52840.cpp`
  - Wrapped Nano-only peripheral implementations in:

    ```cpp
    #if defined(ARDUINO_ARDUINO_NANO33BLE)
    // Nano-specific implementation
    #endif
    ```

- `third_party/flatbuffers/include/flatbuffers/base.h`
  - Allowed `<stdint.h>` and `<cstdint>` to be included under Arduino builds.
- `tensorflow/lite/micro/system_setup.cpp`
  - Allowed `ARDUINO_ARCH_RP2040`.
  - Replaced Nano-specific ring-buffer logging with a small fixed buffer.

## Why This Is Needed

The Pico 2 W uses the Arduino-Pico core, not the Arduino Nano 33 BLE core. The
sketch supplies its own board-specific audio implementation in
`micro_speech/arduino_audio_provider.cpp`, so the Nano BLE peripheral wrappers
are not needed for this build.

## Confirming The Patch

After compiling, the log should include the sketch files:

```text
sketch/arduino_audio_provider.cpp.o
sketch/recognize_commands.cpp.o
```

And should end with a normal size report similar to:

```text
Sketch uses 399724 bytes (9%) of program storage space.
Global variables use 150436 bytes (28%) of dynamic memory.
```

Warnings about deprecated `std::is_pod` in TensorFlow internals are noisy but
not fatal.

## Runtime Confirmation

After upload, Serial Monitor should print:

```text
Recognizer tuning: yes>60 near+10 no>150 startup>1000ms
Initialization complete
```

That confirms the tuned recognizer code is running on the device.
