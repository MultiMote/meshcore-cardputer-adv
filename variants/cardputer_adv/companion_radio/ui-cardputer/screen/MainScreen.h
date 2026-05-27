#pragma once

#include "../CardputerUITask.h"
#include "../icons.h"

#include <MyMesh.h>

#ifndef UI_RECENT_LIST_SIZE
  #define UI_RECENT_LIST_SIZE 4
#endif

#ifndef BATT_MIN_MILLIVOLTS
  #define BATT_MIN_MILLIVOLTS 3000
#endif
#ifndef BATT_MAX_MILLIVOLTS
  #define BATT_MAX_MILLIVOLTS 4200
#endif

#ifndef UI_CONTACT_LIST_SIZE
  #define UI_CONTACT_LIST_SIZE 13
#endif

#ifndef UI_CHANNEL_LIST_SIZE
  #define UI_CHANNEL_LIST_SIZE 13
#endif

#ifndef UI_TEXTBOX_MAX
  #define UI_TEXTBOX_MAX 150
#endif

class MainScreen : public UIScreen {
  enum MainScreenPage {
    FIRST,
    CHANNELS,
    CONTACTS,
    CHAT,
    RECENT,
    RADIO,
    BLUETOOTH,
    ADVERT,
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

  void renderBatteryIndicator(DisplayDriver &display, uint16_t batteryMilliVolts);

  void renderFirstPage();
  void renderChannelsPage();
  void renderContactsPage();
  void renderChatPage();
  void renderRecentPage();
  void renderRadioPage();
  void renderBluetoothPage();
  void renderAdvertPage();
  void renderGpsPage();
  void renderShutdownPage();

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs)
      : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs) {
    chat_text_box.reserve(UI_TEXTBOX_MAX);
  }

  void poll() override;
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
};
