#pragma once

#pragma once

#include "hw/CardputerKeyboard.h"

#include <helpers/ui/DisplayDriver.h>


class CardputerScreen {
protected:
  CardputerScreen() {}

public:
  virtual int render(DisplayDriver &display) = 0; // return value is number of millis until next render
  virtual bool handleInput(Keyboard::Event &e) { return false; }
  virtual void poll() {}
};
