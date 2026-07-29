#pragma once

#include "../CardputerScreen.h"
#include "../MainScreen.h"
#include "companion_radio/CardputerMesh.h"

class ContactsPage : public CardputerScreen {
private:
  MainScreen *_p;
  int contact_list_idx = 0;

  String contact_search_box;

  int getFilteredContactCount();
  bool getFilteredContactIndex(int list_idx, int &real_idx);

public:
  ContactsPage(MainScreen *parent) : _p(parent) {};
  int render(CardputerDisplay &lcd) override;
  bool handleInput(Keyboard::Event &e) override;
};
