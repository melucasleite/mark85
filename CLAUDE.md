# mark85 — Coding Guidelines

Firmware for an Iron Man suit movement-sound system on the Arduino Nano ESP32
(ESP32-S3), built with PlatformIO. Four IMUs (one per limb) drive synthesized
electro-mechanical suit sounds on two I²S outputs — left speaker for left-limb
movement, right speaker for right-limb movement — alongside servos, RGB
lighting, and a status display. See `README.md` for the hardware inventory,
pin map, and wiring diagram.

## Commands

```sh
pio run                            # build firmware
pio run -t upload                  # flash over USB (DFU)
pio device monitor                 # serial monitor
pio test -e native                 # run host unit tests (no board needed)
pio test -e arduino_nano_esp32     # run on-target tests (board attached)
```

## Project structure

```
src/
  main.cpp           # setup()/loop() only — wiring and scheduling, no logic
  config.h           # ALL pin assignments and hardware constants
  drivers/           # one file pair (.h/.cpp) per physical peripheral
    imu_array.*      # 4x MPU-6050 (one per limb) behind the HW-617 mux
    display.*        # ST7789V2 LCD
    audio_out_left.*   # I2S0 -> left speaker (left arm/leg sounds)
    audio_out_right.*  # I2S1 -> right speaker (right arm/leg sounds)
    servos.*         # 4x servo (LEDC)
    rgb_leds.*       # 2x HW-159 (WS2812)
  domains/           # one file pair per business-logic domain
                     # (e.g. limb motion tracking, sound synthesis, UI)
test/
  native/            # host unit tests, one test_<domain> dir per domain
  embedded/          # on-target smoke tests (drivers)
```

- **One module per file pair.** Every driver and every business-logic domain
  gets its own `.h`/`.cpp`. No god files; `main.cpp` stays under ~100 lines.
- **Drivers vs. domains.** Drivers talk to hardware and expose a small API.
  Domains implement behavior and depend only on driver APIs — never on
  registers, buses, or pin numbers directly.
- **`config.h` is the single source of truth for hardware.** Every pin
  number, I2C address, mux channel, bus frequency, and timing constant lives
  there as a named `constexpr`. No magic pin numbers anywhere else. When the
  wiring changes, `config.h` and the README pin table change in the same
  commit.

## C++ conventions

- `constexpr` over `#define` for constants; `enum class` over plain enums.
- Fixed-width integer types (`uint8_t`, `int32_t`) for anything
  hardware-facing; plain `int` only for local arithmetic.
- No dynamic allocation after `setup()` — no `new`, no `String`, no growing
  containers in the loop path. Statically size buffers.
- Pass by reference or pointer-to-const; avoid copies of buffers.
- Naming: `snake_case` for files/functions/variables, `PascalCase` for types,
  `kPascalCase` for constexpr constants, `UPPER_SNAKE` reserved for macros.

## Firmware rules

- **Never block.** No `delay()` outside `setup()`. Use `millis()`-based
  scheduling or FreeRTOS tasks. Audio and LED writes are DMA/RMT-backed —
  keep them fed, never spin-wait on them.
- **Every driver has `begin()` that returns `bool`.** `setup()` checks every
  return value and reports failures on serial and the display or RGB LEDs;
  a missing peripheral must not crash or hang the rest of the system.
- **ISRs are minimal.** Set a flag or push to a queue, return. `IRAM_ATTR`
  on every ISR; anything shared with an ISR is `volatile` or behind a
  FreeRTOS primitive.
- **I2C discipline.** All MPU-6050 access goes through the mux driver — no
  raw `Wire` calls outside `drivers/`. Select the mux channel, do the
  transaction, and don't assume the channel afterward.
- **Tasks and cores.** If FreeRTOS tasks are introduced: audio on one core,
  everything else on the other; document task, priority, and stack size where
  the task is created. Protect shared state; no cross-task globals without a
  mutex or queue.
- **Fail loudly in development.** Serial logging at boot for every subsystem
  init (`[imu] ok`, `[audio] FAILED`, ...). Keep per-loop logging behind a
  compile-time flag so it can't starve the loop.

## Testing

Test as much as possible on the host, before anything is flashed. Two tiers,
both on PlatformIO's Unity test runner:

- **Host-native tests (`pio test -e native`) — the default, where coverage
  lives.** All `domains/` code must compile and run on the host: no
  `Arduino.h`, no driver includes, no hardware types. Domains consume plain
  data (IMU samples in, sound/servo/LED commands out) and receive time as a
  parameter instead of calling `millis()`, so every state machine is
  deterministic and testable with synthetic sensor data — including edge
  cases that are hard to produce physically (impacts, sensor dropout,
  simultaneous limb movement).
- **On-target smoke tests (`pio test -e arduino_nano_esp32`) — sparingly.**
  Short Unity tests that run on the board to verify real hardware: each mux
  channel answers WHO_AM_I, display init succeeds, I2S starts. These verify
  wiring and drivers, not logic.

Rules:

- New domain logic ships with host tests in the same commit.
- If a piece of domain code can't be tested on the host, that's a design
  smell — restructure it (extract the pure logic, inject the dependency)
  rather than skipping the test.
- Drivers stay thin precisely so untested code is minimal: a driver method
  should do I/O and nothing else worth testing.

## Workflow

- Code must compile clean: fix warnings, don't suppress them.
- `pio run` and `pio test -e native` must pass before every commit.
- Hardware-behavior claims in commit messages only when actually flashed and
  observed (say "builds" vs. "tested on device").
