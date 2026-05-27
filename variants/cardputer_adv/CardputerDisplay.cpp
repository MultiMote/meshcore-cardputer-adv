#include "CardputerDisplay.h"

bool CardputerDisplay::begin() {
  M5Cardputer.Display.setBaseColor(TFT_BLACK);

  bool success = M5Cardputer.Display.begin();

  if (!success) {
    return false;
  }

  canvas.createSprite(M5Cardputer.Display.width(), M5Cardputer.Display.height());
  canvas.setTextColor(TFT_WHITE, TFT_BLACK);
  canvas.setBaseColor(TFT_BLACK);

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
  canvas.clear();
}

void CardputerDisplay::startFrame(Color bkg) {
  canvas.clear(convertColor(bkg));
}

void CardputerDisplay::endFrame() {
  canvas.pushSprite(0, 0);
}

void CardputerDisplay::setTextSize(int sz) {
  canvas.setTextSize(sz);
}

void CardputerDisplay::setColor(Color c) {
  _lastColor = convertColor(c);
  canvas.setColor(_lastColor);
  canvas.setTextColor(_lastColor);
}

void CardputerDisplay::setCursor(int x, int y) {
  canvas.setCursor(x, y);
}

void CardputerDisplay::print(const char *str) {
  canvas.print(str);
}

void CardputerDisplay::fillRect(int x, int y, int w, int h) {
  canvas.fillRect(x, y, w, h);
}

void CardputerDisplay::drawRect(int x, int y, int w, int h) {
  canvas.drawRect(x, y, w, h);
}

void CardputerDisplay::drawXbm(int x, int y, const uint8_t *bits, int w, int h) {
  canvas.drawBitmap(x, y, bits, w, h, _lastColor);
}

uint16_t CardputerDisplay::getTextWidth(const char *str) {
  return canvas.textWidth(str);
}
