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

#define PRESS_LABEL "long press / Enter"


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
#if UI_SENSORS_PAGE == 1
    SENSORS,
#endif
    SHUTDOWN,
    Count // keep as last
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  SensorManager *_sensors;
  NodePrefs *_node_prefs;
  uint8_t _page;
  bool _shutdown_init;
  AdvertPath recent[UI_RECENT_LIST_SIZE];
  // char text_box[TEXTBOX_MAX + 1];
  String text_box;

  int contact_list_idx;
  int contact_open_idx;
  int channel_list_idx;
  int channel_open_idx;

  void renderBatteryIndicator(DisplayDriver &display, uint16_t batteryMilliVolts) {
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

  CayenneLPP sensors_lpp;
  int sensors_nb = 0;
  bool sensors_scroll = false;
  int sensors_scroll_offset = 0;
  int next_sensors_refresh = 0;

  void refresh_sensors() {
    if (millis() > next_sensors_refresh) {
      sensors_lpp.reset();
      sensors_nb = 0;
      sensors_lpp.addVoltage(TELEM_CHANNEL_SELF, (float)board.getBattMilliVolts() / 1000.0f);
      sensors.querySensors(0xFF, sensors_lpp);
      LPPReader reader(sensors_lpp.getBuffer(), sensors_lpp.getSize());
      uint8_t channel, type;
      while (reader.readHeader(channel, type)) {
        reader.skipData(type);
        sensors_nb++;
      }
      sensors_scroll = sensors_nb > UI_RECENT_LIST_SIZE;
#if AUTO_OFF_MILLIS > 0
      next_sensors_refresh = millis() + 5000; // refresh sensor values every 5 sec
#else
      next_sensors_refresh = millis() + 60000; // refresh sensor values every 1 min
#endif
    }
  }

  int find_num_of_channels() { // not sure if there is no gaps
    ChannelDetails chan;
    int num = 0;

    for (int i = 0; i < MAX_GROUP_CHANNELS; i++) {
      if (!the_mesh.getChannel(i, chan) || strlen(chan.name) == 0) {
        break;
      }
      num++;
    }

    return num;
  }

public:
  MainScreen(CardputerUITask *task, mesh::RTCClock *rtc, SensorManager *sensors, NodePrefs *node_prefs)
      : _task(task), _rtc(rtc), _sensors(sensors), _node_prefs(node_prefs), _page(0), sensors_lpp(200) {
    _page = 0;
    _shutdown_init = false;
    contact_list_idx = 0;
    channel_list_idx = 0;
    contact_open_idx = -1;
    channel_open_idx = -1;
    text_box.reserve(UI_TEXTBOX_MAX);
  }

  void poll() override {
    if (_shutdown_init && !_task->isButtonPressed()) { // must wait for USR button to be released
      _task->shutdown();
    }
  }

  void sendChatMessage() {
    if (text_box.isEmpty()) {
      return;
    }

    ContactInfo contact;
    ChannelDetails chan;

    uint32_t ts = the_mesh.getRTCClock()->getCurrentTime();

    if (contact_open_idx > -1 && the_mesh.getContactByIdx(contact_open_idx, contact) &&
        contact.type == ADV_TYPE_CHAT) { // direct msg
      uint32_t est_timeout;
      uint32_t expected_ack;

      int result = the_mesh.sendMessage(contact, ts, 0, text_box.c_str(), expected_ack, est_timeout);

      if (result != MSG_SEND_FAILED) {
        text_box.clear();
      }
      return;
    }

    if (channel_open_idx > -1 && the_mesh.getChannel(channel_open_idx, chan) &&
        strlen(chan.name) > 0) { // channel msg

      bool ok = the_mesh.sendGroupMessage(ts, chan.channel, _node_prefs->node_name, text_box.c_str(),
                                          text_box.length());
      if (ok) {
        text_box.clear();
      }
      return;
    }
  }

  int render(DisplayDriver &display) override {
    char tmp[80];
    // node name
    display.setTextSize(1);
    display.setColor(DisplayDriver::GREEN);
    char filtered_name[sizeof(_node_prefs->node_name)];
    display.translateUTF8ToBlocks(filtered_name, _node_prefs->node_name, sizeof(filtered_name));
    display.setCursor(0, 0);
    display.print(filtered_name);

    // battery voltage
    renderBatteryIndicator(display, _task->getBattMilliVolts());

    // curr page indicator
    int y = 14;
    int x = display.width() / 2 - 5 * (MainScreenPage::Count - 1);
    for (uint8_t i = 0; i < MainScreenPage::Count; i++, x += 10) {
      if (i == _page) {
        display.fillRect(x - 1, y - 1, 3, 3);
      } else {
        display.fillRect(x, y, 1, 1);
      }
    }

    if (_page == MainScreenPage::FIRST) {
      display.setColor(DisplayDriver::YELLOW);
      display.setTextSize(2);
      sprintf(tmp, "MSG: %d", _task->getMsgCount());
      display.drawTextCentered(display.width() / 2, 20, tmp);

      if (_task->hasConnection()) {
        display.setColor(DisplayDriver::GREEN);
        display.setTextSize(1);
        display.drawTextCentered(display.width() / 2, 43, "< Connected >");

      } else if (the_mesh.getBLEPin() != 0) { // BT pin
        display.setColor(DisplayDriver::RED);
        display.setTextSize(2);
        sprintf(tmp, "Pin:%d", the_mesh.getBLEPin());
        display.drawTextCentered(display.width() / 2, 43, tmp);
      }
    } else if (_page == MainScreenPage::CHANNELS) {
      display.setColor(DisplayDriver::GREEN);
      display.drawTextCentered(display.width() / 2, 20, "Channels");

      int real_idx = 0;
      int list_page = channel_list_idx / UI_CHANNEL_LIST_SIZE;
      int list_idx = channel_list_idx % UI_CHANNEL_LIST_SIZE;

      for (int i = 0; i < UI_CHANNEL_LIST_SIZE; i++) {
        ChannelDetails chan;
        real_idx = list_page * UI_CHANNEL_LIST_SIZE + i;

        if (!the_mesh.getChannel(real_idx, chan) || strlen(chan.name) == 0) {
          break;
        }

        if (i == list_idx) {
          display.drawTextLeftAlign(5, 30 + i * 8, ">");
        }

        display.drawTextLeftAlign(25, 30 + i * 8, chan.name);
      }
    } else if (_page == MainScreenPage::CONTACTS) {
      display.setColor(DisplayDriver::GREEN);
      display.drawTextCentered(display.width() / 2, 20, "Contacts");
      int real_idx = 0;
      int list_page = contact_list_idx / UI_CONTACT_LIST_SIZE;
      int list_idx = contact_list_idx % UI_CONTACT_LIST_SIZE;

      for (int i = 0; i < UI_CONTACT_LIST_SIZE; i++) {
        ContactInfo contact;
        real_idx = list_page * UI_CONTACT_LIST_SIZE + i;

        if (!the_mesh.getContactByIdx(real_idx, contact)) {
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
    } else if (_page == MainScreenPage::CHAT) {
      display.setColor(DisplayDriver::GREEN);
      ContactInfo contact;
      ChannelDetails chan;

      if (contact_open_idx < 0 && channel_open_idx < 0) {
        display.drawTextCentered(display.width() / 2, 20, "Contact/Channel not selected");
      } else if (the_mesh.getContactByIdx(contact_open_idx, contact)) {
        display.drawTextCentered(display.width() / 2, 20, contact.name);
      } else if (the_mesh.getChannel(channel_open_idx, chan)) {
        display.drawTextCentered(display.width() / 2, 20, chan.name);
      }

      display.drawRect(1, display.height() - 15, display.width() - 1, 15);
      display.setColor(DisplayDriver::LIGHT);
      display.drawTextLeftAlign(5, display.height() - 10, text_box.c_str()); // todo limit
      display.setColor(DisplayDriver::GREEN);

    } else if (_page == MainScreenPage::RECENT) {
      the_mesh.getRecentlyHeard(recent, UI_RECENT_LIST_SIZE);
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

        char filtered_recent_name[sizeof(a->name)];
        display.translateUTF8ToBlocks(filtered_recent_name, a->name, sizeof(filtered_recent_name));
        display.drawTextEllipsized(0, y, max_name_width, filtered_recent_name);
        display.setCursor(display.width() - timestamp_width - 1, y);
        display.print(tmp);
      }
    } else if (_page == MainScreenPage::RADIO) {
      display.setColor(DisplayDriver::YELLOW);
      display.setTextSize(1);
      // freq / sf
      display.setCursor(0, 20);
      sprintf(tmp, "FQ: %06.3f   SF: %d", _node_prefs->freq, _node_prefs->sf);
      display.print(tmp);

      display.setCursor(0, 31);
      sprintf(tmp, "BW: %03.2f     CR: %d", _node_prefs->bw, _node_prefs->cr);
      display.print(tmp);

      // tx power,  noise floor
      display.setCursor(0, 42);
      sprintf(tmp, "TX: %ddBm", _node_prefs->tx_power_dbm);
      display.print(tmp);
      display.setCursor(0, 53);
      sprintf(tmp, "Noise floor: %d", radio_driver.getNoiseFloor());
      display.print(tmp);
    } else if (_page == MainScreenPage::BLUETOOTH) {
      display.setColor(DisplayDriver::GREEN);
      display.drawXbm((display.width() - 32) / 2, 18, _task->isSerialEnabled() ? bluetooth_on : bluetooth_off,
                      32, 32);
      display.setTextSize(1);
      display.drawTextCentered(display.width() / 2, 64 - 11, "toggle: " PRESS_LABEL);
    } else if (_page == MainScreenPage::ADVERT) {
      display.setColor(DisplayDriver::GREEN);
      display.drawXbm((display.width() - 32) / 2, 18, advert_icon, 32, 32);
      display.drawTextCentered(display.width() / 2, 64 - 11, "advert: " PRESS_LABEL);
#if ENV_INCLUDE_GPS == 1
    } else if (_page == MainScreenPage::GPS) {
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
#endif
#if UI_SENSORS_PAGE == 1
    } else if (_page == HomePage::SENSORS) {
      int y = 18;
      refresh_sensors();
      char buf[30];
      char name[30];
      LPPReader r(sensors_lpp.getBuffer(), sensors_lpp.getSize());

      for (int i = 0; i < sensors_scroll_offset; i++) {
        uint8_t channel, type;
        r.readHeader(channel, type);
        r.skipData(type);
      }

      for (int i = 0; i < (sensors_scroll ? UI_RECENT_LIST_SIZE : sensors_nb); i++) {
        uint8_t channel, type;
        if (!r.readHeader(channel, type)) { // reached end, reset
          r.reset();
          r.readHeader(channel, type);
        }

        display.setCursor(0, y);
        float v;
        switch (type) {
        case LPP_GPS: // GPS
          float lat, lon, alt;
          r.readGPS(lat, lon, alt);
          strcpy(name, "gps");
          sprintf(buf, "%.4f %.4f", lat, lon);
          break;
        case LPP_VOLTAGE:
          r.readVoltage(v);
          strcpy(name, "voltage");
          sprintf(buf, "%6.2f", v);
          break;
        case LPP_CURRENT:
          r.readCurrent(v);
          strcpy(name, "current");
          sprintf(buf, "%.3f", v);
          break;
        case LPP_TEMPERATURE:
          r.readTemperature(v);
          strcpy(name, "temperature");
          sprintf(buf, "%.2f", v);
          break;
        case LPP_RELATIVE_HUMIDITY:
          r.readRelativeHumidity(v);
          strcpy(name, "humidity");
          sprintf(buf, "%.2f", v);
          break;
        case LPP_BAROMETRIC_PRESSURE:
          r.readPressure(v);
          strcpy(name, "pressure");
          sprintf(buf, "%.2f", v);
          break;
        case LPP_ALTITUDE:
          r.readAltitude(v);
          strcpy(name, "altitude");
          sprintf(buf, "%.0f", v);
          break;
        case LPP_POWER:
          r.readPower(v);
          strcpy(name, "power");
          sprintf(buf, "%6.2f", v);
          break;
        default:
          r.skipData(type);
          strcpy(name, "unk");
          sprintf(buf, "");
        }
        display.setCursor(0, y);
        display.print(name);
        display.setCursor(display.width() - display.getTextWidth(buf) - 1, y);
        display.print(buf);
        y = y + 12;
      }
      if (sensors_scroll)
        sensors_scroll_offset = (sensors_scroll_offset + 1) % sensors_nb;
      else
        sensors_scroll_offset = 0;
#endif
    } else if (_page == MainScreenPage::SHUTDOWN) {
      display.setColor(DisplayDriver::GREEN);
      display.setTextSize(1);
      if (_shutdown_init) {
        display.drawTextCentered(display.width() / 2, 34, "hibernating...");
      } else {
        display.drawXbm((display.width() - 32) / 2, 18, power_icon, 32, 32);
        display.drawTextCentered(display.width() / 2, 64 - 11, "hibernate:" PRESS_LABEL);
      }
    }
    return 5000; // next render after 5000 ms
  }

  bool handleInput(char c) override {
    MESH_DEBUG_PRINT("kb %d '%c' isprint %d", c, c, isprint(c));

    if (_page == MainScreenPage::CONTACTS) {
      if (c == KEY_UP) {
        if (contact_list_idx == 0) {
          contact_list_idx = the_mesh.getNumContacts() - 1;
        } else {
          contact_list_idx--;
        }
        return true;
      }
      if (c == KEY_DOWN) {
        if (contact_list_idx < the_mesh.getNumContacts() - 1) {
          contact_list_idx++;
        } else {
          contact_list_idx = 0;
        }
        return true;
      }
      if (c == ASCII_CTRL_LF) {
        ContactInfo contact;
        if (the_mesh.getContactByIdx(contact_list_idx, contact) && contact.type == ADV_TYPE_CHAT) {
          contact_open_idx = contact_list_idx;
          channel_open_idx = -1;
          _page = MainScreenPage::CHAT;
          return true;
        }
      }
    }

    if (_page == MainScreenPage::CHANNELS) {
      if (c == KEY_UP) {
        if (channel_list_idx == 0) {
          channel_list_idx = find_num_of_channels() - 1;
        } else {
          channel_list_idx--;
        }
        return true;
      }
      if (c == KEY_DOWN) {
        if (channel_list_idx < find_num_of_channels() - 1) {
          channel_list_idx++;
        } else {
          channel_list_idx = 0;
        }
        return true;
      }
      if (c == ASCII_CTRL_LF) {
        ChannelDetails chan;
        if (the_mesh.getChannel(channel_list_idx, chan) && strlen(chan.name) > 0) {
          channel_open_idx = channel_list_idx;
          contact_open_idx = -1;
          _page = MainScreenPage::CHAT;
          return true;
        }
      }
    }

    if (_page == MainScreenPage::CHAT) {
      if (c == ASCII_CTRL_BACKSPACE) {
        if (!text_box.isEmpty()) {
          text_box.remove(text_box.length() - 1);
        }
        return true;
      }

      if (c == ASCII_CTRL_LF) {
        sendChatMessage();
        return true;
      }

      if (isprint(c)) {
        text_box += c;
        return true;
      }
    }

    if (c == KEY_LEFT || c == KEY_PREV) {
      _page = (_page + MainScreenPage::Count - 1) % MainScreenPage::Count;
      return true;
    }
    if (c == KEY_NEXT || c == KEY_RIGHT) {
      _page = (_page + 1) % MainScreenPage::Count;
      if (_page == MainScreenPage::RECENT) {
        _task->showAlert("Recent adverts", 800);
      }
      return true;
    }
    if (c == ASCII_CTRL_LF && _page == MainScreenPage::BLUETOOTH) {
      if (_task->isSerialEnabled()) { // toggle Bluetooth on/off
        _task->disableSerial();
      } else {
        _task->enableSerial();
      }
      return true;
    }
    if (c == ASCII_CTRL_LF && _page == MainScreenPage::ADVERT) {
      _task->notify(UIEventType::ack);
      if (the_mesh.advert()) {
        _task->showAlert("Advert sent!", 1000);
      } else {
        _task->showAlert("Advert failed..", 1000);
      }
      return true;
    }
#if ENV_INCLUDE_GPS == 1
    if (c == ASCII_CTRL_LF && _page == MainScreenPage::GPS) {
      _task->toggleGPS();
      return true;
    }
#endif
#if UI_SENSORS_PAGE == 1
    if (c == ASCII_CTRL_LF && _page == HomePage::SENSORS) {
      _task->toggleGPS();
      next_sensors_refresh = 0;
      return true;
    }
#endif
    if (c == ASCII_CTRL_LF && _page == MainScreenPage::SHUTDOWN) {
      _shutdown_init = true; // need to wait for button to be released
      return true;
    }

    if (c == 's') {
      _task->setSleepEnabled(!_task->isSleepEnabled());
      return true;
    }

    return false;
  }
};
