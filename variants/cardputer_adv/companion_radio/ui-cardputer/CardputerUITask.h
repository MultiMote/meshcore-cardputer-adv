#pragma once

#include "CardputerAdvBoard.h"

#include <AbstractUITask.h>
#include <Arduino.h>
#include <MeshCore.h>
#include <NodePrefs.h>
#include <helpers/BaseSerialInterface.h>
#include <helpers/ContactInfo.h>
#include <helpers/SensorManager.h>
#include <helpers/sensors/LPPDataHelpers.h>
#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/UIScreen.h>


#ifndef AUTO_OFF_MILLIS
  #define AUTO_OFF_MILLIS 15000 // 15 seconds
#endif

#define LONG_PRESS_MILLIS    1200

#define ASCII_CTRL_LF        0x0A // newline (Enter)
#define ASCII_CTRL_BACKSPACE 0x08
#define ASCII_CTRL_ESCAPE    0x1B
#define ASCII_CTRL_DC1       0x11 // well, we will use it for "OPT" button

class CardputerUITask : public AbstractUITask {
  DisplayDriver *_display;
  SensorManager *_sensors;
  CardputerAdvBoard *_board;
  NodePrefs *_node_prefs;
  unsigned long next_refresh = 0;
  unsigned long auto_off_time;
  bool sleep_enabled = false;
  char alert_text[80];
  unsigned long alert_expiry;
  int unsynced_msg_count;
  unsigned long ui_started_at = 0;
  unsigned long next_batt_chck = 0;
  uint32_t last_ping_tag = 0;

  UIScreen *splash;
  UIScreen *home;
  UIScreen *msg_preview;
  UIScreen *settings;
  UIScreen *current_screen = nullptr;

  // Button action handlers
  char checkDisplayOn(char c);
  char handleLongPress(char c);
  char handleDoubleClick(char c);
  char handleTripleClick(char c);

  void setCurrScreen(UIScreen *c);

public:
  CardputerUITask(CardputerAdvBoard *board, BaseSerialInterface *serial)
      : AbstractUITask(board, serial), _display(NULL), _sensors(NULL), _board(board) {}
  void begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs);

  void gotoHomeScreen() { setCurrScreen(home); }
  void gotoSettingsScreen() { setCurrScreen(settings); }
  void showAlert(const char *text, int duration_millis);
  int getMsgCount() const { return unsynced_msg_count; }
  bool hasDisplay() const { return _display != NULL; }
  bool isButtonPressed() const;
  bool isBuzzerQuiet() { return _node_prefs->buzzer_quiet; }
  void toggleBuzzer();
  bool getGPSState();
  void toggleGPS();
  void keyboardBeep();
  void notifyBeep();
  unsigned long getAutoOffTime() const { return auto_off_time; }
  bool isSleepEnabled() const { return sleep_enabled; }
  void setSleepEnabled(bool enabled) { sleep_enabled = enabled; }

  void msgRead(int msgcount) override;
  void newMsg(uint8_t path_len, const char *from_name, const char *text, int msgcount) override;
  void pingRecv(uint32_t tag, float snr_tx, float snr_rx);
  void notify(UIEventType t = UIEventType::none) override;
  void loop() override;

  void shutdown(bool restart = false);
  void ping(ContactInfo &contact);
};
