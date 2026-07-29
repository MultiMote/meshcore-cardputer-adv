#include "StatsPage.h"

int StatsPage::render(CardputerDisplay &lcd) {
  char tmp[80];

  int y = 20;
  display.drawTextCentered(display.width() / 2, y, "Stats");

  display.setColor(DisplayDriver::YELLOW);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Radio noise floor: %d", radio_driver.getNoiseFloor());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Heap usage: %d/%d (%d%%)", ESP.getFreeHeap(), ESP.getHeapSize(),
          (ESP.getHeapSize() - ESP.getFreeHeap()) * 100 / ESP.getHeapSize());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Packets received: %u", the_mesh_cp.receivedPacketsCount());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Battery: %umV", _p->getUiTask()->getBattMilliVoltsCorrected());
  display.drawTextLeftAlign(5, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;
  sprintf(tmp, "Unsynced messages: %d", _p->getUiTask()->getMsgCount());
  display.drawTextLeftAlign(5, y, tmp);

  return 5000;
}
