#include "CardputerUITask.h"

#include "screen/MainScreen.h"
#include "screen/MsgPreviewScreen.h"
#include "screen/SettingsScreen.h"
#include "screen/SplashScreen.h"
#include "screen/ToolsScreen.h"
#include "target.h"

void CardputerUITask::begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs,
                            CustomNodePrefs *custom_prefs) {
  _display = display;
  _sensors = sensors;
  auto_off_time = millis() + AUTO_OFF_MILLIS;

  _node_prefs = node_prefs;
  _custom_prefs = custom_prefs;

  if (_display != NULL) {
    _display->turnOn();
  }

  ui_started_at = millis();

  splash_screen = new SplashScreen(this);
  main_screen = new MainScreen(this, &rtc_clock, sensors, node_prefs, custom_prefs);
  settings_screen = new SettingsScreen(this, &rtc_clock, node_prefs, custom_prefs, _board);
  msg_preview_screen = new MsgPreviewScreen(this, &rtc_clock);
  tools_screen = new ToolsScreen(this, &rtc_clock);
  setCurrScreen(splash_screen);
}

void CardputerUITask::showAlert(const char *text, int duration_millis) {
  snprintf(alert_text, sizeof(alert_text), "%s", text);
  alert_expiry = millis() + duration_millis;
  next_refresh = 0;
}

void CardputerUITask::dismissAlert() {
  alert_expiry = 0;
  next_refresh = 0;
}

bool CardputerUITask::isAlertActive() {
  return millis() < alert_expiry;
}

void CardputerUITask::notify(UIEventType t) {
  if (t == UIEventType::channelMessage || t == UIEventType::contactMessage) {
    playSound(SoundType::NewMessage);
  }
}

void CardputerUITask::playSound(SoundType t) {
  if (_node_prefs->buzzer_quiet) {
    return;
  }

  switch (t) {
    case SoundType::Keyboard:
      _board->getSpeaker()->queueTone(4000, 50, 0.2f);
      break;
    case SoundType::NewMessage:
      _board->getSpeaker()->queueTone(3000, 100);
      break;
    case SoundType::DiscoveryResult:
    case SoundType::MessageAck:
      _board->getSpeaker()->queueTone(800, 70, 0.3f);
      break;
  }
}

void CardputerUITask::msgRead(int msgcount) {
  unsynced_msg_count = msgcount;
  if (msgcount == 0) {
    gotoMainScreen();
  }
}

void CardputerUITask::newMsg(uint8_t path_len, const char *from_name, const char *text, int msgcount) {
  unsynced_msg_count = msgcount;

  if (current_screen == main_screen && ((MainScreen *)main_screen)->getCurrentPage() == MainScreen::CHAT &&
      _display->isOn()) {
    return;
  }

  ((MsgPreviewScreen *)msg_preview_screen)->addPreview(path_len, from_name, text);
  setCurrScreen(msg_preview_screen);

  if (_display != NULL) {
    if (!_display->isOn() && !hasConnection()) {
      _display->turnOn();
    }
    if (_display->isOn()) {
      auto_off_time = millis() + AUTO_OFF_MILLIS; // extend the auto-off timer
      next_refresh = 100;                         // trigger refresh
    }
  }
}

void CardputerUITask::onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text) {
  ((MainScreen *)main_screen)->onChannelMessageRecv(channel, text);
}

void CardputerUITask::onContactMessageRecv(const ContactInfo &contact, const char *text) {
  ((MainScreen *)main_screen)->onContactMessageRecv(contact, text);
}

void CardputerUITask::onMessageSendAttempt(uint8_t attempt, uint8_t total, MessageSendState state) {
  auto_off_time = millis() + AUTO_OFF_MILLIS;

  if (current_screen == main_screen) {
    static_cast<MainScreen *>(main_screen)->onMessageSendAttempt(attempt, total, state);
  }
}

void CardputerUITask::pingRecv(float snr_tx, float snr_rx) {
  char buf[40];
  sprintf(buf, "SNR there/back: %.2f/%.2f", snr_tx, snr_rx);
  showAlert(buf, 4000);
}

void CardputerUITask::discoverRecv(const mesh::Identity &id, float snr) {
  if (current_screen == tools_screen) {
    static_cast<ToolsScreen *>(tools_screen)->discoverRecv(id, snr);
  }
}

void CardputerUITask::messageRepeatsRecv(uint16_t count) {
  if (current_screen == main_screen) {
    static_cast<MainScreen *>(main_screen)->messageRepeatsRecv(count);
  }
}

