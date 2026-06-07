#pragma once

#include <DataStore.h>

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
  void storeMessage(const uint8_t pkey[PUB_KEY_SIZE], const char *text, bool is_sent, bool is_channel);
};
