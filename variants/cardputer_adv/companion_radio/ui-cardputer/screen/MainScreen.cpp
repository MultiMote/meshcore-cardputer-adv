#include "MainScreen.h"

#include "helpers.h"
#include "main/ChannelsPage.h"
#include "main/ChatPage.h"
#include "main/ContactsPage.h"
#include "main/FirstPage.h"
#include "main/GpsPage.h"
#include "main/RecentAdvertsPage.h"
#include "main/StatsPage.h"

MainScreen::MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors,
                       NodePrefs *node_prefs, CustomNodePrefs *custom_prefs)
    : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs), _custom_prefs(custom_prefs) {
  contacts_page = new ContactsPage(this);
  channels_page = new ChannelsPage(this);
  chat_page = new ChatPage(this);
  stats_page = new StatsPage(this);
  gps_page = new GpsPage(this);
  first_page = new FirstPage(this);
  recent_adverts_page = new RecentAdvertsPage(this, rtc);
}

void MainScreen::renderStatusIcons(CardputerDisplay &lcd) {
  char tmp[8];
  uint16_t batteryMilliVolts = _task->getBattMilliVolts();
  // Convert millivolts to percentage
  const int minMilliVolts = BATT_MIN_MILLIVOLTS;
  const int maxMilliVolts = BATT_MAX_MILLIVOLTS;
  int batteryMilliVoltsCorrected = _custom_prefs->battery_correction * batteryMilliVolts;
  int batteryPercentage =
      ((batteryMilliVoltsCorrected - minMilliVolts) * 100) / (maxMilliVolts - minMilliVolts);
  if (batteryPercentage < 0) batteryPercentage = 0;     // Clamp to 0%
  if (batteryPercentage > 100) batteryPercentage = 100; // Clamp to 100%

  // battery icon
  int iconWidth = 26;
  int iconHeight = 14;
  int iconX = lcd.width() - iconWidth - 5; // Position the icon near the top-right corner
  int iconY = 2;
  lcd.setColor(CardputerDisplay::P_GREEN);

  // battery outline
  lcd.drawRect(iconX, iconY, iconWidth, iconHeight);

  // battery "cap"
  lcd.fillRect(iconX + iconWidth, iconY + (iconHeight / 4), 3, iconHeight / 2);

  // fill the battery based on the percentage
  int fillWidth = (batteryPercentage * (iconWidth - 4)) / 100;
  lcd.fillRect(iconX + 2, iconY + 2, fillWidth, iconHeight - 4);

  sprintf(tmp, "%d", batteryPercentage);
  lcd.drawTextCentered(iconX + 2 + iconWidth / 2, 0, tmp);
  lcd.setColor(CardputerDisplay::P_BLACK);
  lcd.drawTextCentered(iconX + 2 + iconWidth / 2 - 1, 0, tmp);

  iconX = 3;
  iconY = 5;

  lcd.setColor(CardputerDisplay::P_GREEN);

  CardputerLayout *lay = _task->getBoard()->getLayout();

  if (lay->hasAlternateLayout()) {
    const char *layout_label = _task->getBoard()->getLayout()->getCurrentCode();
    int label_width =
        std::max(lcd.getTextWidth(lay->getMainLayoutCode()), lcd.getTextWidth(lay->getAlternateLayoutCode()));
    lcd.drawTextLeftAlign(iconX, 0, layout_label);
    iconX += label_width + 5;
  }

  if (_task->isBuzzerQuiet()) {
    lcd.drawXbm(iconX, iconY, muted_icon, 8, 8);
    iconX += 11;
  }

  if (_task->powerSaveEnabled()) {
    lcd.drawXbm(iconX, iconY, sleep_icon, 8, 8);
    iconX += 11;
  }

  if (unread.countChats() > 0) {
    lcd.drawXbm(iconX, iconY, unread_icon, 8, 8);
    iconX += 11;
  }
}

int MainScreen::render(CardputerDisplay &lcd) {
  lcd.setTextSize(1);

  renderStatusIcons(lcd);

  lcd.setColor(CardputerDisplay::P_GREEN);

  // curr page indicator
  int y = 16;
  int x = lcd.width() / 2 - 5 * (MainScreenPage::Count - 1);
  for (uint8_t i = 0; i < MainScreenPage::Count; i++, x += 10) {
    if (i == current_page) {
      lcd.fillRect(x - 1, y - 1, 3, 3);
    } else {
      lcd.fillRect(x, y, 1, 1);
    }
  }

  switch (current_page) {
    case MainScreenPage::FIRST:
      return first_page->render(lcd);
    case MainScreenPage::CHANNELS:
      return channels_page->render(lcd);
    case MainScreenPage::CONTACTS:
      return contacts_page->render(lcd);
    case MainScreenPage::CHAT:
      return chat_page->render(lcd);
    case MainScreenPage::RECENT:
      return recent_adverts_page->render(lcd);
    case MainScreenPage::STATS:
      return stats_page->render(lcd);
    case MainScreenPage::GPS:
      return gps_page->render(lcd);
    default:
      break;
  }
  return 5000;
}

