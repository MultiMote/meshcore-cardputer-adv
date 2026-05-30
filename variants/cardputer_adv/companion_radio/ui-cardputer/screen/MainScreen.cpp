#include "MainScreen.h"

#define PRESS_LABEL "long press / Enter"

void MainScreen::renderBatteryIndicator(DisplayDriver &display, uint16_t batteryMilliVolts) {
  {
    // Convert millivolts to percentage

    const int minMilliVolts = BATT_MIN_MILLIVOLTS;
    const int maxMilliVolts = BATT_MAX_MILLIVOLTS;
    int batteryPercentage = ((batteryMilliVolts - minMilliVolts) * 100) / (maxMilliVolts - minMilliVolts);
    if (batteryPercentage < 0) batteryPercentage = 0;     // Clamp to 0%
    if (batteryPercentage > 100) batteryPercentage = 100; // Clamp to 100%

    // battery icon
    int iconWidth = 24;
    int iconHeight = 10;
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

    // show muted icon if buzzer is muted
    if (_task->isBuzzerQuiet()) {
      display.setColor(DisplayDriver::RED);
      display.drawXbm(iconX - 9, iconY + 1, muted_icon, 8, 8);
      display.setColor(DisplayDriver::GREEN);
    }

    if (_task->isSleepEnabled()) {
      display.setColor(DisplayDriver::BLUE);
      display.drawXbm(iconX - 18, iconY + 1, sleep_icon, 8, 8);
      display.setColor(DisplayDriver::GREEN);
    }
  }
}

void MainScreen::renderFirstPage() {
  char tmp[80];

  display.setColor(DisplayDriver::YELLOW);
  display.setTextSize(2);
  sprintf(tmp, "MSG: %d", _task->getMsgCount());
  display.drawTextCentered(display.width() / 2, 20, tmp);

  if (_task->hasConnection()) {
    display.setColor(DisplayDriver::GREEN);
    display.setTextSize(1);
    display.drawTextCentered(display.width() / 2, 43, "< Connected >");

  } else if (the_mesh_cp.getBLEPin() != 0) { // BT pin
    display.setColor(DisplayDriver::RED);
    display.setTextSize(2);
    sprintf(tmp, "Pin:%d", the_mesh_cp.getBLEPin());
    display.drawTextCentered(display.width() / 2, 43, tmp);
  }

  display.setColor(DisplayDriver::GREEN);
  display.setTextSize(1);

  display.drawTextCentered(display.width() / 2, display.height() - 11, "Press OPT to open settings");
}

void MainScreen::renderChannelsPage() {
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
      display.drawTextLeftAlign(5, 30 + i * 8, ">");
    }

    display.drawTextLeftAlign(25, 30 + i * 8, channel.name);
  }
}

void MainScreen::renderContactsPage() {
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
      display.drawTextLeftAlign(5, 30 + i * 8, ">");
    }

    if (contact.type == ADV_TYPE_CHAT) {
      display.drawTextLeftAlign(15, 30 + i * 8, "C");
    }

    display.drawTextLeftAlign(25, 30 + i * 8, contact.name);
  }
}

void MainScreen::renderChatPage() {
  display.setColor(DisplayDriver::GREEN);
  ContactInfo contact;
  ChannelDetails channel;

  if (contact_open_idx < 0 && channel_open_idx < 0) {
    display.drawTextCentered(display.width() / 2, 20, "Contact/Channel not selected");
  } else if (the_mesh_cp.getContactByIdx(contact_open_idx, contact)) {
    display.drawTextCentered(display.width() / 2, 20, contact.name);
  } else if (the_mesh_cp.getChannel(channel_open_idx, channel)) {
    display.drawTextCentered(display.width() / 2, 20, channel.name);
  }

  display.drawRect(1, display.height() - 15, display.width() - 1, 15);
  display.setColor(DisplayDriver::LIGHT);
  display.drawTextLeftAlign(5, display.height() - 10, chat_text_box.c_str()); // todo limit
  display.setColor(DisplayDriver::GREEN);
}

