#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"

class GpsPage : public CardputerScreen {
private:
  MainScreen *_p;

public:
  GpsPage(MainScreen *parent) : _p(parent) {};
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
};
