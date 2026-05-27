#include "SettingsScreen.h"

void SettingsScreen::renderItem(DisplayDriver &display, SettingsItem item, int x, int y) {
  char tmp[64];
  display.setColor(DisplayDriver::YELLOW);

  if (item == SettingsItem::HdrRadio) {
    display.setColor(DisplayDriver::GREEN);
    display.drawTextLeftAlign(x, y, "---- RADIO ----");
  } else if (item == SettingsItem::RadioFreq) {
    sprintf(tmp, "FREQ: %06.3f", _node_prefs->freq);
    display.drawTextLeftAlign(x, y, tmp);
  } else if (item == SettingsItem::RadioBw) {
    sprintf(tmp, "BW: %03.2f", _node_prefs->bw);
    display.drawTextLeftAlign(x, y, tmp);
  } else if (item == SettingsItem::RadioSf) {
    sprintf(tmp, "SF: %d", _node_prefs->sf);
    display.drawTextLeftAlign(x, y, tmp);
  } else if (item == SettingsItem::RadioCr) {
    sprintf(tmp, "CR: %d", _node_prefs->cr);
    display.drawTextLeftAlign(x, y, tmp);
  } else if (item == SettingsItem::RadioPwr) {
    sprintf(tmp, "PWR: %ddBm", _node_prefs->tx_power_dbm);
    display.drawTextLeftAlign(x, y, tmp);
  } else if (item == SettingsItem::HdrDevice) {
    display.setColor(DisplayDriver::GREEN);
    display.drawTextLeftAlign(x, y, "---- DEVICE ----");
  } else if (item == SettingsItem::DeviceBeep) {
    sprintf(tmp, "Sound: %s", _node_prefs->buzzer_quiet ? "OFF" : "ON");
    display.drawTextLeftAlign(x, y, tmp);
  } else {
    display.setColor(DisplayDriver::RED);
    display.drawTextLeftAlign(x, y, "???");
  }
}

bool SettingsScreen::enterItemEdit(SettingsItem item) {
  if (item == SettingsItem::DeviceBeep) {
    _node_prefs->buzzer_quiet = !_node_prefs->buzzer_quiet;
    the_mesh.savePrefs();

    return true;
  }

  return false;
}

void SettingsScreen::cancelItemEdit(SettingsItem item) {
  is_editing = false;
}

bool SettingsScreen::commitItemEdit(SettingsItem item) {
  is_editing = false;
  return false;
}

int SettingsScreen::render(DisplayDriver &display) {
  display.setTextSize(1);
  display.setColor(DisplayDriver::GREEN);
  display.drawTextCentered(display.width() / 2, 5, "Settings");

  display.setColor(DisplayDriver::YELLOW);
  display.setTextSize(1);

  int real_idx = 0;
  int list_page = list_sel_idx / UI_SETTINGS_LIST_SIZE;
  int list_idx = list_sel_idx % UI_SETTINGS_LIST_SIZE;

  for (int i = 0; i < UI_SETTINGS_LIST_SIZE; i++) {
    real_idx = list_page * UI_SETTINGS_LIST_SIZE + i;

    if (real_idx >= SettingsItem::Count) {
      break;
    }

    renderItem(display, static_cast<SettingsItem>(i), 15, 20 + i * 10);

    if (i == list_idx) {
      display.setColor(DisplayDriver::GREEN);
      display.drawTextLeftAlign(5, 20 + i * 10, ">");
    }
  }

  return 1000;
}

bool SettingsScreen::handleInput(char c) {
  if (is_editing) {
    if (c == ASCII_CTRL_LF) {
      if(!commitItemEdit(static_cast<SettingsItem>(list_sel_idx))) {
        _task->showAlert("Validation error", 2000);
      }
      return true;
    }

    if (c == ASCII_CTRL_ESCAPE) {
      cancelItemEdit(static_cast<SettingsItem>(list_sel_idx));
      return true;
    }

  } else {
    if (c == ASCII_CTRL_ESCAPE || c == ASCII_CTRL_DC1) {
      _task->gotoHomeScreen();
      return true;
    }

    if (c == KEY_UP) {
      if (list_sel_idx == 0) {
        list_sel_idx = SettingsItem::Count - 1;
      } else {
        list_sel_idx--;
      }

      return true;
    }
    if (c == KEY_DOWN) {
      if (list_sel_idx < SettingsItem::Count - 1) {
        list_sel_idx++;
      } else {
        list_sel_idx = 0;
      }

      return true;
    }
    if (c == ASCII_CTRL_LF) {
      if (!enterItemEdit(static_cast<SettingsItem>(list_sel_idx))) {
        _task->showAlert("Not implemented yet", 2000);
      }
      return true;
    }
  }

  return false;
}
