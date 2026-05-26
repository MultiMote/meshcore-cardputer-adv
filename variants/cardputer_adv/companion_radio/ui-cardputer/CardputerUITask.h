#pragma once

#include <MeshCore.h>
#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/UIScreen.h>
#include <helpers/SensorManager.h>
#include <helpers/BaseSerialInterface.h>
#include <Arduino.h>
#include <helpers/sensors/LPPDataHelpers.h>

#ifndef LED_STATE_ON
  #define LED_STATE_ON 1
#endif

#define PRESS_LABEL "long press / Enter"


#include "AbstractUITask.h"
#include "NodePrefs.h"
#include "CardputerAdvBoard.h"

class CardputerUITask : public AbstractUITask {
  DisplayDriver* _display;
  SensorManager* _sensors;
  CardputerAdvBoard* _board;
  unsigned long _next_refresh, _auto_off;
  bool _sleep_enabled;
  NodePrefs* _node_prefs;
  char _alert[80];
  unsigned long _alert_expiry;
  int _msgcount;
  unsigned long ui_started_at, next_batt_chck;

  UIScreen* splash;
  UIScreen* home;
  UIScreen* msg_preview;
  UIScreen* curr;

  void userLedHandler();

  // Button action handlers
  char checkDisplayOn(char c);
  char handleLongPress(char c);
  char handleDoubleClick(char c);
  char handleTripleClick(char c);

  void setCurrScreen(UIScreen* c);

public:

  CardputerUITask(CardputerAdvBoard* board, BaseSerialInterface* serial) : AbstractUITask(board, serial), _display(NULL), _sensors(NULL), _board(board) {
    next_batt_chck = _next_refresh = 0;
    ui_started_at = 0;
    _sleep_enabled = false;
    curr = NULL;
  }
  void begin(DisplayDriver* display, SensorManager* sensors, NodePrefs* node_prefs);

  void gotoHomeScreen() { setCurrScreen(home); }
  void showAlert(const char* text, int duration_millis);
  int  getMsgCount() const { return _msgcount; }
  bool hasDisplay() const { return _display != NULL; }
  bool isButtonPressed() const;

  bool isBuzzerQuiet() {
    return _node_prefs->buzzer_quiet;
  }

  void toggleBuzzer();
  bool getGPSState();
  void toggleGPS();
  void keyboardBeep();
  void notifyBeep();
  unsigned long getAutoOffTime() const { return _auto_off; }
  bool isSleepEnabled() const { return _sleep_enabled; }
  void setSleepEnabled(bool enabled) { _sleep_enabled = enabled; }


  // from AbstractUITask
  void msgRead(int msgcount) override;
  void newMsg(uint8_t path_len, const char* from_name, const char* text, int msgcount) override;
  void notify(UIEventType t = UIEventType::none) override;
  void loop() override;

  void shutdown(bool restart = false);
};
