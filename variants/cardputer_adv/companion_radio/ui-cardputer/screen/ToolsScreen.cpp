#include "ToolsScreen.h"

int ToolsScreen::render(DisplayDriver &display) {
  if (page == ToolsPage::MenuPage) {
    display.setTextSize(1);
    display.setColor(DisplayDriver::GREEN);
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

    default:
      break;
  }
}

bool ToolsScreen::handleInput(char c) {
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

    if (c == ASCII_CTRL_LF && !_task->isAlertActive()) {
      menuItemEnter(static_cast<ToolsMenuItem>(menu_index));
      return true;
    }
  }

  return false;
}
