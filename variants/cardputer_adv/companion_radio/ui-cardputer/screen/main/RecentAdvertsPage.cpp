#include "RecentAdvertsPage.h"

int RecentAdvertsPage::render(CardputerDisplay &lcd) {
  char tmp[80];
  AdvertPath recent[UI_RECENT_LIST_SIZE];

  the_mesh_cp.getRecentlyHeard(recent, UI_RECENT_LIST_SIZE);
  lcd.setColor(CardputerDisplay::P_GREEN);

  int y = 20;
  lcd.drawTextCentered(lcd.width() / 2, y, "Recent adverts");

  y += UI_TEXT_LINE_HEIGHT;

  for (int i = 0; i < UI_RECENT_LIST_SIZE; i++, y += UI_TEXT_LINE_HEIGHT) {
    auto a = &recent[i];
    if (a->name[0] == 0) continue; // empty slot
    int secs = _rtc->getCurrentTime() - a->recv_timestamp;
    if (secs < 60) {
      sprintf(tmp, "%ds", secs);
    } else if (secs < 60 * 60) {
      sprintf(tmp, "%dm", secs / 60);
    } else {
      sprintf(tmp, "%dh", secs / (60 * 60));
    }

    int timestamp_width = lcd.getTextWidth(tmp);
    int max_name_width = lcd.width() - timestamp_width - 1;

    lcd.drawTextEllipsized(0, y, max_name_width, a->name);
    lcd.setCursor(lcd.width() - timestamp_width - 1, y);
    lcd.print(tmp);
  }

  return 5000;
}
