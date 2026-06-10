#include "ToolsScreen.h"

int ToolsScreen::render(DisplayDriver &display) {
  char buf[64];

  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  if (page == ToolsPage::MenuPage) {
    display.drawTextCentered(display.width() / 2, 5, "Tools");

    int real_idx = 0;
    int list_page = menu_index / UI_TOOLS_LIST_SIZE;
    int list_idx = menu_index % UI_TOOLS_LIST_SIZE;

    for (int i = 0; i < UI_TOOLS_LIST_SIZE; i++) {
      real_idx = list_page * UI_TOOLS_LIST_SIZE + i;

      if (real_idx >= ToolsMenuItem::Count) {
        break;
      }

      display.setColor(DisplayDriver::YELLOW);
      display.drawTextLeftAlign(15, 20 + i * UI_TEXT_LINE_HEIGHT, menu_item_labels[real_idx]);

      if (i == list_idx) {
        display.setColor(DisplayDriver::GREEN);
        display.drawTextLeftAlign(5, 20 + i * UI_TEXT_LINE_HEIGHT, ">");
      }
    }
  } else if (page == ToolsPage::DiscoverPage) {
    display.drawTextCentered(display.width() / 2, 5, "Discover");

    for (int i = 0; i < discovered_repeaters_count; i++) {
      DiscoveredRepeater *rep = &discovered_repeaters[i];

      display.setColor(DisplayDriver::YELLOW);
      display.drawTextLeftAlign(0, 25 + i * UI_TEXT_LINE_HEIGHT, rep->name);

      if (rep->snr > 0) {
        display.setColor(DisplayDriver::GREEN);
      } else {
        display.setColor(DisplayDriver::RED);
      }

      sprintf(buf, "%.2fdb", rep->snr);
      display.drawTextRightAlign(display.width() - 2, 25 + i * UI_TEXT_LINE_HEIGHT, buf);
    }

    return 1000;
  }

  return 5000;
}

void ToolsScreen::menuItemEnter(ToolsMenuItem item) {
  switch (item) {
    case ToolsMenuItem::AdvertFlood:
      if (the_mesh_cp.sendAdvert(true)) {
        _task->showAlert("Flood Advert sent", 3000);
      }

      break;
    case ToolsMenuItem::AdvertZeroHop:
      if (the_mesh_cp.sendAdvert(false)) {
        _task->showAlert("Advert sent", 3000);
      }
      break;

    case ToolsMenuItem::DiscoverRepeaters:
      discovered_repeaters_count = 0;

      if (the_mesh_cp.sendRepeatersDiscover()) {
        page = ToolsPage::DiscoverPage;
        _task->showAlert("Waiting for response...", 10000);
      }

      break;

    default:
      break;
  }
}

bool ToolsScreen::handleInput(char c) {
  if (_task->isAlertActive()) {
    return false;
  }

  if (page == ToolsPage::MenuPage) {
    if (c == ASCII_CTRL_ESCAPE) {
      _task->gotoMainScreen();
      return true;
    }

    if (c == KEY_UP) {
      if (menu_index == 0) {
        menu_index = ToolsMenuItem::Count - 1;
      } else {
        menu_index--;
      }
      return true;
    }

    if (c == KEY_DOWN) {
      if (menu_index < ToolsMenuItem::Count - 1) {
        menu_index++;
      } else {
        menu_index = 0;
      }
      return true;
    }

    if (c == ASCII_CTRL_LF) {
      menuItemEnter(static_cast<ToolsMenuItem>(menu_index));
      return true;
    }
  } else if (page == ToolsPage::DiscoverPage) {
    if (c == ASCII_CTRL_ESCAPE) {
      page = ToolsPage::MenuPage;
      return true;
    }
  }

  return false;
}

void ToolsScreen::discoverRecv(const mesh::Identity &id, float snr) {
  if (page == ToolsPage::DiscoverPage && discovered_repeaters_count < MAX_DISCOVERED_REPEATERS) {
    DiscoveredRepeater rep;
    rep.snr = snr;
    rep.name[0] = '\0';

    ContactInfo *contact =
        the_mesh_cp.lookupContactByPubKey(id.pub_key, CONTACT_LOOKUP_BYTES);

    if (contact) {
      snprintf(rep.name, sizeof(rep.name), "%s", contact->name);
    } else {
      snprintf(rep.name, sizeof(rep.name), "[%02X %02X %02X %02X]", id.pub_key[0], id.pub_key[1],
               id.pub_key[2], id.pub_key[3]);
    }

    discovered_repeaters[discovered_repeaters_count] = rep;
    discovered_repeaters_count++;

    _task->dismissAlert();
    _task->playSound(SoundType::DiscoveryResult);
  }
}
