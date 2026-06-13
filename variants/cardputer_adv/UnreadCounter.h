#pragma once
#include "globals.h"

#include <stdint.h>
#include <string.h>

struct UnreadCounterItem {
  bool is_channel;
  uint8_t pkey[CONTACT_LOOKUP_BYTES]; // todo: maybe replace to index, but what should we do if it gets deleted?
  int count;
};

class UnreadCounter {

private:
  UnreadCounterItem _items[UNREAD_COUNTER_MAX_ITEMS];
  uint16_t _count = 0;
  using LookupPkey = uint8_t[CONTACT_LOOKUP_BYTES];

public:
  UnreadCounter() {}

  const UnreadCounterItem *get(uint16_t index) const {
    if (index < 0 || index >= _count) {
      return nullptr;
    }
    return &_items[index];
  }

  inline const uint16_t countChats() const { return _count; }
  const uint16_t countMessages() const {
    uint16_t total = 0;
    for (uint16_t i = 0; i < _count; i++) {
      total += _items[i].count;
    }
    return total;
  }

  bool add(bool is_channel, const LookupPkey pkey, int count) {
    for (uint16_t i = 0; i < _count; i++) {
      if (_items[i].is_channel == is_channel && memcmp(_items[i].pkey, pkey, CONTACT_LOOKUP_BYTES) == 0) {
        _items[i].count += count;
        return true;
      }
    }

    if (_count < UNREAD_COUNTER_MAX_ITEMS) {
      _items[_count].is_channel = is_channel;
      _items[_count].count = count;
      memcpy(_items[_count].pkey, pkey, CONTACT_LOOKUP_BYTES);
      _count++;
      return true;
    }
    return false;
  }

  void reset(bool is_channel, LookupPkey pkey) {
    for (uint16_t i = 0; i < _count; i++) {
      if (_items[i].is_channel == is_channel && memcmp(_items[i].pkey, pkey, CONTACT_LOOKUP_BYTES) == 0) {
        // Swap the target item with the last active item
        if (i < _count - 1) {
          _items[i] = _items[_count - 1];
        }

        _count--;
        _items[_count] = { false, 0, 0 };
        return;
      }
    }
  }

  inline void addChannel(const LookupPkey pkey, int count) { add(true, pkey, count); }

  inline void addContact(const LookupPkey pkey, int count) { add(false, pkey, count); }

  inline void resetChannel(LookupPkey pkey) { reset(true, pkey); }

  inline void resetContact(LookupPkey pkey) { reset(false, pkey); }
};
