#include "../CardputerUITask.h"
#include "../icons.h"

#include <MyMesh.h>


#define BOOT_SCREEN_MILLIS 3000 // 3 seconds

class SplashScreen : public UIScreen {
  CardputerUITask *_task;
  unsigned long dismiss_after;
  char _version_info[12];

public:
  SplashScreen(CardputerUITask *task) : _task(task) {
    // strip off dash and commit hash by changing dash to null terminator
    // e.g: v1.2.3-abcdef -> v1.2.3
    const char *ver = FIRMWARE_VERSION;
    const char *dash = strchr(ver, '-');

    int len = dash ? dash - ver : strlen(ver);
    if (len >= sizeof(_version_info)) len = sizeof(_version_info) - 1;
    memcpy(_version_info, ver, len);
    _version_info[len] = 0;

    dismiss_after = millis() + BOOT_SCREEN_MILLIS;
  }

  int render(DisplayDriver &display) override {
    // meshcore logo
    display.setColor(DisplayDriver::BLUE);
    int logoWidth = 128;
    display.drawXbm((display.width() - logoWidth) / 2, 3, meshcore_logo, logoWidth, 13);

    // version info
    display.setColor(DisplayDriver::LIGHT);
    display.setTextSize(2);
    display.drawTextCentered(display.width() / 2, 22, _version_info);

    display.setTextSize(1);
    display.drawTextCentered(display.width() / 2, 42, FIRMWARE_BUILD_DATE);

    display.drawTextCentered(display.width() / 2, 62, "for Cardputer ADV");

    return 1000;
  }

  void poll() override {
    if (millis() >= dismiss_after) {
      _task->gotoHomeScreen();
    }
  }
};
