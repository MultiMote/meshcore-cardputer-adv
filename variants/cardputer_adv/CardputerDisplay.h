#pragma once

#include <helpers/ui/DisplayDriver.h>

class CardputerDisplay : public DisplayDriver {
private:
  bool _isOn;
  uint16_t _color;

public:
  CardputerDisplay() : DisplayDriver(240, 135) { _isOn = false; }
  bool begin();

  bool isOn() override { return _isOn; };
  void turnOn() override;
  void turnOff() override;
  void clear() override;
  void startFrame(Color bkg = DARK) override;
  void setTextSize(int sz) override;
  void setColor(Color c) override;
  void setCursor(int x, int y) override;
  void print(const char *str) override;
  void fillRect(int x, int y, int w, int h) override;
  void drawRect(int x, int y, int w, int h) override;
  void drawXbm(int x, int y, const uint8_t *bits, int w, int h) override;
  uint16_t getTextWidth(const char *str);
  void endFrame();
};
