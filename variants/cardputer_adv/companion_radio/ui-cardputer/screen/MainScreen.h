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

  UnreadCounter unread;

  CardputerScreen *first_page;
  CardputerScreen *contacts_page;
  CardputerScreen *channels_page;
  CardputerScreen *chat_page;
  CardputerScreen *stats_page;
  CardputerScreen *gps_page;
  CardputerScreen *recent_adverts_page;

  void renderStatusIcons(CardputerDisplay &lcd);

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs);
  void messageRepeatsRecv(uint16_t count);
  void onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  void onContactMessageRecv(const ContactInfo &contact, const char *text);
  void onMessageSendAttempt(uint8_t attempt, uint8_t total, MessageSendState state);

  void poll() override;
  int render(CardputerDisplay &display) override;
  bool handleInput(Keyboard::Event &e) override;
  void refreshSelectedContact();
  void selectContact(ContactInfo &contact, int idx);
  void selectChannel(const ChannelDetails &channel, int idx);

  inline MainScreenPage getCurrentPage() const { return (MainScreenPage)current_page; }
  inline void setCurrentPage(MainScreenPage page) {
    page_to_return = MainScreenPage::FIRST;
    current_page = page;
  }
  inline void returnToLastPage() { setCurrentPage(page_to_return); }
  inline const UnreadCounter *getUnread() { return &unread; }
  inline CardputerUITask *getUiTask() { return _task; }
};
