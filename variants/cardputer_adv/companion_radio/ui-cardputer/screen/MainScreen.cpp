#include "MainScreen.h"

#define PRESS_LABEL      "long press / Enter"
#define MAX_MESSAGE_SIZE 133

void MainScreen::renderStatusIcons() {
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
  int iconX = display.width() - iconWidth - 5; // Position the icon near the top-right corner
  int iconY = 0;
  display.setColor(DisplayDriver::GREEN);

  // battery outline
  display.drawRect(iconX, iconY, iconWidth, iconHeight);

  // battery "cap"
  display.fillRect(iconX + iconWidth, iconY + (iconHeight / 4), 3, iconHeight / 2);

  // fill the battery based on the percentage
  int fillWidth = (batteryPercentage * (iconWidth - 4)) / 100;
  display.fillRect(iconX + 2, iconY + 2, fillWidth, iconHeight - 4);

  sprintf(tmp, "%d", batteryPercentage);
  display.drawTextCentered(iconX + 2 + iconWidth / 2, 0, tmp);
  display.setColor(DisplayDriver::DARK);
  display.drawTextCentered(iconX + 2 + iconWidth / 2 - 1, 0, tmp);

  iconX -= 3;
  iconY += 3;

  if (_task->isBuzzerQuiet()) {
    iconX -= 9;
    display.setColor(DisplayDriver::RED);
    display.drawXbm(iconX, iconY + 1, muted_icon, 8, 8);
  }

  if (_task->powerSaveEnabled()) {
    iconX -= 9;
    display.setColor(DisplayDriver::BLUE);
    display.drawXbm(iconX, iconY + 1, sleep_icon, 8, 8);
  }
}

int MainScreen::renderFirstPage() {
  char tmp[80];

  display.setColor(DisplayDriver::ORANGE);
  display.setTextSize(2);
  sprintf(tmp, "MSG: %d", _task->getMsgCount());
  display.drawTextCentered(display.width() / 2, 40, tmp);

  if (_task->hasConnection()) {
    display.setColor(DisplayDriver::GREEN);
    display.setTextSize(1);
    display.drawTextCentered(display.width() / 2, 63, "< Connected >");

  } else if (the_mesh_cp.getBLEPin() != 0) { // BT pin
    display.setColor(DisplayDriver::RED);
    display.setTextSize(2);
    sprintf(tmp, "Pin: %d", the_mesh_cp.getBLEPin());
    display.drawTextCentered(display.width() / 2, 63, tmp);
  }

  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  display.drawTextCentered(display.width() / 2, display.height() - UI_TEXT_LINE_HEIGHT * 2 - 2,
                           "Press OPT to open Settings");
  display.setColor(DisplayDriver::ORANGE);
  display.drawTextCentered(display.width() / 2, display.height() - UI_TEXT_LINE_HEIGHT - 2,
                           "Press T to open Tools");

  return 5000;
}

int MainScreen::renderChannelsPage() {
  display.setColor(DisplayDriver::GREEN);
  display.drawTextCentered(display.width() / 2, 20, "Channels");

  int real_idx = 0;
  int list_page = channel_list_idx / UI_CHANNEL_LIST_SIZE;
  int list_idx = channel_list_idx % UI_CHANNEL_LIST_SIZE;

  for (int i = 0; i < UI_CHANNEL_LIST_SIZE; i++) {
    ChannelDetails channel;
    real_idx = list_page * UI_CHANNEL_LIST_SIZE + i;

    if (!the_mesh_cp.getChannel(real_idx, channel) || strlen(channel.name) == 0) {
      break;
    }

    if (i == list_idx) {
      display.drawTextLeftAlign(5, 30 + i * UI_TEXT_LINE_HEIGHT, ">");
    }

    display.drawTextLeftAlign(25, 30 + i * UI_TEXT_LINE_HEIGHT, channel.name);
  }

  return 5000;
}

