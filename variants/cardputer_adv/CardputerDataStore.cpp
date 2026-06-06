#include "CardputerDataStore.h"

void CardputerDataStore::begin() {
  _fs->mkdir(CUSTOM_DATA_DIR);
}

void CardputerDataStore::loadCustomPrefs(CustomNodePrefs &prefs) {
  if (!_fs->exists(CUSTOM_PREFS_FILE)) {
    return;
  }

  File file = _fs->open(CUSTOM_PREFS_FILE, FILE_READ);

  // Read file if it match struct size, otherwise use default values
  if (file.size() <= sizeof(prefs)) {
    file.read((uint8_t *)&prefs, sizeof(prefs));
  }

  file.close();
}

void CardputerDataStore::saveCustomPrefs(const CustomNodePrefs &prefs) {
  File file = _fs->open(CUSTOM_PREFS_FILE, FILE_WRITE);

  if (!file) {
    return;
  }

  file.write((uint8_t *)&prefs, sizeof(prefs));
  file.close();
}
