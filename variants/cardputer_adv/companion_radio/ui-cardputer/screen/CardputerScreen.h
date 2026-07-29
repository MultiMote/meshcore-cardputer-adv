#pragma once

#pragma once

#include "hw/CardputerKeyboard.h"
#include "hw/CardputerDisplay.h"

class CardputerScreen {
protected:
  CardputerScreen() {}

public:
  virtual int render(CardputerDisplay &lcd) = 0; // return value is number of millis until next render
  virtual bool handleInput(Keyboard::Event &e) { return false; }
  virtual void poll() {}
};
