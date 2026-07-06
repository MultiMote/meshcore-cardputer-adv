#pragma once

#include "CardputerKeyboard.h"
#include "globals.h"

#include <FS.h>
#include <stdint.h>


#define TOTAL_KEYS 56

namespace Keyboard {
namespace Layout {
enum LayoutType { LAYOUT_TYPE_MAIN, LAYOUT_TYPE_ALT };

enum LayerMask {
  LAYER_MASK_BASE = 0x00,
  LAYER_MASK_FN = 0x01,
  LAYER_MASK_CTRL = 0x02,
  LAYER_MASK_SHIFT = 0x04,
  LAYER_MASK_OPT = 0x08,
  LAYER_MASK_ALT = 0x10
};

struct LayoutEntry {
  char data[KB_LAYOUT_CHAR_MAX + 1];
};

struct LayoutLayer {
  uint8_t modifiers_mask;
  LayoutEntry entries[TOTAL_KEYS];
};

struct LayoutData {
  char language_code[3];
  uint8_t layers_count;
  LayoutLayer *layers;
};

class CardputerLayout {
private:
  LayoutData *main_layout = nullptr;
  LayoutData *alternate_layout = nullptr;
  LayoutData *current_layout = nullptr;
  LayoutType current_layout_type = LAYOUT_TYPE_MAIN;
  void parseLayout(FS &fs, LayoutType layoutType);
  uint8_t getLayerMask(Keyboard::Modifiers modifiers) const;

public:
  CardputerLayout();
  void begin(FS &fs);

  /// @brief Tries to find a matching layout utf-8 character
  /// @param event Keyboard event
  /// @param force_default Ignores custom layout and uses the default one
  /// @return Utf-8 character or empty string. NULL is never returned.
  const char *lookup(Keyboard::Event &event, bool force_default = false);

  inline const char *lookupDefault(Keyboard::Event &event) { return lookup(event, true); }

  void switchLayout();
  inline Keyboard::Layout::LayoutType getCurrentLayoutType() const { return current_layout_type; }
  inline bool hasAlternateLayout() const { return alternate_layout; }
  inline const char *getMainLayoutCode() const { return main_layout ? main_layout->language_code : "??"; }
  inline const char *getAlternateLayoutCode() const {
    return alternate_layout ? alternate_layout->language_code : "??";
  }
  inline const char *getCurrentCode() const {
    return getCurrentLayoutType() == LAYOUT_TYPE_MAIN ? getMainLayoutCode() : getAlternateLayoutCode();
  }
};

} // namespace Layout

} // namespace Keyboard

using Keyboard::Layout::CardputerLayout;
