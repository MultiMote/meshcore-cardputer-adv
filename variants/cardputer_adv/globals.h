#pragma once

#define CUSTOM_DATA_DIR                          "/meshcore_custom"
#define CUSTOM_PREFS_FILE                        CUSTOM_DATA_DIR "/prefs"
#define HISTORY_DIR                              CUSTOM_DATA_DIR "/history"
#define HISTORY_DIR_CHANNEL                      HISTORY_DIR "/chan"
#define HISTORY_DIR_DIRECT                       HISTORY_DIR "/direct"

#define MAX_MESSAGE_LENGTH                       150

#define BATT_MIN_MILLIVOLTS                      3350 // From M5Unified
#define BATT_MAX_MILLIVOLTS                      4150 // From M5Unified

#define UI_TEXT_LINE_HEIGHT                      12 // adjust to the used font
#define UI_RECENT_LIST_SIZE                      4
#define UI_CONTACT_LIST_SIZE                     8
#define UI_CHANNEL_LIST_SIZE                     8
#define UI_SETTINGS_LIST_SIZE                    9
#define UI_CHAT_HISTORY_SIZE                     7
#define UI_TOOLS_LIST_SIZE                       9

#define AUTO_OFF_MILLIS                          15000 // 15 seconds

#define MAX_UNREAD_MSGS                          32
#define BOOT_SCREEN_MILLIS                       3000

#define MAX_KB_LAYOUT_ITEMS                      100
#define MAX_KB_LAYOUT_REPLACEMENT_BYTES          3
#define KB_LAYOUT_FILE                           (CUSTOM_DATA_DIR "/keyboard.txt")

#define USER_FONT_NAME                           (CUSTOM_DATA_DIR "/font.vlw")

#define MAX_DISCOVERED_REPEATERS                 8
#define CONTACT_LOOKUP_BYTES 8 // I hope 8 bytes will be enough