void MainScreen::messageRepeatsRecv(uint16_t count) {
  if (current_page == MainScreenPage::CHAT) {
    char buf[32];
    sprintf(buf, "Heard repeats: %d", count);
    _task->showAlert(buf, 2000);
    _task->playSound(SoundType::MessageAck);
  }
}

void MainScreen::onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text) {
  bool to_selected = static_cast<ChatPage *>(chat_page)->onChannelMessageRecv(channel, text);

  if (!to_selected) {
    unread.addChannel(channel.secret, 1);
  }
}

void MainScreen::onContactMessageRecv(const ContactInfo &contact, const char *text) {
  bool to_selected = static_cast<ChatPage *>(chat_page)->onContactMessageRecv(contact, text);

  if (!to_selected) {
    unread.addContact(contact.id.pub_key, 1);
  }
}

void MainScreen::onMessageSendAttempt(uint8_t attempt, uint8_t total, MessageSendState state) {
  char buf[32];

  if (state == MessageSendState::MESSAGE_FAILED) {
    _task->showAlert("Message ACK not received", 2000);
    return;
  }

  if (state == MessageSendState::MESSAGE_DELIVERED) {
    refreshSelectedContact();
    _task->showAlert("Message delivered", 2000);
    _task->playSound(SoundType::MessageAck);
    return;
  }

  if (attempt == 4 && total == 6) { // Path reset
    refreshSelectedContact();
  }

  int ms = std::max(DIRECT_SEND_ROUTE_RESEND_RELAY, DIRECT_SEND_FLOOD_RESEND_RELAY);

  if (attempt == 1) {
    _task->showAlert("Waiting for delivery...", ms);
  } else {
    sprintf(buf, "Resending... (%u/%u)", attempt, total);
    _task->showAlert(buf, ms);
  }
}

void MainScreen::poll() {}

bool MainScreen::handleInput(Keyboard::Event &e) {
  if (e.modifiers.ctrl && e.key == Keyboard::KEY_SPACE) {
    _task->getBoard()->getLayout()->switchLayout();
    return true;
  }

  if (current_page == MainScreenPage::CONTACTS && contacts_page->handleInput(e)) {
    return true;
  }

  if (current_page == MainScreenPage::CHANNELS && channels_page->handleInput(e)) {
    return true;
  }

  if (current_page == MainScreenPage::CHAT && chat_page->handleInput(e)) {
    return true;
  }

  if (current_page == MainScreenPage::GPS && gps_page->handleInput(e)) {
    return true;
  }

  if (current_page == MainScreenPage::FIRST && first_page->handleInput(e)) {
    return true;
  }

  if (e.key == Keyboard::ARROW_LEFT) {
    auto p = static_cast<MainScreenPage>((current_page + MainScreenPage::Count - 1) % MainScreenPage::Count);
    setCurrentPage(p);
    return true;
  }

  if (e.key == Keyboard::ARROW_RIGHT || e.key == Keyboard::KEY_TAB) {
    auto p = static_cast<MainScreenPage>((current_page + 1) % MainScreenPage::Count);
    setCurrentPage(p);
    return true;
  }

  if (e.key == Keyboard::KEY_ESC) {
    current_page = MainScreenPage::FIRST;
    return true;
  }
  return false;
}

void MainScreen::refreshSelectedContact() {
  static_cast<ChatPage *>(chat_page)->refreshSelectedContact();
}

void MainScreen::selectContact(ContactInfo &contact, int idx) {
  setCurrentPage(MainScreenPage::CHAT);
  page_to_return = MainScreenPage::CONTACTS;
  unread.resetContact(contact.id.pub_key);

  static_cast<ChatPage *>(chat_page)->selectContact(contact, idx);
}

void MainScreen::selectChannel(const ChannelDetails &channel, int idx) {
  setCurrentPage(MainScreenPage::CHAT);
  page_to_return = MainScreenPage::CHANNELS;
  unread.resetChannel(channel.channel.secret);

  static_cast<ChatPage *>(chat_page)->selectChannel(channel, idx);
}
