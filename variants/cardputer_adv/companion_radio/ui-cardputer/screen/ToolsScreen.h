#pragma once

#include "../../CardputerMesh.h"
#include "../CardputerUITask.h"

class ToolsScreen : public UIScreen {
  enum ToolsPage { MenuPage, DiscoverPage };
  enum ToolsMenuItem { AdvertZeroHop, AdvertFlood, DiscoverRepeaters, Count };

  struct DiscoveredRepeater {
    char name[25];
    float snr;
  };

  const char *menu_item_labels[ToolsMenuItem::Count] = {
    "Advert (Zero Hop)",
    "Advert (Flood)",
    "Discover repeaters",
  };

  CardputerUITask *_task;
  mesh::RTCClock *_rtc;

  ToolsPage page = ToolsPage::MenuPage;

  int menu_index = 0;

  DiscoveredRepeater discovered_repeaters[MAX_DISCOVERED_REPEATERS];
  uint16_t discovered_repeaters_count = 0;

public:
  ToolsScreen(CardputerUITask *task, mesh::RTCClock *rtc) : _task(task), _rtc(rtc) {}
  int render(DisplayDriver &display) override;
  void menuItemEnter(ToolsMenuItem item);
  bool handleInput(char c) override;
  void discoverRecv(const mesh::Identity &id, float snr);

};
