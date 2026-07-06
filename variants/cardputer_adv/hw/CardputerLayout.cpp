#include "CardputerLayout.h"

using namespace Keyboard::Layout;

static LayoutLayer default_layout_layers[2] = {
  { (Keyboard::Layout::LAYER_MASK_BASE),
    { "`", "",  "",  "",  "1", "q", "",  "",  "2", "w", "a", "",  "3", "e", "s", "z",  "4", "r", "d",
      "x", "5", "t", "f", "c", "6", "y", "g", "v", "7", "u", "h", "b", "8", "i", "j",  "n", "9", "o",
      "k", "m", "0", "p", "l", ",", "-", "[", ";", ".", "=", "]", "'", "/", "",  "\\", "",  "" } },
  { (Keyboard::Layout::LAYER_MASK_SHIFT),
    { "~", "",  "",  "",  "!", "Q", "",  "",  "@", "W", "A", "",  "#",  "E", "S", "Z", "$", "R", "D",
      "X", "%", "T", "F", "C", "^", "Y", "G", "V", "&", "U", "H", "B",  "*", "I", "J", "N", "(", "O",
      "K", "M", ")", "P", "L", "<", "_", "{", ":", ">", "+", "}", "\"", "?", "",  "|", "",  "" } }
};

static LayoutData default_layout = { .language_code = "en",
                                     .layers_count = 2,
                                     .layers = default_layout_layers };

uint8_t CardputerLayout::getLayerMask(Keyboard::Modifiers modifiers) const {
  return (modifiers.fn ? LAYER_MASK_FN : 0) | (modifiers.ctrl ? LAYER_MASK_CTRL : 0) |
         (modifiers.shift ? LAYER_MASK_SHIFT : 0) | (modifiers.opt ? LAYER_MASK_OPT : 0) |
         (modifiers.alt ? LAYER_MASK_ALT : 0);
}

Keyboard::Layout::CardputerLayout::CardputerLayout()
    : main_layout(&default_layout), current_layout(&default_layout) {}


const char *CardputerLayout::lookup(Keyboard::Event &event, bool force_default) {
  if (event.key == Keyboard::KEY_NONE || event.key > TOTAL_KEYS || !current_layout) {
    return "";
  }

  const LayoutData *tgt = force_default ? &default_layout : current_layout;
  uint8_t mask = getLayerMask(event.modifiers);
  uint8_t key = event.key - 1;

  // Try to find an exact match in the current modifier layer
  for (uint8_t i = 0; i < tgt->layers_count; i++) {
    if (mask == tgt->layers[i].modifiers_mask) {
      const char *data = tgt->layers[i].entries[key].data;
      if (data && data[0]) {
        return data;
      }
    }
  }

  // If Shift is pressed and nothing was found, try the dedicated SHIFT layer
  if (event.modifiers.shift && mask != LAYER_MASK_SHIFT) {
    for (uint8_t i = 0; i < tgt->layers_count; i++) {
      if (LAYER_MASK_SHIFT == tgt->layers[i].modifiers_mask) {
        const char *data = tgt->layers[i].entries[key].data;
        if (data && data[0]) {
          return data;
        }
      }
    }
  }

  // Fallback to the BASE layer if we haven't returned anything yet
  if (mask != LAYER_MASK_BASE) {
    for (uint8_t i = 0; i < tgt->layers_count; i++) {
      if (LAYER_MASK_BASE == tgt->layers[i].modifiers_mask) {
        const char *data = tgt->layers[i].entries[key].data;
        if (data && data[0]) {
          return data;
        }
      }
    }
  }

  // Handle space if it not overriden
  if (event.key == Keyboard::KEY_SPACE && !event.modifiers.ctrl) {
    return " ";
  }

  return "";
}

void CardputerLayout::begin(FS &fs) {
  parseLayout(fs, LAYOUT_TYPE_MAIN);
  parseLayout(fs, LAYOUT_TYPE_ALT);
}