void MainScreen::renderRecentPage() {
  char tmp[80];

  the_mesh_cp.getRecentlyHeard(recent, UI_RECENT_LIST_SIZE);
  display.setColor(DisplayDriver::GREEN);
  int y = 20;
  for (int i = 0; i < UI_RECENT_LIST_SIZE; i++, y += 11) {
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
}

void MainScreen::renderStatsPage() { // todo: separate menu maybe
  char tmp[80];

  display.drawTextCentered(display.width() / 2, 20, "Stats");

  display.setColor(DisplayDriver::YELLOW);

  sprintf(tmp, "Radio noise floor: %d", radio_driver.getNoiseFloor());
  display.drawTextLeftAlign(5, 30, tmp);
  sprintf(tmp, "Heap usage: %d/%d (%d%%)", ESP.getFreeHeap(), ESP.getHeapSize(),
          (ESP.getHeapSize() - ESP.getFreeHeap()) * 100 / ESP.getHeapSize());
  display.drawTextLeftAlign(5, 40, tmp);
  sprintf(tmp, "Packets received: %u", the_mesh_cp.receivedPacketsCount());
  display.drawTextLeftAlign(5, 50, tmp);
}

void MainScreen::renderAdvertPage() {
  display.setColor(DisplayDriver::GREEN);
  display.drawXbm((display.width() - 32) / 2, 18, advert_icon, 32, 32);
  display.drawTextCentered(display.width() / 2, 64 - 11, "advert: " PRESS_LABEL);
}

#if ENV_INCLUDE_GPS == 1
void MainScreen::renderGpsPage() {

  LocationProvider *nmea = sensors.getLocationProvider();
  char buf[50];
  int y = 18;
  bool gps_state = _task->getGPSState();
  strcpy(buf, gps_state ? "gps on" : "gps off");
  display.drawTextLeftAlign(0, y, buf);
  if (nmea == NULL) {
    y = y + 12;
    display.drawTextLeftAlign(0, y, "Can't access GPS");
  } else {
    strcpy(buf, nmea->isValid() ? "fix" : "no fix");
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + 12;
    display.drawTextLeftAlign(0, y, "sat");
    sprintf(buf, "%d", nmea->satellitesCount());
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + 12;
    display.drawTextLeftAlign(0, y, "pos");
    sprintf(buf, "%.4f %.4f", nmea->getLatitude() / 1000000., nmea->getLongitude() / 1000000.);
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + 12;
    display.drawTextLeftAlign(0, y, "alt");
    sprintf(buf, "%.2f", nmea->getAltitude() / 1000.);
    display.drawTextRightAlign(display.width() - 1, y, buf);
    y = y + 12;
  }
}
#endif

void MainScreen::renderShutdownPage() {
  display.setColor(DisplayDriver::GREEN);
  display.setTextSize(1);
  if (shutdown_init) {
    display.drawTextCentered(display.width() / 2, 34, "hibernating...");
  } else {
    display.drawXbm((display.width() - 32) / 2, 18, power_icon, 32, 32);
    display.drawTextCentered(display.width() / 2, 64 - 11, "hibernate:" PRESS_LABEL);
  }
}

int MainScreen::render(DisplayDriver &display) {
  char tmp[80];
  // node name
  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  display.setCursor(0, 0);
  display.print(_node_prefs->node_name);

  // battery voltage
  renderBatteryIndicator(display, _task->getBattMilliVolts());

  // curr page indicator
  int y = 14;
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
      renderFirstPage();
      break;
    case MainScreenPage::CHANNELS:
      renderChannelsPage();
      break;
    case MainScreenPage::CONTACTS:
      renderContactsPage();
      break;
    case MainScreenPage::CHAT:
      renderChatPage();
      break;
    case MainScreenPage::RECENT:
      renderRecentPage();
      break;
    case MainScreenPage::STATS:
      renderStatsPage();
      break;
    case MainScreenPage::ADVERT:
      renderAdvertPage();
      break;
#if ENV_INCLUDE_GPS
    case MainScreenPage::GPS:
      renderGpsPage();
      break;
#endif
    case MainScreenPage::SHUTDOWN:
      renderShutdownPage();
      break;
    default:
      break;
  }
  return 5000; // next render after 5000 ms
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

  ContactInfo contact;
  ChannelDetails channel;
  uint32_t ts = the_mesh_cp.getRTCClock()->getCurrentTime();

  if (contact_open_idx > -1 && the_mesh_cp.getContactByIdx(contact_open_idx, contact) &&
      contact.type == ADV_TYPE_CHAT) { // direct msg
    uint32_t est_timeout;
    uint32_t expected_ack;

    int result = the_mesh_cp.sendMessage(contact, ts, 0, chat_text_box.c_str(), expected_ack, est_timeout);

    if (result != MSG_SEND_FAILED) {
      chat_text_box.clear();
    }
    return;
  }

  if (channel_open_idx > -1 && the_mesh_cp.getChannel(channel_open_idx, channel) &&
      strlen(channel.name) > 0) { // channel msg

    bool ok = the_mesh_cp.sendGroupMessage(ts, channel.channel, _node_prefs->node_name, chat_text_box.c_str(),
                                           chat_text_box.length());
    if (ok) {
      chat_text_box.clear();
    }
    return;
  }
}

