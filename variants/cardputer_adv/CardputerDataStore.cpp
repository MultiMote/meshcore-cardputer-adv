#include "CardputerDataStore.h"

#include "globals.h"

static void buf_to_hex_fast(const unsigned char *src, size_t src_len, char *dst) {
  static const char hex_digits[] = "0123456789abcdef";

  for (size_t i = 0; i < src_len; i++) {
    dst[i * 2] = hex_digits[(src[i] >> 4) & 0x0F]; // High nibble
    dst[i * 2 + 1] = hex_digits[src[i] & 0x0F];    // Low nibble
  }
  dst[src_len * 2] = '\0';
}

void CardputerDataStore::begin() {
  _fs->mkdir(CUSTOM_DATA_DIR);
  _fs->mkdir(HISTORY_DIR);
  _fs->mkdir(HISTORY_DIR_CHANNEL);
  _fs->mkdir(HISTORY_DIR_DIRECT);
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

void CardputerDataStore::storeMessage(const uint8_t pkey[PUB_KEY_SIZE], const char *text, bool is_sent,
                                      bool is_channel) {
#if USE_SD_CARD
  char path[128];
  char msg_copy[MAX_MESSAGE_LENGTH + 1] = { 0 };
  char pkey_hex[PUB_KEY_SIZE * 2 + 1];
  uint8_t key_len = is_channel ? (PUB_KEY_SIZE / 2) : PUB_KEY_SIZE;

  buf_to_hex_fast(pkey, key_len, pkey_hex);

  int msg_len;
  for (msg_len = 0; text[msg_len] != '\0' && msg_len < MAX_MESSAGE_LENGTH; msg_len++) {
    if (text[msg_len] == '\n' || text[msg_len] == '\r') {
      msg_copy[msg_len] = ' '; // Swap newline with space
    } else {
      msg_copy[msg_len] = text[msg_len];
    }
  }

  msg_copy[msg_len] = '\0';

  snprintf(path, sizeof(path), "%s/%s", is_channel ? HISTORY_DIR_CHANNEL : HISTORY_DIR_DIRECT, pkey_hex);

  File file = _fs->open(path, FILE_APPEND, true);

  if (!file) {
    return;
  }

  file.print(is_sent ? ">> " : "<< ");
  file.println(msg_copy);
  file.close();
#endif
}

static void push_history_line(ChatHistory &history, char *buf) {
  int len = strlen(buf);

  if (len < 4 || (buf[0] != '>' && buf[0] != '<') || buf[0] != buf[1]) {
    return;
  }

  HistoryMessage *msg = history.push_ref();
  msg->out = buf[0] == '>';

  strncpy(msg->text, buf + 3, sizeof(msg->text));
  msg->text[sizeof(msg->text) - 1] = '\0';
}

void CardputerDataStore::loadMessages(const uint8_t pkey[PUB_KEY_SIZE], bool is_channel,
                                      ChatHistory &history) {
  history.clear();

#if USE_SD_CARD
  char path[128];
  char pkey_hex[PUB_KEY_SIZE * 2 + 1];
  char line_buf[256] = { 0 };
  uint8_t file_read_buf[1024];

  uint8_t key_len = is_channel ? (PUB_KEY_SIZE / 2) : PUB_KEY_SIZE;

  buf_to_hex_fast(pkey, key_len, pkey_hex);

  snprintf(path, sizeof(path), "%s/%s", is_channel ? HISTORY_DIR_CHANNEL : HISTORY_DIR_DIRECT, pkey_hex);

  if (!_fs->exists(path)) {
    return;
  }

  File file = _fs->open(path, FILE_READ, false);

  if (!file) {
    return;
  }

  size_t line_idx = 0;

  while (file.available()) {
    int bytesRead = file.read(file_read_buf, sizeof(file_read_buf));

    for (int b = 0; b < bytesRead; b++) {
      char c = (char)file_read_buf[b];

      if (c == '\n' || line_idx >= sizeof(line_buf) - 1) {
        line_buf[line_idx] = '\0';
        line_idx = 0;
        push_history_line(history, line_buf);
      } else if (c != '\r') {
        line_buf[line_idx++] = c;
      }
    }
  }

  // Handle any remaining text if the file didn't end with a trailing LF
  if (line_idx > 0) {
    line_buf[line_idx] = '\0';
    push_history_line(history, line_buf);
  }

  file.close();
#endif
}
