#pragma once

#include "CardputerAdvBoard.h"
#include "CardputerDataStore.h"
#include "KeyboardLayout.h"

#include <AbstractUITask.h>
#include <Arduino.h>
#include <MeshCore.h>
#include <helpers/BaseSerialInterface.h>
#include <helpers/ContactInfo.h>
#include <helpers/SensorManager.h>
#include <helpers/sensors/LPPDataHelpers.h>
#include <helpers/ui/DisplayDriver.h>
#include <helpers/ui/UIScreen.h>

#define LONG_PRESS_MILLIS    1200

#define ASCII_CTRL_LF        0x0A // newline (Enter)
#define ASCII_CTRL_BACKSPACE 0x08
#define ASCII_CTRL_ESCAPE    0x1B
#define ASCII_CTRL_DC1       0x11 // We will use it for "OPT" button
#define ASCII_CTRL_SUBST     0x1A // We will use it to switch layout

enum class SoundType {
  NewMessage,
  Keyboard,
  DiscoveryResult,
  MessageAck,
};

class CardputerUITask : public AbstractUITask {
  DisplayDriver *_display;
  SensorManager *_sensors;
  CardputerAdvBoard *_board;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;
  KeyboardLayout *_keyboard_layout;
  unsigned long next_refresh = 0;
  unsigned long auto_off_time;
  bool sleep_enabled = false;
  char alert_text[80];
  unsigned long alert_expiry = 0;
  int unsynced_msg_count;
  unsigned long ui_started_at = 0;
  unsigned long next_batt_chck = 0;

  UIScreen *splash_screen;
  UIScreen *main_screen;
  UIScreen *msg_preview_screen;
  UIScreen *settings_screen;
  UIScreen *tools_screen;
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

  void begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs, KeyboardLayout *keyboard_layout);

  void gotoMainScreen() { setCurrScreen(main_screen); }
  void gotoSettingsScreen() { setCurrScreen(settings_screen); }
  void gotoToolsScreen() { setCurrScreen(tools_screen); }
  void showAlert(const char *text, int duration_millis);
  void dismissAlert();
  bool isAlertActive();
  inline int getMsgCount() const { return unsynced_msg_count; }
  inline bool hasDisplay() const { return _display != NULL; }
  bool isButtonPressed() const;
  inline bool isBuzzerQuiet() { return _node_prefs->buzzer_quiet; }
  void toggleBuzzer();
  void togglePowerSave();
  bool getGPSState();
  void toggleGPS();
  // void notifyBeep();
  inline unsigned long getAutoOffTime() const { return auto_off_time; }
  inline bool powerSaveEnabled() const { return _custom_prefs->power_save; }
  void msgRead(int msgcount) override;
  void newMsg(uint8_t path_len, const char *from_name, const char *text, int msgcount) override;

  void onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  void onContactMessageRecv(const ContactInfo &contact, const char *text);
  void onAckRecv(uint32_t hash);

  void pingRecv(float snr_tx, float snr_rx);
  void discoverRecv(const mesh::Identity &id, float snr);
  void messageRepeatsRecv(uint16_t count);
  void notify(UIEventType t = UIEventType::none) override;
  void playSound(SoundType t);
  void loop() override;
  void shutdown(bool restart = false);
};
