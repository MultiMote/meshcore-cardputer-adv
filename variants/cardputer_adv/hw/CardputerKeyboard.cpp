#include "CardputerKeyboard.h"
#include "globals.h"
#include <Wire.h>

bool CardputerKeyboard::isModifier(Keyboard::Key key) {

  return key == Keyboard::KEY_SHIFT || key == Keyboard::KEY_OPT ||
         key == Keyboard::KEY_ALT || key == Keyboard::KEY_CTRL ||
         key == Keyboard::KEY_FN;
}

void CardputerKeyboard::updateModifier(Keyboard::Key key, bool down) {
  switch (key) {
  case Keyboard::KEY_SHIFT:
    modifiers.shift = down;
    break;
  case Keyboard::KEY_OPT:
    modifiers.opt = down;
    break;
  case Keyboard::KEY_ALT:
    modifiers.alt = down;
    break;
  case Keyboard::KEY_CTRL:
    modifiers.ctrl = down;
    break;
  case Keyboard::KEY_FN:
    modifiers.fn = down;
    break;
  }
}

void CardputerKeyboard::begin() {
  tio.begin(KEYBOARD_I2C_ADDRESS, &Wire1);
  tio.matrix(7, 8);
  tio.flush();
  tio.enableInterrupts();
}

Keyboard::Event CardputerKeyboard::poll() {
  Keyboard::Event result;
  result.changed = tio.available() > 0;

  if (result.changed) {
    uint8_t e = tio.getEvent();
    bool down = (e & 0x80);
    tio.writeRegister(TCA8418_REG_INT_STAT, 1);

    e &= 0x7F;

    // Normalize key code to remove gaps
    uint8_t key_index = ((e - 1) / 10) * 8 + ((e - 1) % 10) + 1;
    Keyboard::Key key = static_cast<Keyboard::Key>(key_index);

    result.down = down;
    
    if (isModifier(key)) {
      updateModifier(key, down);
    } else {
      result.key = key;
    }
  }

  result.modifiers = modifiers;
  return result;
}
