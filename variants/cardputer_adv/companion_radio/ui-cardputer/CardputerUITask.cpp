#include "CardputerUITask.h"

#include "../CardputerMesh.h"
#include "KeyboardLayout.h"
#include "screen/MainScreen.h"
#include "screen/MsgPreviewScreen.h"
#include "screen/SettingsScreen.h"
#include "screen/SplashScreen.h"
#include "screen/ToolsScreen.h"
#include "target.h"

#include <M5Cardputer.h>
#include <helpers/TxtDataHelpers.h>

void CardputerUITask::begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs,
                            CustomNodePrefs *custom_prefs, KeyboardLayout *keyboard_layout) {
  _display = display;
  _sensors = sensors;
  _keyboard_layout = keyboard_layout;
  auto_off_time = millis() + AUTO_OFF_MILLIS;

#if defined(PIN_USER_BTN)
  user_btn.begin();
#endif

  _node_prefs = node_prefs;
  _custom_prefs = custom_prefs;

  if (_display != NULL) {
    _display->turnOn();
  }

  ui_started_at = millis();

  splash_screen = new SplashScreen(this);
  main_screen = new MainScreen(this, &rtc_clock, sensors, node_prefs, custom_prefs, keyboard_layout);
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
      M5Cardputer.Speaker.tone(4000, 50);
      break;
    case SoundType::NewMessage:
      M5Cardputer.Speaker.tone(3000, 100);
      break;
    case SoundType::DiscoveryResult:
    case SoundType::MessageAck:
      M5Cardputer.Speaker.tone(800, 70);
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

void CardputerUITask::onAckRecv(uint32_t hash) {
  ((MainScreen *)main_screen)->onAckRecv(hash);
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

void CardputerUITask::setCurrScreen(UIScreen *c) {
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

bool CardputerUITask::isButtonPressed() const {
#ifdef PIN_USER_BTN
  return user_btn.isPressed();
#else
  return false;
#endif
}

void CardputerUITask::loop() {
  M5Cardputer.Keyboard.updateKeyList();
  M5Cardputer.Keyboard.updateKeysState();

  char c = 0;
  if (M5Cardputer.Keyboard.isChange()) {
    if (M5Cardputer.Keyboard.isPressed()) {
      Keyboard_Class::KeysState status = M5Cardputer.Keyboard.keysState();

      if (M5Cardputer.Keyboard.isKeyPressed(',') && !status.alt) { // left
        c = checkDisplayOn(KEY_LEFT);
      } else if ((M5Cardputer.Keyboard.isKeyPressed('/') && !status.alt) || status.tab) { // right
        c = checkDisplayOn(KEY_RIGHT);
      } else if (M5Cardputer.Keyboard.isKeyPressed(';') && !status.alt) { // up
        c = checkDisplayOn(KEY_UP);
      } else if (M5Cardputer.Keyboard.isKeyPressed('.') && !status.alt) { // down
        c = checkDisplayOn(KEY_DOWN);
      } else if (M5Cardputer.Keyboard.isKeyPressed('`') && !status.alt) { // esc
        c = checkDisplayOn(ASCII_CTRL_ESCAPE);
      } else if (M5Cardputer.Keyboard.isKeyPressed(' ') && status.ctrl) { // ctrl+space keyboard layout switch
        c = checkDisplayOn(ASCII_CTRL_SUBST);
      } else if (status.enter) { // enter
        c = checkDisplayOn(ASCII_CTRL_LF);
      } else if (status.del) { // backspace
        c = checkDisplayOn(ASCII_CTRL_BACKSPACE);
      } else if (status.opt) { // opt
        c = checkDisplayOn(ASCII_CTRL_DC1);
      } else if (status.word.size() > 0) {
        c = checkDisplayOn(status.word.at(0));
      }
    }
  } else {
#if defined(PIN_USER_BTN)
    int ev = user_btn.check();
    if (ev == BUTTON_EVENT_CLICK) {
      c = checkDisplayOn(KEY_NEXT);
    } else if (ev == BUTTON_EVENT_LONG_PRESS) {
      c = handleLongPress(ASCII_CTRL_LF);
    } else if (ev == BUTTON_EVENT_DOUBLE_CLICK) {
      c = handleDoubleClick(KEY_PREV);
    } else if (ev == BUTTON_EVENT_TRIPLE_CLICK) {
      c = handleTripleClick(KEY_SELECT);
    }
#endif
  }

  if (c != 0 && current_screen) {
    playSound(SoundType::Keyboard);
    current_screen->handleInput(c);
    auto_off_time = millis() + AUTO_OFF_MILLIS; // extend auto-off timer
    next_refresh = 100;                         // trigger refresh
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

char CardputerUITask::checkDisplayOn(char c) {
  if (_display != NULL) {
    if (!_display->isOn()) {
      _display->turnOn(); // turn display on and consume event
      c = 0;
    }
    auto_off_time = millis() + AUTO_OFF_MILLIS; // extend auto-off timer
    next_refresh = 0;                           // trigger refresh
  }
  return c;
}

char CardputerUITask::handleLongPress(char c) {
  if (millis() - ui_started_at < 8000) { // long press in first 8 seconds since startup -> CLI/rescue
    the_mesh_cp.enterCLIRescue();
    c = 0; // consume event
  }
  return c;
}

char CardputerUITask::handleDoubleClick(char c) {
  checkDisplayOn(c);
  return c;
}

char CardputerUITask::handleTripleClick(char c) {
  checkDisplayOn(c);
  toggleBuzzer();
  c = 0;
  return c;
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
