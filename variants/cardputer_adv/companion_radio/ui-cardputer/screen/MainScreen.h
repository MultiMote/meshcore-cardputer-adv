#pragma once

#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "globals.h"
#include "RingBuffer.h"
#include "UnreadCounter.h"

class MainScreen : public UIScreen {
  enum MainScreenPage {
    FIRST,
    CHANNELS,
    CONTACTS,
    CHAT,
    RECENT,
    STATS,
#if ENV_INCLUDE_GPS == 1
    GPS,
#endif
    SHUTDOWN,
    Count // keep as last
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  SensorManager *_sensors;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;
  KeyboardLayout *_keyboard_layout;

  uint8_t current_page = MainScreenPage::FIRST;
  bool shutdown_init = false;
  AdvertPath recent[UI_RECENT_LIST_SIZE];
  String chat_text_box;

  ChatHistory chat_history;
  int chat_history_offset = 0; // from bottom

  UnreadCounter unread;

  int contact_list_idx = 0;
  int channel_list_idx = 0;

  ChannelDetails current_channel;
  int current_channel_idx = -1;

  ContactInfo current_contact;
  int current_contact_idx = -1;

  uint32_t last_message_ack = 0;

  int getChannelCount();
  void sendChatMessage();
  void chatInputRemoveLastChar();

  void renderStatusIcons();

  int renderFirstPage();
  int renderChannelsPage();
  int renderContactsPage();
  int renderChatPage();
  int renderRecentPage();
  int renderStatsPage();
  int renderGpsPage();
  int renderShutdownPage();

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs, KeyboardLayout *keyboard_layout)
      : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs), _custom_prefs(custom_prefs),
        _keyboard_layout(keyboard_layout) {
    chat_text_box.reserve(MAX_MESSAGE_LENGTH);
  }
  void messageRepeatsRecv(uint16_t count);
  void onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  void onContactMessageRecv(const ContactInfo &contact, const char *text);
  void onAckRecv(uint32_t hash);
  void poll() override;
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
  void refreshSelectedContact();
};
