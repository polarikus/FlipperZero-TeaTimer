# Tea Timer

A pour-over tea brewing timer for the Flipper Zero. It keeps the count of pours for one
session, grows every next pour duration by a per-preset increment, and plays a single
sound/vibration when the pour is done.

Presets are plain text `.tea` files that you create on a computer and copy to the SD card.

## Requirements

- Flipper Zero with firmware that supports external apps (FAP).
- A microSD card inserted into the Flipper.
- `ufbt` (uFBT) for building from source: <https://github.com/flipperdevices/flipperzero-ufbt>

## Install

From the project directory:

```bash
ufbt                # build → dist/tea_timer.fap
ufbt launch         # build, upload over USB and run on the device
```

Or copy `dist/tea_timer.fap` to the SD card manually:

```
/ext/apps/Tools/tea_timer.fap
```

## Presets

Preset files live in a single directory on the SD card:

```
/ext/apps_data/tea_timer/
```

Create the directory and put one or more `.tea` files there. The app lists only files with
the `.tea` extension (the system file browser is used for selection).

If the directory is missing or contains no `.tea` files, the app shows `No presets found`
with a `Reload` button. If the SD card is not available, it shows `SD card not found` and
lets you retry or exit.

### `.tea` file format

Line-based `key: value` pairs, one pair per line. Comments start with `#`, empty lines are
ignored, unknown keys are ignored. UTF-8/ASCII only (no Cyrillic — the device fonts cannot
render it). Maximum file size is 1024 bytes.

| Key | Type | Allowed range | Example |
|---|---|---|---|
| `name` | text | 1–24 chars: Latin letters, digits, space, `_` | `Green Tea` |
| `leaf_g_per_100ml` | decimal, `.` as separator, exactly one digit after the dot | `0.1`–`50.0` | `5.0` |
| `first_pour_sec` | integer | `1`–`3600` | `20` |
| `increment_sec` | integer | `0`–`600` | `5` |
| `max_pours` | integer | `1`–`99` | `6` |

All five keys are mandatory and must appear exactly once.

Example `green_tea.tea`:

```tea
# Green Tea — 5 g per 100 ml, 6 pours
name: Green Tea
leaf_g_per_100ml: 5.0
first_pour_sec: 20
increment_sec: 5
max_pours: 6
```

With this preset the first pour lasts 20 s, the second 25 s, the third 30 s, and so on.
`max_pours` is only a recommendation: brewing beyond it is allowed and is shown as
`Pours 9 / 8` with an `OVER` badge.

If a file is invalid, the app shows a short dialog with the reason (for example
`Missing field: max_pours`, `Out of range: first_pour_sec`, `File is larger than 1 KB`) and
returns to the file browser. The other presets stay usable.

## Controls

| Screen | Button | Action |
|---|---|---|
| Main menu | `OK` / `Right` | Open the preset list |
| Main menu | `BACK` | Exit the app |
| File browser | `OK` / `Right` | Open the highlighted preset |
| File browser | `BACK` | Back to the main menu |
| Preset card | `OK` (center `Start`) | Start the pour immediately |
| Preset card | `BACK` | Back to the file browser |
| Brewing (`RUNNING`) | `OK` | Stop the pour; the pour counter does not change |
| Brewing (`RUNNING`) | `BACK` | Stop and go back to the preset card |
| `Time is up!` (`DONE`) | `OK` | Start the next pour |
| `Time is up!` (`DONE`) | `BACK` | Back to the preset card |

The display backlight is forced on while a pour is counting down and returns to the system
auto mode when the pour finishes or the app exits. There is no autostart: every next pour
is started manually.

## Build and tests

All commands are run from the project directory and use `ufbt` only:

```bash
ufbt                # build dist/tea_timer.fap
ufbt launch         # build, upload and run
ufbt format         # apply clang-format
ufbt lint           # static analysis
```

Unit tests run **on the device** from the app itself (minunit runner compiled into the FAP):

```bash
ufbt launch         # upload and run the app
ufbt cli            # in another terminal — CLI session
```

Inside the CLI:

```
tea_timer_test              # all suites
tea_timer_test tea_preset   # .tea parser only
tea_timer_test brew_math    # pour arithmetic only
```

Expected report:

```
test_reference_file()
...
20 tests, 101 assertions, 0 failures
Time: 248 ms
Leaked: 0
Status: PASSED
```

A successful run is also confirmed by the device notification (sound/vibration); a failure
plays the error notification. The CLI test command is registered only in builds with the
`TEA_TIMER_TESTS` define (set in `application.fam`), so a release build can omit it.

## Project layout

```
tea_timer/
├── application.fam        # FAP manifest
├── tea_timer.c            # entry point, resource ownership
├── scenes/                # SceneManager scenes
├── views/                 # custom brew view (Canvas)
├── lib/tea_logic/         # pure logic: .tea parser, pour arithmetic
├── unit_tests/            # on-device unit tests (minunit)
├── images/                # icons
└── specs/                 # feature specifications (Russian)
```
