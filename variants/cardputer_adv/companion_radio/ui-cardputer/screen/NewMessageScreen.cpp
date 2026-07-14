#include "NewMessageScreen.h"

void NewMessageScreen::newMessage(const char *from, const char *msg) {
  StrHelper::strncpy(origin, from, sizeof(origin));
  StrHelper::strncpy(message, msg, sizeof(message));
  dismiss_after = millis() + NEW_MESSAGE_DISPLAY_MILLIS;
}

int NewMessageScreen::render(DisplayDriver &display) {
  display.setTextSize(1);

  // origin
  display.setColor(DisplayDriver::YELLOW);
  display.drawTextLeftAlign(0, 0, origin);

  // message
  display.setCursor(0, UI_TEXT_LINE_HEIGHT);
  display.setColor(DisplayDriver::LIGHT);
  display.printWordWrap(message, display.width());

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