void CardputerUITask::setCurrScreen(CardputerScreen *c) {
  current_screen = c;
  next_refresh = 100;
}

/*
  hardware-agnostic pre-shutdown activity should be done here
*/
void CardputerUITask::shutdown(bool restart) {
  if (restart) {
    _board->reboot();
  } else {
    _display->turnOff();
    radio_driver.powerOff();
    _board->powerOff();
  }
}

// In case if input has utf-8 multibyte characters
void CardputerUITask::removeLastStringChar(String &str) {
  unsigned int len = str.length();
  if (len == 0) {
    return;
  }

  unsigned int bytesToRemove = 1;

  while (len - bytesToRemove > 0 && ((uint8_t)str.charAt(len - bytesToRemove) & 0xC0) == 0x80) {
    bytesToRemove++;
  }

  str.remove(len - bytesToRemove, bytesToRemove);
}

void CardputerUITask::loop() {
  _board->getSpeaker()->processQueue();

  auto event = _board->getKeyboard()->poll();

  if (event.changed && event.down && !turnDisplayOn()) {
    playSound(SoundType::Keyboard);

    if (current_screen) {
      current_screen->handleInput(event);
    }
  }

  if (current_screen) {
    current_screen->poll();
  }

  if (_display != NULL && _display->isOn()) {
    if (millis() >= next_refresh && current_screen) {
      _display->startFrame();
      int delay_millis = current_screen->render(*_display);
      if (millis() < alert_expiry) { // render alert popup
        _display->setTextSize(1);
        int y = _display->height() / 3;
        int p = _display->height() / 32;
        _display->setColor(DisplayDriver::DARK);
        _display->fillRect(p, y, _display->width() - p * 2, y);
        _display->setColor(DisplayDriver::LIGHT); // draw box border
        _display->drawRect(p, y, _display->width() - p * 2, y);
        _display->drawTextCentered(_display->width() / 2, y + p * 3, alert_text);
        next_refresh = alert_expiry; // will need refresh when alert is dismissed
      } else {
        next_refresh = millis() + delay_millis;
      }
      _display->endFrame();
    }
#if AUTO_OFF_MILLIS > 0
    if (millis() > auto_off_time) {
      _display->turnOff();
      _board->getSpeaker()->sleep();
    }
#endif
  }

#ifdef AUTO_SHUTDOWN_MILLIVOLTS
  if (millis() > next_batt_chck) {
    uint16_t milliVolts = getBattMilliVolts();
    if (milliVolts > 0 && milliVolts < AUTO_SHUTDOWN_MILLIVOLTS) {
      shutdown();
    }
    next_batt_chck = millis() + 8000;
  }
#endif
}

bool CardputerUITask::turnDisplayOn() {
  if (_display != NULL) {
    auto_off_time = millis() + AUTO_OFF_MILLIS; // extend auto-off timer
    next_refresh = 0;                           // trigger refresh

    if (!_display->isOn()) {
      _display->turnOn();
      return true;
    }
  }
  return false;
}

bool CardputerUITask::getGPSState() {
  if (_sensors != NULL) {
    int num = _sensors->getNumSettings();
    for (int i = 0; i < num; i++) {
      if (strcmp(_sensors->getSettingName(i), "gps") == 0) {
        return !strcmp(_sensors->getSettingValue(i), "1");
      }
    }
  }
  return false;
}

void CardputerUITask::toggleGPS() {
  if (_sensors != NULL) {
    int num = _sensors->getNumSettings();
    for (int i = 0; i < num; i++) {
      if (strcmp(_sensors->getSettingName(i), "gps") == 0) {
        if (strcmp(_sensors->getSettingValue(i), "1") == 0) {
          _sensors->setSettingValue("gps", "0");
          _node_prefs->gps_enabled = 0;
        } else {
          _sensors->setSettingValue("gps", "1");
          _node_prefs->gps_enabled = 1;
        }
        the_mesh_cp.savePrefs();
        next_refresh = 0;
        break;
      }
    }
  }
}

void CardputerUITask::toggleBuzzer() {
  _node_prefs->buzzer_quiet = !_node_prefs->buzzer_quiet;
  the_mesh_cp.savePrefs();
}

void CardputerUITask::togglePowerSave() {
  _custom_prefs->power_save = !_custom_prefs->power_save;
  the_mesh_cp.savePrefs();
}
