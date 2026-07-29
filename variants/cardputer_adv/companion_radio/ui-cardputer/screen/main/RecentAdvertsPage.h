#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"

class RecentAdvertsPage : public CardputerScreen {
private:
  MainScreen *_p;
  mesh::RTCClock *_rtc;

public:
  RecentAdvertsPage(MainScreen *parent, mesh::RTCClock *rtc) : _p(parent), _rtc(rtc) {};
  int render(CardputerDisplay &lcd) override;
};
