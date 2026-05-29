#include "CardputerDisplay.h"

bool CardputerDisplay::begin() {
  LCD.setBaseColor(TFT_BLACK);

  bool success = LCD.begin();

  if (!success) {
    return false;
  }

  LCD.setTextColor(TFT_WHITE, TFT_BLACK);
  return true;
}

void CardputerDisplay::turnOn() {
  _isOn = true;
  LCD.wakeup();
}

void CardputerDisplay::turnOff() {
  _isOn = false;
  LCD.sleep();
}

void CardputerDisplay::clear() {
  LCD.clear();
}

void CardputerDisplay::startFrame(Color bkg) {
  LCD.startWrite();
  LCD.clear(convertColor(bkg));
}

void CardputerDisplay::endFrame() {
  LCD.endWrite();
}

void CardputerDisplay::setTextSize(int sz) {
  LCD.setTextSize(sz);
}

void CardputerDisplay::setColor(Color c) {
  _lastColor = convertColor(c);
  LCD.setColor(_lastColor);
  LCD.setTextColor(_lastColor);
}

void CardputerDisplay::setCursor(int x, int y) {
  LCD.setCursor(x, y);
}

void CardputerDisplay::print(const char *str) {
  LCD.print(str);
}

void CardputerDisplay::fillRect(int x, int y, int w, int h) {
  LCD.fillRect(x, y, w, h);
}

void CardputerDisplay::drawRect(int x, int y, int w, int h) {
  LCD.drawRect(x, y, w, h);
}

void CardputerDisplay::drawXbm(int x, int y, const uint8_t *bits, int w, int h) {
  LCD.drawBitmap(x, y, bits, w, h, _lastColor);
}

uint16_t CardputerDisplay::getTextWidth(const char *str) {
  return LCD.textWidth(str);
}
