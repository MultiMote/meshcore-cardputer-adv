#include "ChannelsPage.h"

int ChannelsPage::getChannelCount() { // not sure if there is no gaps
  ChannelDetails channel;
  int num = 0;

  for (int i = 0; i < MAX_GROUP_CHANNELS; i++) {
    if (!the_mesh_cp.getChannel(i, channel) || strlen(channel.name) == 0) {
      break;
    }
    num++;
  }

  return num;
}

int ChannelsPage::render(CardputerDisplay &lcd) {
  lcd.setColor(CardputerDisplay::P_GREEN);
  lcd.drawTextCentered(lcd.width() / 2, 20, "Channels");

  int real_idx = 0;
  int list_page = channel_list_idx / UI_CHANNEL_LIST_SIZE;
  int list_idx = channel_list_idx % UI_CHANNEL_LIST_SIZE;
  char buf[6];

  for (int i = 0; i < UI_CHANNEL_LIST_SIZE; i++) {
    lcd.setColor(CardputerDisplay::P_GREEN);

    ChannelDetails channel;
    real_idx = list_page * UI_CHANNEL_LIST_SIZE + i;

    if (!the_mesh_cp.getChannel(real_idx, channel) || strlen(channel.name) == 0) {
      break;
    }

    int unread_count = 0;

    for (int j = 0; j < _p->getUnread()->countChats(); j++) {
      const UnreadCounterItem *item = _p->getUnread()->get(j);
      if (item && item->is_channel && memcmp(item->pkey, channel.channel.secret, CONTACT_LOOKUP_BYTES) == 0) {
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
      lcd.setColor(CardputerDisplay::P_ORANGE);
      lcd.drawTextRightAlign(lcd.width() - 1, 30 + i * UI_TEXT_LINE_HEIGHT, buf);
    }

    lcd.drawTextEllipsized(25, 30 + i * UI_TEXT_LINE_HEIGHT, lcd.width() - 25 - right_pad, channel.name);
  }

  return 5000;
}

bool ChannelsPage::handleInput(Keyboard::Event &e) {
  if (e.key == Keyboard::ARROW_UP) {
    if (channel_list_idx == 0) {
      channel_list_idx = std::max(getChannelCount() - 1, 0);
    } else {
      channel_list_idx--;
    }
    return true;
  }
  if (e.key == Keyboard::ARROW_DOWN) {
    if (channel_list_idx < getChannelCount() - 1) {
      channel_list_idx++;
    } else {
      channel_list_idx = 0;
    }
    return true;
  }
  if (e.key == Keyboard::KEY_RETURN) {
    ChannelDetails c;
    if (the_mesh_cp.getChannel(channel_list_idx, c) && strlen(c.name) > 0) {
      _p->selectChannel(c, channel_list_idx);
    }
    return true;
  }

  return false;
}
