#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"


class ChannelsPage : public CardputerScreen {
private:
  MainScreen *_p;
  int channel_list_idx = 0;
  int getChannelCount();

public:
  ChannelsPage(MainScreen *parent) : _p(parent) {};
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
};
