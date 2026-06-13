#include "CardputerDisplay.h"

#include "font.h"
#include "globals.h"

#include <MeshCore.h>

int32_t emoji_draw_callback(lgfx::LGFXBase *gfx, int32_t x, int32_t y, uint32_t code, int32_t font_height) {
  int w = gfx->textWidth("O");
  int h = gfx->fontHeight();
  gfx->drawRect(x, y + (h - w - w / 2), w, w, TFT_DARKGRAY);
  return w;
}

void CardputerDisplay::updateFontYAdvance() {
  lgfx::FontMetrics metrics;
  LCD.getFont()->getDefaultMetric(&metrics);
  _fontYAdvance = metrics.y_advance;
}

bool CardputerDisplay::begin() {
  LCD.setBaseColor(TFT_BLACK);

  bool success = LCD.begin();

  if (!success) {
    return false;
  }

  LCD.setTextColor(TFT_WHITE, TFT_BLACK);
  LCD.loadFont(DejaVuSans_11);
  LCD.setEmojiCallback(emoji_draw_callback);

  updateFontYAdvance();
  return true;
}

void CardputerDisplay::tryLoadUserFont() {
#if USE_SD_CARD
  // fixme: Do not use, redraw is incredibly slow for some reason
  if (SD.exists(USER_FONT_NAME)) {
    MESH_DEBUG_PRINTLN("User font found");
    if (LCD.loadFont(SD, USER_FONT_NAME)) {
      MESH_DEBUG_PRINTLN("%s loaded", USER_FONT_NAME);
      updateFontYAdvance();
    } else {
      MESH_DEBUG_PRINTLN("%s load failed", USER_FONT_NAME);
    }
  }
#endif
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

int32_t CardputerDisplay::getFontHeight() const {
  return LCD.fontHeight();
}

int16_t CardputerDisplay::getFontYAdvance() const {
  return _fontYAdvance;
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
