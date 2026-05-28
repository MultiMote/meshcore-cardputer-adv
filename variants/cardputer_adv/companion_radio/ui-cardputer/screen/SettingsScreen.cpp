#include "SettingsScreen.h"

void SettingsScreen::renderItem(DisplayDriver &display, SettingsItem item, int x, int y) {
  char tmp[64] = { 0 };
  DisplayDriver::Color text_color = DisplayDriver::YELLOW;

  switch (item) {
    case SettingsItem::HdrRadio:
      text_color = DisplayDriver::GREEN;
      snprintf(tmp, sizeof(tmp), "---- RADIO ----");
      break;

    case SettingsItem::HdrDevice:
      text_color = DisplayDriver::GREEN;
      snprintf(tmp, sizeof(tmp), "---- DEVICE ----");
      break;

    case SettingsItem::RadioFreq:
      snprintf(tmp, sizeof(tmp), "FREQ: %06.3f", _node_prefs->freq);
      break;

    case SettingsItem::RadioBw:
      snprintf(tmp, sizeof(tmp), "BW: %03.2f", _node_prefs->bw);
      break;

    case SettingsItem::RadioSf:
      snprintf(tmp, sizeof(tmp), "SF: %d", _node_prefs->sf);
      break;

    case SettingsItem::RadioCr:
      snprintf(tmp, sizeof(tmp), "CR: %d", _node_prefs->cr);
      break;

    case SettingsItem::RadioPwr:
      snprintf(tmp, sizeof(tmp), "PWR: %ddBm", _node_prefs->tx_power_dbm);
      break;

    case SettingsItem::DeviceBeep:
      snprintf(tmp, sizeof(tmp), "Sound: %s", _node_prefs->buzzer_quiet ? "OFF" : "ON");
      break;

    case SettingsItem::DeviceGps:
      snprintf(tmp, sizeof(tmp), "GPS: %s", _task->getGPSState() ? "ON" : "OFF");
      break;

    case SettingsItem::DeviceBluetooth:
      snprintf(tmp, sizeof(tmp), "Bluetooth: %s", _task->isSerialEnabled() ? "ON" : "OFF");
      break;

    default:
      text_color = DisplayDriver::RED;
      snprintf(tmp, sizeof(tmp), "???");
      break;
  }

  display.setColor(text_color);
  display.drawTextLeftAlign(x, y, tmp);
}
bool SettingsScreen::enterItemEdit(SettingsItem item) {
  switch (item) {
    case SettingsItem::HdrRadio:
    case SettingsItem::HdrDevice:
      return true;

    case SettingsItem::DeviceBeep:
      _node_prefs->buzzer_quiet = !_node_prefs->buzzer_quiet;
      the_mesh_cp.savePrefs();
      return true;

    case SettingsItem::DeviceGps:
      _task->toggleGPS();
      the_mesh_cp.savePrefs();
      return true;

    case SettingsItem::DeviceBluetooth:
      if (_task->isSerialEnabled()) {
        _task->disableSerial();
      } else {
        _task->enableSerial();
      }
      the_mesh_cp.savePrefs();
      return true;

    default:
      break;
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
      if (!commitItemEdit(static_cast<SettingsItem>(list_sel_idx))) {
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
