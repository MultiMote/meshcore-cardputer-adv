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

    case SettingsItem::HdrMesh:
      text_color = DisplayDriver::GREEN;
      snprintf(tmp, sizeof(tmp), "---- MESH ----");
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

    case SettingsItem::DevicePowersave:
      snprintf(tmp, sizeof(tmp), "Power save: %s", _task->powerSaveEnabled() ? "ON" : "OFF");
      break;

    case SettingsItem::DeviceBatteryCorrection:
      snprintf(tmp, sizeof(tmp), "Battery correction: %.3f",
               _custom_prefs->battery_correction);
      break;

    case SettingsItem::MeshPathSize:
      snprintf(tmp, sizeof(tmp), "Path hash size: %d", _node_prefs->path_hash_mode + 1);
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
    case SettingsItem::HdrMesh:
      return true;

    case SettingsItem::DeviceBeep:
      _task->toggleBuzzer();
      return true;

    case SettingsItem::DeviceGps:
      _task->toggleGPS();
      return true;

    case SettingsItem::DeviceBluetooth:
      if (_task->isSerialEnabled()) {
        _task->disableSerial();
      } else {
        _task->enableSerial();
      }
      the_mesh_cp.savePrefs();
      return true;

    case SettingsItem::DevicePowersave:
      _task->togglePowerSave();
      return true;

    case SettingsItem::MeshPathSize:
      edit_u8 = _node_prefs->path_hash_mode + 1;
      is_editing = true;
      return true;

    case SettingsItem::RadioSf:
      edit_u8 = _node_prefs->sf;
      is_editing = true;
      return true;

    case SettingsItem::RadioCr:
      edit_u8 = _node_prefs->cr;
      is_editing = true;
      return true;

    case SettingsItem::RadioPwr:
      edit_u8 = _node_prefs->tx_power_dbm;
      is_editing = true;
      return true;

    case SettingsItem::RadioFreq:
      edit_buffer = String(_node_prefs->freq, 3);
      is_editing = true;
      return true;

    case SettingsItem::RadioBw:
      edit_buffer = String(_node_prefs->bw, 2);
      is_editing = true;
      return true;

    case SettingsItem::DeviceBatteryCorrection:
      edit_buffer = String(_board->getBattMilliVolts());
      is_editing = true;
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

  switch (item) {
    case SettingsItem::RadioSf:
      _node_prefs->sf = edit_u8;
      the_mesh_cp.savePrefs();
      restart_required = true;
      return true;

    case SettingsItem::RadioCr:
      _node_prefs->cr = edit_u8;
      the_mesh_cp.savePrefs();
      restart_required = true;
      return true;

    case SettingsItem::RadioPwr:
      _node_prefs->tx_power_dbm = edit_u8;
      the_mesh_cp.savePrefs();
      restart_required = true;
      return true;

    case SettingsItem::MeshPathSize:
      _node_prefs->path_hash_mode = edit_u8 - 1;
      the_mesh_cp.savePrefs();
      return true;

    case SettingsItem::RadioFreq:
      if (inputParsePositiveFloat(_node_prefs->freq)) {
        the_mesh_cp.savePrefs();
        restart_required = true;
        return true;
      }
      break;

    case SettingsItem::RadioBw:
      if (inputParsePositiveFloat(_node_prefs->bw)) {
        the_mesh_cp.savePrefs();
        restart_required = true;
        return true;
      }
      break;

    case SettingsItem::DeviceBatteryCorrection:

      if (edit_buffer.length() > 0) {
        uint16_t real = edit_buffer.toInt();
        uint16_t adc = _board->getBattMilliVolts();
        if (adc != 0) {
          _custom_prefs->battery_correction = (float)real / adc;
          the_mesh_cp.savePrefs();
          return true;
        }
      }

      break;

    default:
      break;
  }

  return false;
}

void SettingsScreen::handleEditInput(SettingsItem item, char k) {
  switch (item) {
    case SettingsItem::RadioSf:
      if (k == KEY_LEFT && edit_u8 > 5) {
        edit_u8--;
      } else if (k == KEY_RIGHT && edit_u8 < 12) {
        edit_u8++;
      }
      break;
    case SettingsItem::RadioCr:
      if (k == KEY_LEFT && edit_u8 > 5) {
        edit_u8--;
      } else if (k == KEY_RIGHT && edit_u8 < 8) {
        edit_u8++;
      }
      break;

    case SettingsItem::RadioPwr:
      if (k == KEY_LEFT && edit_u8 > 1) {
        edit_u8--;
      } else if (k == KEY_RIGHT && edit_u8 < LORA_TX_POWER) {
        edit_u8++;
      }
      break;
    case SettingsItem::MeshPathSize:
      if (k == KEY_LEFT && edit_u8 > 1) {
        edit_u8--;
      } else if (k == KEY_RIGHT && edit_u8 < 3) {
        edit_u8++;
      }
      break;

    case SettingsItem::RadioFreq:
    case SettingsItem::RadioBw:
      if (k == ASCII_CTRL_BACKSPACE && edit_buffer.length() > 0) {
        edit_buffer.remove(edit_buffer.length() - 1);
      } else if ((k == '.' || k == KEY_DOWN) && edit_buffer.indexOf('.') == -1) {
        edit_buffer.concat('.');
      } else if (isdigit(k)) {
        edit_buffer.concat(k);
      }
      break;
    case SettingsItem::DeviceBatteryCorrection:
      if (k == ASCII_CTRL_BACKSPACE && edit_buffer.length() > 0) {
        edit_buffer.remove(edit_buffer.length() - 1);
      } else if (isdigit(k) && edit_buffer.length() < 4) {
        edit_buffer.concat(k);
      }
      break;

    default:
      break;
  }
}

