#pragma once

#include "CardputerAdvBoard.h"
#include "CardputerDataStore.h"
#include "screen/CardputerScreen.h"
#include "types.h"

#include <AbstractUITask.h>
#include <Arduino.h>
#include <MeshCore.h>
#include <helpers/MultiSerialInterface.h>
#include <helpers/ContactInfo.h>
#include <helpers/SensorManager.h>
#include <helpers/sensors/LPPDataHelpers.h>

#define LONG_PRESS_MILLIS 1200

class CardputerUITask : public AbstractUITask {
  CardputerDisplay *_display;
  SensorManager *_sensors;
  CardputerAdvBoard *_board;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;
  unsigned long next_refresh = 0;
  unsigned long auto_off_time;
  bool sleep_enabled = false;
  char alert_text[80];
  unsigned long alert_expiry = 0;
  int unsynced_msg_count;
  unsigned long ui_started_at = 0;
  unsigned long next_batt_chck = 0;

  CardputerScreen *splash_screen;
  CardputerScreen *main_screen;
  CardputerScreen *new_message_screen;
  CardputerScreen *settings_screen;
  CardputerScreen *tools_screen;
  CardputerScreen *current_screen = nullptr;

  // Returns false if display is already on
  bool turnDisplayOn();
  void setCurrScreen(CardputerScreen *c);

public:
  CardputerUITask(CardputerAdvBoard *board, MultiSerialInterface *serial)
      : AbstractUITask(board, serial), _display(NULL), _sensors(NULL), _board(board) {}

  void begin(DisplayDriver *display, SensorManager *sensors, NodePrefs *node_prefs,
             CustomNodePrefs *custom_prefs);

  void gotoMainScreen() { setCurrScreen(main_screen); }
  void gotoSettingsScreen() { setCurrScreen(settings_screen); }
  void gotoToolsScreen() { setCurrScreen(tools_screen); }
  void showAlert(const char *text, int duration_millis);
  void dismissAlert();
  bool isAlertActive();
  inline int getMsgCount() const { return unsynced_msg_count; }
  inline bool hasDisplay() const { return _display != NULL; }
  inline bool isBuzzerQuiet() { return _node_prefs->buzzer_quiet; }
  inline const char* getNodeName() { return _node_prefs->node_name; }
  inline uint16_t getBattMilliVoltsCorrected() { return (float)getBattMilliVolts() * _custom_prefs->battery_correction; }
  void toggleBuzzer();
  void togglePowerSave();
  bool getGPSState();
  void toggleGPS();
  inline unsigned long getAutoOffTime() const { return auto_off_time; }
  inline bool powerSaveEnabled() const { return _custom_prefs->power_save; }
  void msgRead(int msgcount) override;
  void newMsg(uint8_t path_len, const char *from_name, const char *text, int msgcount) override;

  void onChannelMessageRecv(const mesh::GroupChannel &channel, const char *text);
  void onContactMessageRecv(const ContactInfo &contact, const char *text);
  void onMessageSendAttempt(uint8_t attempt, uint8_t total, MessageSendState state);

  void pingRecv(float snr_tx, float snr_rx);
  void discoverRecv(const mesh::Identity &id, float snr);
  void messageRepeatsRecv(uint16_t count);
  void notify(UIEventType t = UIEventType::none) override;
  void playSound(SoundType t);
  void loop() override;
  void shutdown(bool restart = false);
  CardputerAdvBoard *getBoard() { return _board; }
  inline void extendAutoOff() { auto_off_time = millis() + AUTO_OFF_MILLIS; }
  inline void scheduleRefresh(unsigned long ms = 0) { next_refresh = millis() + ms; }
};
