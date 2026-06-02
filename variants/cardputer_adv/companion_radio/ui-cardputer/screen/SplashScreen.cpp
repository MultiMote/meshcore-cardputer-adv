#include "SplashScreen.h"

SplashScreen::SplashScreen(CardputerUITask *task) : _task(task) {
  // strip off dash and commit hash by changing dash to null terminator
  // e.g: v1.2.3-abcdef -> v1.2.3
  const char *ver = FIRMWARE_VERSION;
  const char *dash = strchr(ver, '-');

  int len = dash ? dash - ver : strlen(ver);
  if (len >= sizeof(version_info)) len = sizeof(version_info) - 1;
  memcpy(version_info, ver, len);
  version_info[len] = 0;

  dismiss_after = millis() + BOOT_SCREEN_MILLIS;
}

int SplashScreen::render(DisplayDriver &display) {
  int logoWidth = 128;
  // meshcore logo
  display.setColor(DisplayDriver::BLUE);
  display.drawXbm((display.width() - logoWidth) / 2, 3, meshcore_logo, logoWidth, 13);

  // version info
  display.setColor(DisplayDriver::LIGHT);
  display.setTextSize(2);
  display.drawTextCentered(display.width() / 2, 22, version_info);

  display.setTextSize(1);
  display.drawTextCentered(display.width() / 2, 52, FIRMWARE_BUILD_DATE);

  display.drawTextCentered(display.width() / 2, 72, "for Cardputer ADV");

  return 5000;
}

void SplashScreen::poll() {
  if (millis() >= dismiss_after) {
    _task->gotoMainScreen();
  }
}