// Uses dynamic allocation
void CardputerLayout::parseLayout(FS &fs, LayoutType layoutType) {
  const char *filename = (layoutType == LAYOUT_TYPE_MAIN) ? KB_LAYOUT_MAIN_FILE : KB_LAYOUT_ALT_FILE;
  if (!fs.exists(filename)) {
    return;
  }

  File layout_file = fs.open(filename, FILE_READ, false);
  if (!layout_file) {
    return;
  }

  LayoutData *layout = new LayoutData();
  layout->language_code[0] = '\0';
  layout->layers_count = 0;
  layout->layers = nullptr;

  int current_layer_index = -1;

  while (layout_file.available()) {
    String line = layout_file.readStringUntil('\n');
    if (line.endsWith("\r")) {
      line.remove(line.length() - 1);
    }

    if (line.length() < 1 || line.charAt(0) == '#') {
      continue;
    }

    // Lang code (format "N xx")
    if (line.charAt(0) == 'N' && line.length() >= 4) {
      strncpy(layout->language_code, line.c_str() + 2, 2);
      layout->language_code[2] = '\0';
    }

    // Layer mask (format "L BFCSOA", B = base, F = fn, C = ctrl, S = shift, O = opt, A = alt)
    else if (line.charAt(0) == 'L' && line.length() >= 3) {
      uint8_t mask = 0;

      for (unsigned int i = 2; i < line.length(); i++) {
        char mod = line.charAt(i);
        if (mod == 'B') {
          mask = LAYER_MASK_BASE;
        } else if (mod == 'F') {
          mask |= LAYER_MASK_FN;
        } else if (mod == 'C') {
          mask |= LAYER_MASK_CTRL;
        } else if (mod == 'S') {
          mask |= LAYER_MASK_SHIFT;
        } else if (mod == 'O') {
          mask |= LAYER_MASK_OPT;
        } else if (mod == 'A') {
          mask |= LAYER_MASK_ALT;
        }
      }

      // Dynamically expand the layout array for the new layer
      uint8_t new_count = layout->layers_count + 1;
      LayoutLayer *new_layers = new LayoutLayer[new_count];

      memset(new_layers, 0, sizeof(LayoutLayer) * new_count);

      if (layout->layers != nullptr) {
        // Copy old array contents into the expanded array space
        memcpy(new_layers, layout->layers, sizeof(LayoutLayer) * layout->layers_count);
        delete[] layout->layers; // Clear old buffer
      }

      // Setup new layer
      new_layers[new_count - 1].modifiers_mask = mask;
      layout->layers = new_layers;
      layout->layers_count = new_count;
      current_layer_index = new_count - 1;
    }

    // Key replacements (format "K NN=u8char")
    else if (line.charAt(0) == 'K' && line.length() >= 5 && line.charAt(4) == '=' &&
             current_layer_index != -1) {
      String id_str = line.substring(2, 4);
      int key_id = id_str.toInt();

      if (key_id >= 1 && key_id <= TOTAL_KEYS) {
        int key_idx = key_id - 1;
        String val_str = line.substring(5);

        char *dest = layout->layers[current_layer_index].entries[key_idx].data;
        memset(dest, 0, KB_LAYOUT_CHAR_MAX + 1);
        strncpy(dest, val_str.c_str(), KB_LAYOUT_CHAR_MAX);
      }
    }
  }
  layout_file.close();

  if (layout->layers_count == 0) {
    delete layout;
    return;
  }

  if (layoutType == LAYOUT_TYPE_MAIN) {
    if (main_layout && main_layout != &default_layout) {
      delete[] main_layout->layers;
      delete main_layout;
    }
    main_layout = layout;
    current_layout = main_layout;
  } else {
    if (alternate_layout) {
      delete[] alternate_layout->layers;
      delete alternate_layout;
    }
    alternate_layout = layout;
  }
}

void Keyboard::Layout::CardputerLayout::switchLayout() {
  LayoutType new_type = (current_layout_type == LAYOUT_TYPE_MAIN) ? LAYOUT_TYPE_ALT : LAYOUT_TYPE_MAIN;
  LayoutData *new_layout = (new_type == LAYOUT_TYPE_MAIN) ? main_layout : alternate_layout;

  if (new_layout) {
    current_layout_type = new_type;
    current_layout = new_layout;
  }
};
