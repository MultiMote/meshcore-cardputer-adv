#include "ContactsPage.h"

#include "helpers.h"

int ContactsPage::getFilteredContactCount() {
  ContactInfo contact;
  int num = 0;

  for (int i = 0; i < the_mesh_cp.getNumContacts(); i++) {
    if (!the_mesh_cp.getContactByIdx(i, contact)) {
      break;
    }

    if (contact_search_box.isEmpty() ||
        Helpers::containsIgnoreCase(contact.name, contact_search_box.c_str())) {
      num++;
    }
  }

  return num;
}

bool ContactsPage::getFilteredContactIndex(int list_idx, int &real_idx) {
  ContactInfo contact;
  int match_count = 0;

  for (int i = 0; i < the_mesh_cp.getNumContacts(); i++) {
    if (!the_mesh_cp.getContactByIdx(i, contact)) {
      break;
    }

    if (contact_search_box.isEmpty() ||
        Helpers::containsIgnoreCase(contact.name, contact_search_box.c_str())) {
      if (match_count == list_idx) {
        real_idx = i;
        return true;
      }
      match_count++;
    }
  }

  return false;
}

int ContactsPage::render(CardputerDisplay &lcd) {
  lcd.setColor(DisplayDriver::GREEN);
  lcd.drawTextCentered(lcd.width() / 2, 20, "Contacts");
  int real_idx = 0;
  int list_page = contact_list_idx / UI_CONTACT_LIST_SIZE;
  int list_idx = contact_list_idx % UI_CONTACT_LIST_SIZE;
  char buf[6];

  for (int i = 0; i < UI_CONTACT_LIST_SIZE; i++) {
    lcd.setColor(DisplayDriver::GREEN);

    ContactInfo contact;
    int display_idx = list_page * UI_CONTACT_LIST_SIZE + i;

    if (!getFilteredContactIndex(display_idx, real_idx)) {
      break;
    }

    the_mesh_cp.getContactByIdx(real_idx, contact);

    int unread_count = 0;

    for (int j = 0; j < _p->getUnread()->countChats(); j++) {
      const UnreadCounterItem *item = _p->getUnread()->get(j);
      if (item && !item->is_channel && memcmp(item->pkey, contact.id.pub_key, CONTACT_LOOKUP_BYTES) == 0) {
        unread_count = item->count;
        break;
      }
    }

    if (i == list_idx) {
      lcd.drawTextLeftAlign(5, 30 + i * UI_TEXT_LINE_HEIGHT, ">");
    }

    int right_pad = 0;

    if (unread_count > 0) {
      snprintf(buf, sizeof(buf), "%d", unread_count);
      right_pad = lcd.getTextWidth(buf) + 5;
      lcd.setColor(DisplayDriver::ORANGE);
      lcd.drawTextRightAlign(lcd.width() - 1, 30 + i * UI_TEXT_LINE_HEIGHT, buf);
    }

    if (contact.type == ADV_TYPE_CHAT) {
      lcd.drawTextLeftAlign(15, 30 + i * UI_TEXT_LINE_HEIGHT, "C");
    }

    lcd.drawTextEllipsized(25, 30 + i * UI_TEXT_LINE_HEIGHT, lcd.width() - 25 - right_pad, contact.name);
  }

  lcd.drawRect(1, lcd.height() - UI_TEXT_LINE_HEIGHT - 4, lcd.width() - 1, 1);
  if (contact_search_box.isEmpty()) {
    lcd.setColor(TFT_GRAY);
    lcd.drawTextLeftAlign(5, lcd.height() - UI_TEXT_LINE_HEIGHT - 4, "Search");
  } else {
    lcd.setColor(DisplayDriver::LIGHT);
    lcd.drawTextLeftAlignWithScroll(5, lcd.height() - UI_TEXT_LINE_HEIGHT - 4, lcd.width() - 10,
                                    contact_search_box.c_str());

  }
  return 5000;
}

bool ContactsPage::handleInput(Keyboard::Event &e) {
  CardputerLayout *lay = _p->getUiTask()->getBoard()->getLayout();

  if (e.key == Keyboard::KEY_BACKSPACE) {
    if (contact_search_box.length() > 0) {
      Helpers::removeLastStringChar(contact_search_box);
      contact_list_idx = 0;
    }
    return true;
  }

  if (e.key == Keyboard::ARROW_UP) {
    if (contact_list_idx == 0) {
      contact_list_idx = std::max(getFilteredContactCount() - 1, 0);
    } else {
      contact_list_idx--;
    }
    return true;
  }

  if (e.key == Keyboard::ARROW_DOWN) {
    if (contact_list_idx < getFilteredContactCount() - 1) {
      contact_list_idx++;
    } else {
      contact_list_idx = 0;
    }
    return true;
  }

  if (e.key == Keyboard::KEY_RETURN) {
    int real_idx = -1;
    if (getFilteredContactIndex(contact_list_idx, real_idx)) {
      ContactInfo c;
      if (the_mesh_cp.getContactByIdx(real_idx, c)) {
        if (c.type == ADV_TYPE_CHAT) {
          _p->selectContact(c, real_idx);
        } else if ((c.type == ADV_TYPE_REPEATER || c.type == ADV_TYPE_ROOM) &&
                   !_p->getUiTask()->isAlertActive()) {
          the_mesh_cp.sendPing(c);
          _p->getUiTask()->showAlert("Waiting for response...", 4000);
        }
      }
    }
    return true;
  }

  if (e.key == Keyboard::KEY_R) { // reset path
    int real_idx = -1;
    if (getFilteredContactIndex(contact_list_idx, real_idx)) {
      ContactInfo c;
      if (the_mesh_cp.getContactByIdx(real_idx, c)) {
        if (c.type == ADV_TYPE_CHAT) {
          // getContactByIdx does not return a reference
          ContactInfo *ref = the_mesh_cp.lookupContactByPubKey(c.id.pub_key, CONTACT_LOOKUP_BYTES);
          ref->out_path_len = OUT_PATH_UNKNOWN;
          _p->getUiTask()->showAlert("Path cleared", 1000);
          _p->refreshSelectedContact();
        }
      }
    }
    return true;
  }

  bool skip_input = e.key == Keyboard::KEY_ESC || e.key == Keyboard::ARROW_LEFT ||
                    e.key == Keyboard::ARROW_RIGHT || e.key == Keyboard::KEY_TAB;

  if (!skip_input) {
    const char *repl = lay->lookup(e);
    if (repl[0]) {
      if (contact_search_box.length() + strlen(repl) <= UI_CHANNEL_SEARCH_MAX_CHARS) {
        contact_search_box += repl;
      }
      contact_list_idx = 0;
      return true;
    }
  }

  return false;
}