bool SettingsScreen::inputParsePositiveFloat(float &val) {
  float tmp = edit_buffer.toFloat();

  if (tmp == INFINITY) {
    return false;
  }

  if (errno == ERANGE) {
    return false;
  }

  if (tmp < 0) {
    return false;
  }

  val = tmp;

  return true;
}

int SettingsScreen::render(DisplayDriver &display) {
  char buf[64];

  display.setTextSize(1);
  if (restart_required) {
    display.setColor(DisplayDriver::RED);
    display.drawTextCentered(display.width() / 2, 5, "Settings | Restart required");
  } else {
    display.setColor(DisplayDriver::GREEN);
    display.drawTextCentered(display.width() / 2, 5, "Settings");
  }

  display.setColor(DisplayDriver::YELLOW);
  display.setTextSize(1);

  int real_idx = 0;
  int list_page = menu_index / UI_SETTINGS_LIST_SIZE;
  int list_idx = menu_index % UI_SETTINGS_LIST_SIZE;

  for (int i = 0; i < UI_SETTINGS_LIST_SIZE; i++) {
    real_idx = list_page * UI_SETTINGS_LIST_SIZE + i;

    if (real_idx >= SettingsItem::Count) {
      break;
    }

    renderItem(display, static_cast<SettingsItem>(real_idx), 15, 20 + i * UI_TEXT_LINE_HEIGHT);

    if (i == list_idx) {
      display.setColor(DisplayDriver::GREEN);
      display.drawTextLeftAlign(5, 20 + i * UI_TEXT_LINE_HEIGHT, ">");
    }
  }

  if (is_editing) {
    int x_margin = 5;
    int y_margin = 40;
    display.setColor(DisplayDriver::DARK);
    display.fillRect(x_margin, y_margin, display.width() - x_margin * 2, display.height() - y_margin * 2);
    display.setColor(DisplayDriver::LIGHT);
    display.drawRect(x_margin, y_margin, display.width() - x_margin * 2, display.height() - y_margin * 2);
    display.setTextSize(2);

    switch (menu_index) {
      case SettingsItem::RadioSf:
      case SettingsItem::RadioCr:
      case SettingsItem::RadioPwr:
      case SettingsItem::MeshPathSize:
        sprintf(buf, "- %d +", edit_u8);
        display.drawTextCentered(display.width() / 2, display.height() / 2 - 8, buf);
        break;

      case SettingsItem::RadioFreq:
      case SettingsItem::RadioBw:
        sprintf(buf, "%s_", edit_buffer.c_str());
        display.drawTextCentered(display.width() / 2, display.height() / 2 - 8, buf);
        break;
      case SettingsItem::DeviceBatteryCorrection:
        sprintf(buf, "%s_", edit_buffer.c_str());
        display.drawTextCentered(display.width() / 2, display.height() / 2 - 8, buf);

        display.setTextSize(1);
        sprintf(buf, "Read value = %umV, real =", _board->getBattMilliVolts());
        display.drawTextCentered(display.width() / 2, display.height() / 2 - 16, buf);
        break;

      default:
        break;
    }
  }

  return 5000;
}

bool SettingsScreen::handleInput(char c) {
  if (is_editing) {
    if (c == ASCII_CTRL_LF) {
      if (!commitItemEdit(static_cast<SettingsItem>(menu_index))) {
        _task->showAlert("Invalid value", 1000);
      } else {
        is_editing = false;
      }
      return true;
    }

    if (c == ASCII_CTRL_ESCAPE) {
      cancelItemEdit(static_cast<SettingsItem>(menu_index));
      return true;
    }

    handleEditInput(static_cast<SettingsItem>(menu_index), c);

  } else {
    if (c == ASCII_CTRL_ESCAPE || c == ASCII_CTRL_DC1) {
      _task->gotoMainScreen();
      return true;
    }

    if (c == KEY_UP) {
      if (menu_index == 0) {
        menu_index = SettingsItem::Count - 1;
      } else {
        menu_index--;
      }

      return true;
    }
    if (c == KEY_DOWN) {
      if (menu_index < SettingsItem::Count - 1) {
        menu_index++;
      } else {
        menu_index = 0;
      }

      return true;
    }
    if (c == ASCII_CTRL_LF) {
      if (!enterItemEdit(static_cast<SettingsItem>(menu_index))) {
        _task->showAlert("Not implemented yet", 2000);
      }
      return true;
    }
  }

  return false;
}
