#pragma once

#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "globals.h"

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

  int contact_list_idx = 0;
  int channel_list_idx = 0;
  int contact_open_idx = -1;
  int channel_open_idx = -1;

  int getChannelCount();
  void sendChatMessage();
  void chatInputRemoveLastChar();

  void renderStatusIcons();

  void renderFirstPage();
  void renderChannelsPage();
  void renderContactsPage();
  void renderChatPage();
  void renderRecentPage();
  void renderStatsPage();
  void renderGpsPage();
  void renderShutdownPage();

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs, KeyboardLayout *keyboard_layout)
      : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs), _custom_prefs(custom_prefs),
        _keyboard_layout(keyboard_layout) {
    chat_text_box.reserve(MAX_MESSAGE_LENGTH);
  }
  void messageRepeatsRecv(uint16_t count);
  void poll() override;
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
};
