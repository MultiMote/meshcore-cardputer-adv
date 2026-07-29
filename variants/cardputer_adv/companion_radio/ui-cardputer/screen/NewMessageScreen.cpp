#include "NewMessageScreen.h"

void NewMessageScreen::newMessage(const char *from, const char *msg) {
  StrHelper::strncpy(origin, from, sizeof(origin));
  StrHelper::strncpy(message, msg, sizeof(message));
  dismiss_after = millis() + NEW_MESSAGE_DISPLAY_MILLIS;
}

int NewMessageScreen::render(CardputerDisplay &lcd) {
  lcd.setTextSize(1);

  // origin
  lcd.setColor(DisplayDriver::YELLOW);
  lcd.drawTextLeftAlign(0, 0, origin);

  // message
  lcd.setCursor(0, UI_TEXT_LINE_HEIGHT);
  lcd.setColor(DisplayDriver::LIGHT);
  lcd.printWordWrap(message, lcd.width());

  return NEW_MESSAGE_DISPLAY_MILLIS;
}

bool NewMessageScreen::handleInput(Keyboard::Event &e) {
  // any key
  dismiss_after = 0;
  _task->gotoMainScreen();
  return true;
}

void NewMessageScreen::poll() {
  if (millis() >= dismiss_after) {
    _task->gotoMainScreen();
  }
}
