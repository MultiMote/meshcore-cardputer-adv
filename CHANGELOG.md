# 2026.7.2

* Remove messages preview screen
* Add "New message" screen that appears when a new message is received when screen is off or current page is main
* Move unsynced messages count to the stats page

# 2026.7.1

* Multiple direct message sending attempts
* Press `Ctrl+T` to break resending
* Press `Ctrl+Up` to insert last sent message into the text field
* Add arrows to font
* Fix screen not updated on new message

# 2026.7.0

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
* Do not pop up the message preview in the chat page
