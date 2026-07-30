#pragma once

#if USE_SD_CARD
  #include <SD.h>
  #include "CardputerDataStore.h"
#endif
#include <LovyanGFX.hpp>
#include <helpers/ui/DisplayDriver.h>


class LGFX : public lgfx::LGFX_Device {
  lgfx::Panel_ST7789 panel_instance_;
  lgfx::Bus_SPI bus_instance_;
  lgfx::Light_PWM light_instance_;

public:
  LGFX();
};


class CardputerDisplay : public DisplayDriver {
private:
  bool _isOn = false;
  uint16_t _lastColor = 0;
  int16_t _fontYAdvance = 1;
  LGFX LCD;

  void updateFontYAdvance();

public:
  enum PaletteColor {
    P_BLACK = TFT_BLACK,
    P_WHITE = TFT_WHITE,
    P_GREEN = TFT_GREEN,
    P_BLUE = TFT_BLUE,
    P_RED = TFT_RED,
    P_ORANGE = TFT_ORANGE,
    P_YELLOW = TFT_YELLOW,
    P_GRAY = TFT_GRAY,
  };

  CardputerDisplay() : DisplayDriver(240, 135) {}
  bool begin();
  void tryLoadUserFont();

  bool isOn() override { return _isOn; };
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(Color bkg = DARK) override;
  void setTextSize(int sz) override;
  void setColor(Color c) override { setColor(P_RED); };
  void setColor(PaletteColor c);
  void setCursor(int x, int y) override;
  void print(const char *str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t *bits, int w, int h) override;
  void endFrame() override;
  uint16_t getTextWidth(const char *str) override;
  int32_t getFontHeight() const;
  int16_t getFontLineHeight() const;
  void drawTextLeftAlignWithScroll(int x, int y, int available_width, const char *text);
};
