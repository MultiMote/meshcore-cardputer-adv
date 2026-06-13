#pragma once

#include <DataStore.h>
#include "globals.h"
#include "RingBuffer.h"

//** Custom node preferences, persisted to file */
struct __attribute__((packed)) CustomNodePrefs {
  uint8_t power_save;
  float battery_correction;
};

struct HistoryMessage {
  char text[MAX_MESSAGE_LENGTH];
  bool out;
};

using ChatHistory = RingBuffer<HistoryMessage, CHAT_HISTORY_RINGBUF_SIZE>;

class CardputerDataStore : public DataStore {
private:
  FILESYSTEM *_fs;

public:
  CardputerDataStore(FILESYSTEM &fs, mesh::RTCClock &clock) : DataStore(fs, clock), _fs(&fs) {}

  void begin();
  void loadCustomPrefs(CustomNodePrefs &prefs);
  void saveCustomPrefs(const CustomNodePrefs &prefs);
  void storeMessage(const uint8_t pkey[PUB_KEY_SIZE], const char *text, bool is_sent, bool is_channel);
  void loadMessages(const uint8_t pkey[PUB_KEY_SIZE], bool is_channel, ChatHistory &history);
};
