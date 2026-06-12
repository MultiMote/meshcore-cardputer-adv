#include "MsgPreviewScreen.h"

void MsgPreviewScreen::addPreview(uint8_t path_len, const char *from_name, const char *msg) {
  head = (head + 1) % MAX_UNREAD_MSGS;
  if (num_unread < MAX_UNREAD_MSGS) num_unread++;

  auto p = &unread[head];
  p->timestamp = _rtc->getCurrentTime();
  if (path_len == 0xFF) {
    sprintf(p->origin, "(D) %s:", from_name);
  } else {
    sprintf(p->origin, "(%d) %s:", (uint32_t)path_len, from_name);
  }
  StrHelper::strncpy(p->msg, msg, sizeof(p->msg));
}

int MsgPreviewScreen::render(DisplayDriver &display) {
  char tmp[16];
  int y = 0;

  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  sprintf(tmp, "Unread: %d", num_unread);
  display.drawTextLeftAlign(0, 0, tmp);

  auto p = &unread[head];

  int secs = _rtc->getCurrentTime() - p->timestamp;

  if (secs < 60) {
    sprintf(tmp, "%ds", secs);
  } else if (secs < 60 * 60) {
    sprintf(tmp, "%dm", secs / 60);
  } else {
    sprintf(tmp, "%dh", secs / (60 * 60));
  }

  // time
  display.drawTextRightAlign(display.width() - 2, y, tmp);

  y += UI_TEXT_LINE_HEIGHT;

  // sender
  display.drawRect(0, y, display.width(), 1); // horiz line
  display.setColor(DisplayDriver::YELLOW);
  display.drawTextLeftAlign(0, y, p->origin);

  y += UI_TEXT_LINE_HEIGHT;

  // message
  display.setCursor(0, UI_TEXT_LINE_HEIGHT * 2);
  display.setColor(DisplayDriver::LIGHT);
  display.printWordWrap(p->msg, display.width());

#if AUTO_OFF_MILLIS == 0 // probably e-ink
  return 10000;          // 10 s
#else
  return 1000; // next render after 1000 ms
#endif
}

bool MsgPreviewScreen::handleInput(char c) {
  if (c == KEY_NEXT || c == KEY_RIGHT || c == ASCII_CTRL_LF) {
    head = (head + MAX_UNREAD_MSGS - 1) % MAX_UNREAD_MSGS;
    if (num_unread > 0) {
      num_unread--;
    }
    if (num_unread == 0) {
      _task->gotoMainScreen();
    }
    return true;
  }
  if (c == ASCII_CTRL_ESCAPE) {
    num_unread = 0; // clear unread queue
    _task->gotoMainScreen();
    return true;
  }
  return false;
}
