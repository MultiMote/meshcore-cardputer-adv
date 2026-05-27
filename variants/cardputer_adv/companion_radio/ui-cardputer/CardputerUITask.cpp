#include "CardputerUITask.h"

#include "screen/MainScreen.h"
#include "screen/MsgPreviewScreen.h"
#include "screen/SplashScreen.h"
#include "target.h"

#include <M5Cardputer.h>
#include <MyMesh.h>
#include <helpers/TxtDataHelpers.h>

void CardputerUITask::begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs) {
  _display = display;
  _sensors = sensors;
  _auto_off = millis() + AUTO_OFF_MILLIS;

#if defined(PIN_USER_BTN)
  user_btn.begin();
#endif

  _node_prefs = node_prefs;

  if (_display != NULL) {
    _display->turnOn();
  }

  ui_started_at = millis();
  _alert_expiry = 0;

  splash = new SplashScreen(this);
  home = new MainScreen(this, &rtc_clock, sensors, node_prefs);
  msg_preview = new MsgPreviewScreen(this, &rtc_clock);
  setCurrScreen(splash);
}

void CardputerUITask::showAlert(const char *text, int duration_millis) {
  strcpy(_alert, text);
  _alert_expiry = millis() + duration_millis;
}

void CardputerUITask::notify(UIEventType t) {
  switch (t) {
  case UIEventType::contactMessage:
  case UIEventType::channelMessage:
  case UIEventType::roomMessage:
    notifyBeep();
    break;
  case UIEventType::ack:
  case UIEventType::newContactMessage:
  case UIEventType::none:
  default:
    break;
  }
}

void CardputerUITask::msgRead(int msgcount) {
  _msgcount = msgcount;
  if (msgcount == 0) {
    gotoHomeScreen();
  }
}

void CardputerUITask::newMsg(uint8_t path_len, const char *from_name, const char *text, int msgcount) {
  _msgcount = msgcount;

  ((MsgPreviewScreen *)msg_preview)->addPreview(path_len, from_name, text);
  setCurrScreen(msg_preview);

  if (_display != NULL) {
    if (!_display->isOn() && !hasConnection()) {
      _display->turnOn();
    }
    if (_display->isOn()) {
      _auto_off = millis() + AUTO_OFF_MILLIS; // extend the auto-off timer
      _next_refresh = 100;                    // trigger refresh
    }
  }
}

void CardputerUITask::userLedHandler() {
  // todo
}

void CardputerUITask::setCurrScreen(UIScreen *c) {
  curr = c;
  _next_refresh = 100;
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
      } else if (status.enter) { // enter
        c = checkDisplayOn(ASCII_CTRL_LF);
      } else if (status.del) { // backspace
        c = checkDisplayOn(ASCII_CTRL_BACKSPACE);
        // } else if (status.space) { // space
        //   c = checkDisplayOn(' ');
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

  if (c != 0 && curr) {
    keyboardBeep();
    curr->handleInput(c);
    _auto_off = millis() + AUTO_OFF_MILLIS; // extend auto-off timer
    _next_refresh = 100;                    // trigger refresh
  }

  userLedHandler();

  if (curr) curr->poll();

  if (_display != NULL && _display->isOn()) {
    if (millis() >= _next_refresh && curr) {
      _display->startFrame();
      int delay_millis = curr->render(*_display);
      if (millis() < _alert_expiry) { // render alert popup
        _display->setTextSize(1);
        int y = _display->height() / 3;
        int p = _display->height() / 32;
        _display->setColor(DisplayDriver::DARK);
        _display->fillRect(p, y, _display->width() - p * 2, y);
        _display->setColor(DisplayDriver::LIGHT); // draw box border
        _display->drawRect(p, y, _display->width() - p * 2, y);
        _display->drawTextCentered(_display->width() / 2, y + p * 3, _alert);
        _next_refresh = _alert_expiry; // will need refresh when alert is dismissed
      } else {
        _next_refresh = millis() + delay_millis;
      }
      _display->endFrame();
    }
#if AUTO_OFF_MILLIS > 0
    if (millis() > _auto_off) {
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
    _auto_off = millis() + AUTO_OFF_MILLIS; // extend auto-off timer
    _next_refresh = 0;                      // trigger refresh
  }
  return c;
}

char CardputerUITask::handleLongPress(char c) {
  if (millis() - ui_started_at < 8000) { // long press in first 8 seconds since startup -> CLI/rescue
    the_mesh.enterCLIRescue();
    c = 0; // consume event
  }
  return c;
}

char CardputerUITask::handleDoubleClick(char c) {
  MESH_DEBUG_PRINTLN("UITask: double click triggered");
  checkDisplayOn(c);
  return c;
}

char CardputerUITask::handleTripleClick(char c) {
  MESH_DEBUG_PRINTLN("UITask: triple click triggered");
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
    // toggle GPS on/off
    int num = _sensors->getNumSettings();
    for (int i = 0; i < num; i++) {
      if (strcmp(_sensors->getSettingName(i), "gps") == 0) {
        if (strcmp(_sensors->getSettingValue(i), "1") == 0) {
          _sensors->setSettingValue("gps", "0");
          _node_prefs->gps_enabled = 0;
          notify(UIEventType::ack);
        } else {
          _sensors->setSettingValue("gps", "1");
          _node_prefs->gps_enabled = 1;
          notify(UIEventType::ack);
        }
        the_mesh.savePrefs();
        showAlert(_node_prefs->gps_enabled ? "GPS: Enabled" : "GPS: Disabled", 800);
        _next_refresh = 0;
        break;
      }
    }
  }
}

void CardputerUITask::toggleBuzzer() {
  _node_prefs->buzzer_quiet = !_node_prefs->buzzer_quiet;
  the_mesh.savePrefs();
  showAlert(_node_prefs->buzzer_quiet ? "Speaker: OFF" : "Speaker: ON", 800);
  _next_refresh = 0; // trigger refresh
}

void CardputerUITask::keyboardBeep() {
  if (!_node_prefs->buzzer_quiet) {
    M5Cardputer.Speaker.tone(4000, 50);
  }
}
void CardputerUITask::notifyBeep() {
  if (!_node_prefs->buzzer_quiet) {
    M5Cardputer.Speaker.tone(3000, 100);
  }
}