int MainScreen::renderContactsPage() {
  display.setColor(DisplayDriver::GREEN);
  display.drawTextCentered(display.width() / 2, 20, "Contacts");
  int real_idx = 0;
  int list_page = contact_list_idx / UI_CONTACT_LIST_SIZE;
  int list_idx = contact_list_idx % UI_CONTACT_LIST_SIZE;

  for (int i = 0; i < UI_CONTACT_LIST_SIZE; i++) {
    ContactInfo contact;
    real_idx = list_page * UI_CONTACT_LIST_SIZE + i;

    if (!the_mesh_cp.getContactByIdx(real_idx, contact)) {
      break;
    }

    if (i == list_idx) {
      display.drawTextLeftAlign(5, 30 + i * UI_TEXT_LINE_HEIGHT, ">");
    }

    if (contact.type == ADV_TYPE_CHAT) {
      display.drawTextLeftAlign(15, 30 + i * UI_TEXT_LINE_HEIGHT, "C");
    }

    display.drawTextLeftAlign(25, 30 + i * UI_TEXT_LINE_HEIGHT, contact.name);
  }
  return 5000;
}

int MainScreen::renderChatPage() {
  display.setColor(DisplayDriver::GREEN);

  if (current_contact_idx == -1 && current_channel_idx == -1) {
    display.setColor(DisplayDriver::ORANGE);
    display.drawTextCentered(display.width() / 2, 20, "Contact/Channel not selected");
  } else if (current_contact_idx != -1) {
    display.drawTextCentered(display.width() / 2, 20, current_contact.name);
  } else if (current_channel_idx != -1) {
    display.drawTextCentered(display.width() / 2, 20, current_channel.name);
  }

  display.setColor(DisplayDriver::GREEN);

  for (size_t i = 0; i < chat_history.count(); i++) {
    const HistoryMessage *msg;
    if (chat_history.get(i, msg)) {
      if(msg->out) {
        int text_w = display.getTextWidth(msg->text);

        if(text_w > display.width()) {
          text_w = display.width() - 10;
        }

        display.drawTextEllipsized(display.width() - text_w - 5, 30 + i * UI_TEXT_LINE_HEIGHT, text_w, msg->text);
      } else {
        display.drawTextEllipsized(5, 30 + i * UI_TEXT_LINE_HEIGHT, display.width() - 5, msg->text);
      }
    }
  }

  int start_x = 5;
  display.drawRect(1, display.height() - UI_TEXT_LINE_HEIGHT - 2, display.width() - 1, 1);

  if (_keyboard_layout->hasSecondary()) {
    const char *layout_label = _keyboard_layout->getCurrentCode();
    int label_width = std::max(display.getTextWidth(_keyboard_layout->getPrimaryCode()),
                               display.getTextWidth(_keyboard_layout->getSecondaryCode()));
    start_x += label_width;
    display.drawRect(start_x, display.height() - UI_TEXT_LINE_HEIGHT - 2, 1, UI_TEXT_LINE_HEIGHT + 2);
    display.drawTextLeftAlign(2, display.height() - UI_TEXT_LINE_HEIGHT - 2, layout_label);
    start_x += 5;
  }

  display.setColor(DisplayDriver::LIGHT);
  display.drawTextEllipsized(start_x, display.height() - UI_TEXT_LINE_HEIGHT - 2, display.width() - start_x,
                             chat_text_box.c_str());
  return 15000;
}

