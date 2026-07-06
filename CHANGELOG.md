# 2026.4.0

* Add versioning
* **Cardputer hardware abstraction rework**
  - Remove M5Unified, M5GFX, and M5Cardputer
  - Replace M5GFX with "LovyanGFX"
  - Replace M5Cardputer with "Adafruit TCA8418"
* **[BREAKING] Keyboard layout rework**
  - Support keyboard layers based on key modifiers (`Fn`, `OPT`, `Ctrl`, `Alt`, `Shift`)
  - Allow overriding the base layout
  - Layout file format changed (check the Wiki for details)
  - Improve chat page `Fn` + `Arrows` logic (an empty input line does not require pressing `Fn`; the logic is reversed when typing)
* **Power consumption reduced**
  - Sleep mode (power saving): from ~52mA to ~22mA
* Remove G0 button handling
* Add more Unicode blocks to font
* Move shutdown and reset to the tools menu
* Move node name to the settings menu and make it editable
* Move status icons to the left side
* Add unread icon
* Fix chat input line truncated at the wrong side
