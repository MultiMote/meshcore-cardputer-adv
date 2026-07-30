#include "ToolsScreen.h"

int ToolsScreen::render(CardputerDisplay &lcd) {
  char buf[64];

  lcd.setTextSize(1);
  lcd.setColor(CardputerDisplay::P_GREEN);
  if (page == ToolsPage::MenuPage) {
    lcd.drawTextCentered(lcd.width() / 2, 5, "Tools");

    int real_idx = 0;
    int list_page = menu_index / UI_TOOLS_LIST_SIZE;
    int list_idx = menu_index % UI_TOOLS_LIST_SIZE;

    for (int i = 0; i < UI_TOOLS_LIST_SIZE; i++) {
      real_idx = list_page * UI_TOOLS_LIST_SIZE + i;

      if (real_idx >= ToolsMenuItem::Count) {
        break;
      }

      const char *label = menu_item_labels[real_idx];
      lcd.setColor(label[0] == '-' ? CardputerDisplay::P_GREEN : CardputerDisplay::P_YELLOW);
      lcd.drawTextLeftAlign(15, 20 + i * UI_TEXT_LINE_HEIGHT, label);

      if (i == list_idx) {
        lcd.setColor(CardputerDisplay::P_GREEN);
        lcd.drawTextLeftAlign(5, 20 + i * UI_TEXT_LINE_HEIGHT, ">");
      }
    }
  } else if (page == ToolsPage::DiscoverPage) {
    lcd.drawTextCentered(lcd.width() / 2, 5, "Discover");

    for (int i = 0; i < discovered_repeaters_count; i++) {
      DiscoveredRepeater *rep = &discovered_repeaters[i];

      lcd.setColor(CardputerDisplay::P_YELLOW);
      lcd.drawTextLeftAlign(0, 25 + i * UI_TEXT_LINE_HEIGHT, rep->name);

      if (rep->snr > 0) {
        lcd.setColor(CardputerDisplay::P_GREEN);
      } else {
        lcd.setColor(CardputerDisplay::P_RED);
      }

      sprintf(buf, "%.2fdb", rep->snr);
      lcd.drawTextRightAlign(lcd.width() - 2, 25 + i * UI_TEXT_LINE_HEIGHT, buf);
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

    case ToolsMenuItem::PowerOff: {
      bool done = false;
      // Wait for key release to prevent firing interrupt
      do {
        Keyboard::Event e = _task->getBoard()->getKeyboard()->poll();
        done = (e.changed && e.key == Keyboard::KEY_RETURN && e.down == false);
        delay(10);
      } while (!done);
      _task->shutdown();
    } break;

    case ToolsMenuItem::Restart:
      ESP.restart();
      break;

    default:
      break;
  }
}

bool ToolsScreen::handleInput(Keyboard::Event &e) {
  if (_task->isAlertActive()) {
    return false;
  }

  if (page == ToolsPage::MenuPage) {
    if (e.key == Keyboard::KEY_ESC) {
      _task->gotoMainScreen();
      return true;
    }

    if (e.key == Keyboard::ARROW_UP) {
      if (menu_index == 0) {
        menu_index = ToolsMenuItem::Count - 1;
      } else {
        menu_index--;
      }
      return true;
    }

    if (e.key == Keyboard::ARROW_DOWN) {
      if (menu_index < ToolsMenuItem::Count - 1) {
        menu_index++;
      } else {
        menu_index = 0;
      }
      return true;
    }

    if (e.key == Keyboard::KEY_RETURN) {
      menuItemEnter(static_cast<ToolsMenuItem>(menu_index));
      return true;
    }
  } else if (page == ToolsPage::DiscoverPage) {
    if (e.key == Keyboard::KEY_ESC) {
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

    ContactInfo *contact = the_mesh_cp.lookupContactByPubKey(id.pub_key, CONTACT_LOOKUP_BYTES);

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
