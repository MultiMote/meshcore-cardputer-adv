#pragma once

#include "globals.h"

#include <stdint.h>
#include <helpers/ContactInfo.h>


enum class SoundType {
  NewMessage,
  Keyboard,
  DiscoveryResult,
  MessageAck,
};

enum MessageSendState {
  MESSAGE_SENDING,
  MESSAGE_DELIVERED,
  MESSAGE_FAILED,
};

//** Used to store last sent direct message for resending */
struct LastSentMessage {
  ContactInfo recipient;
  uint32_t timestamp = 0;
  uint8_t attempt = 0;
  uint8_t total_attempts = 0;
  uint32_t expected_ack;
  char text[MAX_MESSAGE_LENGTH + 1] = { 0 };
  MessageSendState state = MESSAGE_SENDING;
  bool need_direct = false;
};
