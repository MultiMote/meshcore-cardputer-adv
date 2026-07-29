#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"

class StatsPage : public CardputerScreen {
private:
  MainScreen *_p;

public:
  StatsPage(MainScreen *parent) : _p(parent) {};
  int render(CardputerDisplay &lcd) override;
};
