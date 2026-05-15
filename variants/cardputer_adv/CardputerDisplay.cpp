#include "CardputerDisplay.h"
#include <M5Cardputer.h>

bool CardputerDisplay::begin() {
  bool success = M5Cardputer.Display.begin();

  if (!success) {
    return false;
  }

  M5Cardputer.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5Cardputer.Display.fillScreen(TFT_BLACK);
  return true;
}

void CardputerDisplay::turnOn() {
  _isOn = true;
  M5Cardputer.Display.wakeup();
}

void CardputerDisplay::turnOff() {
  _isOn = false;
  M5Cardputer.Display.sleep();
}

void CardputerDisplay::clear() {
  M5Cardputer.Display.clear();
}

void CardputerDisplay::startFrame(Color bkg) {
  clear();
}

void CardputerDisplay::endFrame() {
  M5Cardputer.Display.display();
}

void CardputerDisplay::setTextSize(int sz) {
  M5Cardputer.Display.setTextSize(sz);
}

void CardputerDisplay::setColor(Color c) {
  switch (c) {
  case DARK:
    _color = TFT_BLACK;
    break;
  case LIGHT:
    _color = TFT_WHITE;
    break;
  case RED:
    _color = TFT_RED;
    break;
  case GREEN:
    _color = TFT_GREEN;
    break;
  case BLUE:
    _color = TFT_BLUE;
    break;
  case YELLOW:
    _color = TFT_YELLOW;
    break;
  case ORANGE:
    _color = TFT_ORANGE;
    break;
  default:
    _color = TFT_WHITE;
  }
  M5.Display.setTextColor(_color);
}

void CardputerDisplay::setCursor(int x, int y) {
  M5Cardputer.Display.setCursor(x, y);
}

void CardputerDisplay::print(const char *str) {
  M5Cardputer.Display.print(str);
}

void CardputerDisplay::fillRect(int x, int y, int w, int h) {
  M5Cardputer.Display.fillRect(x, y, w, h, _color);
}

void CardputerDisplay::drawRect(int x, int y, int w, int h) {
  M5Cardputer.Display.drawRect(x, y, w, h, _color);
}

void CardputerDisplay::drawXbm(int x, int y, const uint8_t *bits, int w, int h) {
  M5Cardputer.Display.drawBitmap(x, y, bits, w, h, _color);
}

uint16_t CardputerDisplay::getTextWidth(const char *str) {
  return M5Cardputer.Display.textWidth(str);
}

