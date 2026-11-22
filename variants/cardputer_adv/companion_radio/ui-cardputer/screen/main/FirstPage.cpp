#include "FirstPage.h"
#include <WiFi.h>

int FirstPage::render(CardputerDisplay &lcd) {
  char tmp[80];

  if (WiFi.getMode() == WIFI_STA) {
    lcd.setTextSize(1);
    if (WiFi.status() == WL_CONNECTED) {
      lcd.setColor(CardputerDisplay::P_GREEN);
      snprintf(tmp, sizeof(tmp), "%s:%d", WiFi.localIP().toString().c_str(), TCP_PORT);
      lcd.drawTextCentered(lcd.width() / 2, 43, tmp);
      lcd.drawTextCentered(lcd.width() / 2, 43 + UI_TEXT_LINE_HEIGHT,
                           _p->getUiTask()->hasConnection() ? "< Connected >" : "< Wi-Fi ready >");
    } else {
      lcd.setColor(CardputerDisplay::P_RED);
      lcd.drawTextCentered(lcd.width() / 2, 43, "Wi-Fi connecting...");
    }
  } else if (_p->getUiTask()->hasConnection()) {
    lcd.setColor(CardputerDisplay::P_GREEN);
    lcd.setTextSize(1);
    lcd.drawTextCentered(lcd.width() / 2, 43, "< Connected >");
  } else if (the_mesh_cp.getBLEPin() != 0) { // BT pin
    lcd.setColor(CardputerDisplay::P_RED);
    lcd.setTextSize(2);
    sprintf(tmp, "Pin: %d", the_mesh_cp.getBLEPin());
    lcd.drawTextCentered(lcd.width() / 2, 43, tmp);
  }

  lcd.setTextSize(1);

  if (_p->getUnread()->countChats() > 0) {
    lcd.setColor(CardputerDisplay::P_YELLOW);
    sprintf(tmp, "Unread: [chats: %u, msgs: %u]", _p->getUnread()->countChats(),
            _p->getUnread()->countMessages());
    lcd.drawTextCentered(lcd.width() / 2, lcd.height() - lcd.getFontLineHeight() * 3, tmp);
  }

  lcd.setColor(CardputerDisplay::P_GREEN);
  lcd.drawTextCentered(lcd.width() / 2, lcd.height() - lcd.getFontLineHeight() * 2,
                       "Press OPT to open Settings");

  lcd.setColor(CardputerDisplay::P_ORANGE);
  lcd.drawTextCentered(lcd.width() / 2, lcd.height() - lcd.getFontLineHeight(), "Press T to open Tools");

  return 5000;
}

bool FirstPage::handleInput(Keyboard::Event &e) {
  if (e.modifiers.opt) {
    _p->getUiTask()->gotoSettingsScreen();
    return true;
  }

  if (e.key == Keyboard::KEY_T) {
    _p->getUiTask()->gotoToolsScreen();
    return true;
  }

  return false;
}
