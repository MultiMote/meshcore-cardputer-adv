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

  inline uint16_t convertColor(Color c) {
    {
      switch (c) {
      case DARK:
        return TFT_BLACK;
      case LIGHT:
        return TFT_WHITE;
      case RED:
        return TFT_RED;
      case GREEN:
        return TFT_GREEN;
      case BLUE:
        return TFT_BLUE;
      case YELLOW:
        return TFT_YELLOW;
      case ORANGE:
        return TFT_ORANGE;
      default:
        return TFT_WHITE;
      }
    }
  }

  void updateFontYAdvance();


public:
  CardputerDisplay() : DisplayDriver(240, 135) {}
  bool begin();
  void tryLoadUserFont();

  bool isOn() override { return _isOn; };
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(Color bkg = DARK) override;
  void setTextSize(int sz) override;
  void setColor(Color c) override;
  void setColor(uint16_t c);
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
