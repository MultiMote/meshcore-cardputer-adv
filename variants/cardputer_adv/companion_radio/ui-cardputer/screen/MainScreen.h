#pragma once

#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "RingBuffer.h"
#include "UnreadCounter.h"
#include "globals.h"

class MainScreen : public CardputerScreen {
public:
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
    Count
  };

private:
  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  SensorManager *_sensors;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;

  MainScreenPage current_page = MainScreen::FIRST;
  MainScreenPage page_to_return = MainScreen::FIRST;
  AdvertPath recent[UI_RECENT_LIST_SIZE];
  String last_sent_message;
  String chat_text_box;
  String contact_search_box;

  ChatHistory chat_history;
  int chat_history_offset = 0; // from bottom

  UnreadCounter unread;

  int contact_list_idx = 0;
  int channel_list_idx = 0;

  ChannelDetails current_channel;
  int current_channel_idx = -1;

  ContactInfo current_contact;
  int current_contact_idx = -1;

  int getChannelCount();
  int getFilteredContactCount();
  bool getFilteredContactIndex(int list_idx, int &real_idx);

  void sendChatMessage();
  void renderStatusIcons();

  int renderFirstPage();
  int renderChannelsPage();
  int renderContactsPage();
  int renderChatPage();
  int renderRecentPage();
  int renderStatsPage();
  int renderGpsPage();

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs)
      : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs), _custom_prefs(custom_prefs) {
    chat_text_box.reserve(MAX_MESSAGE_LENGTH);
  }
  void messageRepeatsRecv(uint16_t count);
  void onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  void onContactMessageRecv(const ContactInfo &contact, const char *text);
  void onMessageSendAttempt(uint8_t attempt, uint8_t total, MessageSendState state);

  void poll() override;
  int render(DisplayDriver &display) override;
  bool handleInput(Keyboard::Event &e) override;
  void refreshSelectedContact();
  inline MainScreenPage getCurrentPage() const { return (MainScreenPage)current_page; }
  inline void setCurrentPage(MainScreenPage page) {
    page_to_return = MainScreenPage::FIRST;
    current_page = page;
  }
};
