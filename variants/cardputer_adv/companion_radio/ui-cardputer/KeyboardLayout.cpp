#include "KeyboardLayout.h"

#include <MeshCore.h>
#include <cstring>

void KeyboardLayout::begin(FS &fs) {
  if (!fs.exists(KB_LAYOUT_FILE)) {
    return;
  }

  File layout_file = fs.open(KB_LAYOUT_FILE, FILE_READ, false);

  if (!layout_file) {
    return;
  }

  int count = 0;

  if (layout_file.available()) {
    String line = layout_file.readStringUntil('\n');
    if (line.endsWith("\r")) {
      line.remove(line.length() - 1);
    }

    strncpy(secondary_code, line.c_str(), 2);
    secondary_code[2] = '\0';
  }

  while (layout_file.available() && count < MAX_KB_LAYOUT_ITEMS) {
    String line = layout_file.readStringUntil('\n');
    if (line.endsWith("\r")) {
      line.remove(line.length() - 1);
    }

    int line_len = line.length();

    if (line_len > 2 && line_len < 2 + MAX_KB_LAYOUT_REPLACEMENT_BYTES + 1) {
      if (line[1] == '=') {
        replacements[count].key = line[0];

        strncpy(replacements[count].repl, line.c_str() + 2, MAX_KB_LAYOUT_REPLACEMENT_BYTES);
        replacements[count].repl[MAX_KB_LAYOUT_REPLACEMENT_BYTES] = '\0';

        count++;
      }
    }
  }

  if (count < MAX_KB_LAYOUT_ITEMS) {
    replacements[count].key = '\0';
    replacements[count].repl[0] = '\0';
  }
}

const char *KeyboardLayout::findReplacement(const char c) const {
  if (!isSecondary()) {
    return nullptr;
  }

  for (int i = 0; i < MAX_KB_LAYOUT_ITEMS; i++) {
    if (!replacements[i].key) {
      break;
    }

    if (replacements[i].key == c) {
      return replacements[i].repl;
    }
  }
  return nullptr;
}

bool KeyboardLayout::switchLayout() {
  if (hasSecondary()) {
    is_secondary = !is_secondary;
    return true;
  }
  return false;
}