bool MainScreen::handleInput(char c) { // todo: refactor this mess
  MESH_DEBUG_PRINTLN("kb %d 0x%x '%c' isprint %d", c, c, c, isprint(c));

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
      ContactInfo contact;
      if (the_mesh_cp.getContactByIdx(contact_list_idx, contact)) {
        if (contact.type == ADV_TYPE_CHAT) {
          contact_open_idx = contact_list_idx;
          channel_open_idx = -1;
          current_page = MainScreenPage::CHAT;
        } else if ((contact.type == ADV_TYPE_REPEATER || contact.type == ADV_TYPE_ROOM) &&
                   !_task->isAlertActive()) {
          _task->ping(contact);
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
      ChannelDetails channel;
      if (the_mesh_cp.getChannel(channel_list_idx, channel) && strlen(channel.name) > 0) {
        channel_open_idx = channel_list_idx;
        contact_open_idx = -1;
        current_page = MainScreenPage::CHAT;
      }
      return true;
    }
  }

  if (current_page == MainScreenPage::CHAT) {
    if (c == ASCII_CTRL_BACKSPACE) {
      if (!chat_text_box.isEmpty()) {
        chat_text_box.remove(chat_text_box.length() - 1);
      }
      return true;
    }

    if (c == ASCII_CTRL_LF) {
      sendChatMessage();
      return true;
    }

    if (isprint(c)) {
      chat_text_box += c;
      return true;
    }
  }

  if (c == KEY_LEFT || c == KEY_PREV) {
    current_page = (current_page + MainScreenPage::Count - 1) % MainScreenPage::Count;
    return true;
  }
  if (c == KEY_NEXT || c == KEY_RIGHT) {
    current_page = (current_page + 1) % MainScreenPage::Count;
    if (current_page == MainScreenPage::RECENT) {
      _task->showAlert("Recent adverts", 800);
    }
    return true;
  }
  if (c == ASCII_CTRL_LF && current_page == MainScreenPage::ADVERT) {
    _task->notify(UIEventType::ack);
    if (the_mesh_cp.advert()) {
      _task->showAlert("Advert sent!", 1000);
    } else {
      _task->showAlert("Advert failed..", 1000);
    }
    return true;
  }

#if ENV_INCLUDE_GPS == 1
  if (c == ASCII_CTRL_LF && current_page == MainScreenPage::GPS) {
    _task->toggleGPS();
    return true;
  }
#endif

  if (c == ASCII_CTRL_LF && current_page == MainScreenPage::SHUTDOWN) {
    shutdown_init = true; // need to wait for button to be released
    return true;
  }

  if (c == ASCII_CTRL_DC1 && current_page == MainScreenPage::FIRST) {
    _task->gotoSettingsScreen();
    return true;
  }

  return false;
}
