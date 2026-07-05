# 2026.4.0

* Add versioning
* Cardputer hardware abstraction reworking
  - Remove M5Unified, M5GM5GFX, M5Cardputer
  - M5GM5GFX replaced with "LovyanGFX"
  - M5Cardputer replaced with "Adafruit TCA8418"
* [BREAKING] Keyboard layouts reworking
  - Support keyboard layers based on key modifiers (`Fn`, `OPT`, `Ctrl`, `Alt`, `Shift`)
  - Allow overriding base layout
  - Layout file format changed (check Wiki for details)
  - Improve chat page Fn+Arrows logic (empty input line does not require fn pressing, logic is reversed when typing)
* Power consumption is reduced
  - Sleep mode (power saving): from ~52mA to ~22mA
* Remove G0 button handling
* Add more unicode blocks to font
