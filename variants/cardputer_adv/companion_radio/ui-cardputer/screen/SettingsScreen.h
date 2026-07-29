#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"
#include "../icons.h"

class SettingsScreen : public CardputerScreen {
  enum SettingsItem {
    HdrPublicInfo,
    PublicInfoName,
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
    MeshDefaultScope,
    Count
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;
  NodePrefs *_node_prefs;
  CustomNodePrefs *_custom_prefs;

  int menu_index = 0;
  bool is_editing = false;
  bool restart_required = false;
  String edit_buffer;
  uint8_t edit_u8;

  void renderItem(CardputerDisplay &display, SettingsItem item, int x, int y);
  bool enterItemEdit(SettingsItem item);
  void cancelItemEdit(SettingsItem item);
  bool commitItemEdit(SettingsItem item);
  void handleEditInput(SettingsItem item, Keyboard::Event &e);
  bool inputParsePositiveFloat(float &val);

public:
  SettingsScreen(CardputerUITask *task, mesh::RTCClock *rtc, NodePrefs *node_prefs,
                 CustomNodePrefs *custom_prefs, CardputerAdvBoard *board)
      : _task(task), _rtc(rtc), _node_prefs(node_prefs), _custom_prefs(custom_prefs) {}
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
};