int MainScreen::renderRecentPage() {
  char tmp[80];

  the_mesh_cp.getRecentlyHeard(recent, UI_RECENT_LIST_SIZE);
  display.setColor(DisplayDriver::GREEN);

  int y = 20;
  display.drawTextCentered(display.width() / 2, y, "Recent adverts");

  y += UI_TEXT_LINE_HEIGHT;

  for (int i = 0; i < UI_RECENT_LIST_SIZE; i++, y += UI_TEXT_LINE_HEIGHT) {
    auto a = &recent[i];
    if (a->name[0] == 0) continue; // empty slot
    int secs = _rtc->getCurrentTime() - a->recv_timestamp;
    if (secs < 60) {
      sprintf(tmp, "%ds", secs);
    } else if (secs < 60 * 60) {
      sprintf(tmp, "%dm", secs / 60);
    } else {
      sprintf(tmp, "%dh", secs / (60 * 60));
    }

    int timestamp_width = display.getTextWidth(tmp);
    int max_name_width = display.width() - timestamp_width - 1;

    display.drawTextEllipsized(0, y, max_name_width, a->name);
    display.setCursor(display.width() - timestamp_width - 1, y);
    display.print(tmp);
  }

  return 5000;
}

int MainScreen::renderStatsPage() { // todo: separate menu maybe
  char tmp[80];

  int y = 20;
  display.drawTextCentered(display.width() / 2, y, "Stats");

  display.setColor(DisplayDriver::YELLOW);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Radio noise floor: %d", radio_driver.getNoiseFloor());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Heap usage: %d/%d (%d%%)", ESP.getFreeHeap(), ESP.getHeapSize(),
          (ESP.getHeapSize() - ESP.getFreeHeap()) * 100 / ESP.getHeapSize());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Packets received: %u", the_mesh_cp.receivedPacketsCount());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Battery: %.0fmV", (float)_task->getBattMilliVolts() * _custom_prefs->battery_correction);
  display.drawTextLeftAlign(5, y, tmp);

  return 5000;
}

#if ENV_INCLUDE_GPS == 1
int MainScreen::renderGpsPage() {

  LocationProvider *nmea = sensors.getLocationProvider();
  char buf[50];
  int y = 18;
  bool gps_state = _task->getGPSState();
  strcpy(buf, gps_state ? "gps on" : "gps off");
  display.drawTextLeftAlign(0, y, buf);
  if (nmea == NULL) {
    y = y + UI_TEXT_LINE_HEIGHT;
    display.drawTextLeftAlign(0, y, "Can't access GPS");
  } else {
    strcpy(buf, nmea->isValid() ? "fix" : "no fix");
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    display.drawTextLeftAlign(0, y, "sat");
    sprintf(buf, "%d", nmea->satellitesCount());
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    display.drawTextLeftAlign(0, y, "pos");
    sprintf(buf, "%.4f %.4f", nmea->getLatitude() / 1000000., nmea->getLongitude() / 1000000.);
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    display.drawTextLeftAlign(0, y, "alt");
    sprintf(buf, "%.2f", nmea->getAltitude() / 1000.);
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
  }
  return 5000;
}
#endif

int MainScreen::renderShutdownPage() {
  display.setColor(DisplayDriver::GREEN);
  display.setTextSize(1);
  if (shutdown_init) {
    display.drawTextCentered(display.width() / 2, 34, "hibernating...");
  } else {
    display.drawXbm((display.width() - 32) / 2, 18, power_icon, 32, 32);
    display.drawTextCentered(display.width() / 2, 64 - 11, "hibernate:" PRESS_LABEL);
  }
  return 5000;
}

