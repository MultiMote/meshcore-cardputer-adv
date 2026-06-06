#pragma once

#include <DataStore.h>

#define CUSTOM_DATA_DIR "/meshcore_custom"
#define CUSTOM_PREFS_FILE (CUSTOM_DATA_DIR "/prefs")

//** Custom node preferences, persisted to file */
struct __attribute__((packed)) CustomNodePrefs {
  uint8_t power_save;
  float battery_correction;
};

class CardputerDataStore : public DataStore {
private:
  FILESYSTEM *_fs;

public:
  CardputerDataStore(FILESYSTEM &fs, mesh::RTCClock &clock) : DataStore(fs, clock), _fs(&fs) {}

  void begin();
  void loadCustomPrefs(CustomNodePrefs &prefs);
  void saveCustomPrefs(const CustomNodePrefs &prefs);
};
