#include "ToolsScreen.h"

int ToolsScreen::render(DisplayDriver &display) {
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
    display.setColor(DisplayDriver::YELLOW);
    display.drawTextLeftAlign(0, 25, discover_tmp.c_str());
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
      discover_tmp.clear();
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
  if (page == ToolsPage::DiscoverPage) {
    char buf[32];
    sprintf(buf, "[%02x %02x %02x ...] SNR: %.2fdb\n", id.pub_key[0], id.pub_key[1], id.pub_key[2], snr);
    discover_tmp.concat(buf);
    _task->dismissAlert();
  }
}