int MainScreen::render(DisplayDriver &display) {
  char tmp[80];
  // node name
  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  display.setCursor(0, 0);
  display.print(_node_prefs->node_name);

  renderStatusIcons();

  display.setColor(DisplayDriver::GREEN);

  // curr page indicator
  int y = 16;
  int x = display.width() / 2 - 5 * (MainScreenPage::Count - 1);
  for (uint8_t i = 0; i < MainScreenPage::Count; i++, x += 10) {
    if (i == current_page) {
      display.fillRect(x - 1, y - 1, 3, 3);
    } else {
      display.fillRect(x, y, 1, 1);
    }
  }

  switch (current_page) {
    case MainScreenPage::FIRST:
      return renderFirstPage();
    case MainScreenPage::CHANNELS:
      return renderChannelsPage();
    case MainScreenPage::CONTACTS:
      return renderContactsPage();
    case MainScreenPage::CHAT:
      return renderChatPage();
    case MainScreenPage::RECENT:
      return renderRecentPage();
    case MainScreenPage::STATS:
      return renderStatsPage();
#if ENV_INCLUDE_GPS
    case MainScreenPage::GPS:
      return renderGpsPage();
#endif
    case MainScreenPage::SHUTDOWN:
      return renderShutdownPage();
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
  if (current_channel_idx != -1 &&
      memcmp(channel.secret, current_channel.channel.secret, CONTACT_LOOKUP_BYTES) == 0) {
    HistoryMessage *msg = chat_history.push_ref();
    msg->out = false;
    snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", text);
  }
}

void MainScreen::onContactMessageRecv(const ContactInfo &contact, const char *text) {
  if (current_contact_idx != -1 &&
      memcmp(contact.id.pub_key, current_contact.id.pub_key, CONTACT_LOOKUP_BYTES) == 0) {
    HistoryMessage *msg = chat_history.push_ref();
    msg->out = false;
    snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", text);
  }
}

void MainScreen::poll() {
  if (shutdown_init && !_task->isButtonPressed()) { // must wait for USR button to be released
    _task->shutdown();
  }
}

int MainScreen::getChannelCount() { // not sure if there is no gaps
  ChannelDetails channel;
  int num = 0;

  for (int i = 0; i < MAX_GROUP_CHANNELS; i++) {
    if (!the_mesh_cp.getChannel(i, channel) || strlen(channel.name) == 0) {
      break;
    }
    num++;
  }

  return num;
}

void MainScreen::sendChatMessage() {
  if (chat_text_box.isEmpty()) {
    return;
  }

  uint32_t ts = the_mesh_cp.getRTCClock()->getCurrentTime();

  if (current_contact_idx != -1 && current_contact.type == ADV_TYPE_CHAT) { // direct msg
    uint32_t est_timeout;
    uint32_t expected_ack;

    int result =
        the_mesh_cp.sendMessage(current_contact, ts, 0, chat_text_box.c_str(), expected_ack, est_timeout);

    if (result != MSG_SEND_FAILED) {
      HistoryMessage *msg = chat_history.push_ref();
      msg->out = true;
      snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", chat_text_box.c_str());

      chat_text_box.clear();
    }
    return;
  }

  if (current_channel_idx != -1) { // channel msg

    bool ok = the_mesh_cp.sendGroupMessage(ts, current_channel.channel, _node_prefs->node_name,
                                           chat_text_box.c_str(), chat_text_box.length());
    if (ok) {
      HistoryMessage *msg = chat_history.push_ref();
      msg->out = true;

      snprintf(msg->text, MAX_MESSAGE_LENGTH, "%s", chat_text_box.c_str());

      chat_text_box.clear();
      _task->showAlert("Waiting for repeats...", 2000);
    }
    return;
  }
}

// In case if input has utf-8 multibyte characters
void MainScreen::chatInputRemoveLastChar() {
  unsigned int len = chat_text_box.length();
  if (len == 0) {
    return;
  }

  unsigned int bytesToRemove = 1;

  while (len - bytesToRemove > 0 && ((unsigned char)chat_text_box[len - bytesToRemove] & 0xC0) == 0x80) {
    bytesToRemove++;
  }

  chat_text_box.remove(len - bytesToRemove, bytesToRemove);
}

bool MainScreen::handleInput(char c) { // todo: refactor this mess
  if (current_page == MainScreenPage::CONTACTS) {
    if (c == KEY_UP) {
      if (contact_list_idx == 0) {
        contact_list_idx = the_mesh_cp.getNumContacts() - 1;
      } else {
        contact_list_idx--;
      }
      return true;
    }
    if (c == KEY_DOWN) {
      if (contact_list_idx < the_mesh_cp.getNumContacts() - 1) {
        contact_list_idx++;
      } else {
        contact_list_idx = 0;
      }
      return true;
    }
    if (c == ASCII_CTRL_LF) {
      if (the_mesh_cp.getContactByIdx(contact_list_idx, current_contact)) {
        if (current_contact.type == ADV_TYPE_CHAT) {
          current_contact_idx = contact_list_idx;
          current_channel_idx = -1;
          current_page = MainScreenPage::CHAT;
          the_mesh_cp.loadMessageHistory(current_contact.id.pub_key, false, chat_history);
        } else if ((current_contact.type == ADV_TYPE_REPEATER || current_contact.type == ADV_TYPE_ROOM) &&
                   !_task->isAlertActive()) {
          the_mesh_cp.sendPing(current_contact);
          _task->showAlert("Waiting for response...", 4000);
        }
      }
      return true;
    }
  }

  if (current_page == MainScreenPage::CHANNELS) {
    if (c == KEY_UP) {
      if (channel_list_idx == 0) {
        channel_list_idx = getChannelCount() - 1;
      } else {
        channel_list_idx--;
      }
      return true;
    }
    if (c == KEY_DOWN) {
      if (channel_list_idx < getChannelCount() - 1) {
        channel_list_idx++;
      } else {
        channel_list_idx = 0;
      }
      return true;
    }
    if (c == ASCII_CTRL_LF) {
      if (the_mesh_cp.getChannel(channel_list_idx, current_channel) && strlen(current_channel.name) > 0) {
        current_channel_idx = channel_list_idx;
        current_contact_idx = -1;
        current_page = MainScreenPage::CHAT;
        the_mesh_cp.loadMessageHistory(current_channel.channel.secret, true, chat_history);
      }
      return true;
    }
  }

  if (current_page == MainScreenPage::CHAT) {
    if (c == ASCII_CTRL_BACKSPACE) {
      chatInputRemoveLastChar();
      return true;
    }

    if (c == ASCII_CTRL_LF) {
      sendChatMessage();
      return true;
    }

    if (c == ASCII_CTRL_SUBST) {
      _keyboard_layout->switchLayout();
      return true;
    }

    if (isprint(c)) {
      const char *repl = _keyboard_layout->findReplacement(c);
      int maxlen = MAX_MESSAGE_SIZE - strlen(_node_prefs->node_name);

      if (repl != nullptr && chat_text_box.length() + strlen(repl) <= maxlen) {
        chat_text_box += repl;
      } else if (chat_text_box.length() < maxlen) {
        chat_text_box += c;
      }
      return true;
    }
  }

  if (c == KEY_LEFT || c == KEY_PREV) {
    current_page = (current_page + MainScreenPage::Count - 1) % MainScreenPage::Count;
    return true;
  }

  if (c == KEY_NEXT || c == KEY_RIGHT) {
    current_page = (current_page + 1) % MainScreenPage::Count;
    return true;
  }

#if ENV_INCLUDE_GPS == 1
  if (c == ASCII_CTRL_LF && current_page == MainScreenPage::GPS) {
    _task->toggleGPS();
    return true;
  }
#endif

  if (current_page == MainScreenPage::SHUTDOWN) {
    if (c == ASCII_CTRL_LF) {
      shutdown_init = true; // need to wait for button to be released
      return true;
    }

    if (c == 'r') {
      ESP.restart();
      return true;
    }
  }

  if (current_page == MainScreenPage::FIRST) {
    if (c == ASCII_CTRL_DC1) {
      _task->gotoSettingsScreen();
      return true;
    }

    if (c == 't') {
      _task->gotoToolsScreen();
      return true;
    }
  }

  if (c == ASCII_CTRL_ESCAPE) {
    current_page = MainScreenPage::FIRST;
    return true;
  }

  return false;
}
