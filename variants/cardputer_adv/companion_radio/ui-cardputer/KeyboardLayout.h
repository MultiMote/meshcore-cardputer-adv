#pragma once

#include "CardputerDataStore.h"
#include "globals.h"

class KeyboardLayout {
  struct Replacement {
    // Key to press
    char key;
    // Max to 3 byte utf-8
    char repl[MAX_KB_LAYOUT_REPLACEMENT_BYTES + 1];
  };

  char secondary_code[3] = { 0 };
  Replacement replacements[MAX_KB_LAYOUT_ITEMS];
  bool is_secondary = false;

public:
  KeyboardLayout() : replacements{ { '\0', { '\0' } } } {}
  void begin(FS &fs);

  inline bool hasSecondary() const { return secondary_code[0] != 0; }
  inline bool isSecondary() const { return is_secondary; }
  inline const char *getPrimaryCode() const { return "en"; }
  inline const char *getSecondaryCode() const { return secondary_code; }
  inline const char *getCurrentCode() const {
    return is_secondary && hasSecondary() ? secondary_code : getPrimaryCode();
  }
  const char *findReplacement(const char c) const;
  bool switchLayout();
};
