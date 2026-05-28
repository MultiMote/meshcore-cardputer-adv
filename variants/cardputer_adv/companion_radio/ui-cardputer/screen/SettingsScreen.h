#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"


#ifndef UI_SETTINGS_LIST_SIZE
  #define UI_SETTINGS_LIST_SIZE 11
#endif

class SettingsScreen : public UIScreen {
  enum SettingsItem {
    HdrRadio,
    RadioFreq,
    RadioBw,
    RadioSf,
    RadioCr,
    RadioPwr,
    HdrDevice,
    DeviceBeep,
    DeviceBluetooth,
    DeviceGps,
    DevicePowersave,
    DevicePrefixSize,
    Count
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  NodePrefs *_node_prefs;
  int list_sel_idx = 0;
  bool is_editing = false;
  bool restart_required = false;
  String edit_buffer;
  uint8_t edit_u8;

  void renderItem(DisplayDriver &display, SettingsItem item, int x, int y);
  bool enterItemEdit(SettingsItem item);
  void cancelItemEdit(SettingsItem item);
  bool commitItemEdit(SettingsItem item);
  void handleEditInput(SettingsItem item, char key);
  bool inputParsePositiveFloat(float &val);

public:
  SettingsScreen(CardputerUITask *task, mesh::RTCClock *rtc, NodePrefs *node_prefs)
      : _task(task), _rtc(rtc), _node_prefs(node_prefs) {}
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
};
