#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"
#include "CardputerAdvBoard.h"

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
    DeviceBatteryCorrection,
    HdrMesh,
    MeshPathSize,
    Count
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;
  CardputerAdvBoard *_board;

  int menu_index = 0;
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
  SettingsScreen(CardputerUITask *task, mesh::RTCClock *rtc, NodePrefs *node_prefs,
                 CustomNodePrefs *custom_prefs, CardputerAdvBoard *board)
      : _task(task), _rtc(rtc), _node_prefs(node_prefs), _custom_prefs(custom_prefs), _board(board) {}
  int render(DisplayDriver &display) override;
  bool handleInput(char c) override;
};
