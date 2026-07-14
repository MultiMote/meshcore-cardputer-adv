#pragma once

#define CUSTOM_DATA_DIR                          "/meshcore_custom"
#define CUSTOM_PREFS_FILE                        CUSTOM_DATA_DIR "/prefs"
#define HISTORY_DIR                              CUSTOM_DATA_DIR "/history"
#define HISTORY_DIR_CHANNEL                      HISTORY_DIR "/chan"
#define HISTORY_DIR_DIRECT                       HISTORY_DIR "/direct"
#define KB_LAYOUT_MAIN_FILE                      CUSTOM_DATA_DIR "/keyboard_main.txt"
#define KB_LAYOUT_ALT_FILE                       CUSTOM_DATA_DIR "/keyboard_alt.txt"
#define USER_FONT_NAME                           (CUSTOM_DATA_DIR "/font.vlw")
/** Max utf-8 character length for keyboard key */
#define KB_LAYOUT_CHAR_MAX                       3

#define MAX_MESSAGE_LENGTH                       150

#define BATT_MIN_MILLIVOLTS                      3350 // From M5Unified
#define BATT_MAX_MILLIVOLTS                      4150 // From M5Unified

#define UI_TEXT_LINE_HEIGHT                      12 // adjust to the used font todo: replace to display.getFontLineHeight
#define UI_RECENT_LIST_SIZE                      4
#define UI_CONTACT_LIST_SIZE                     8
#define UI_CHANNEL_LIST_SIZE                     8
#define UI_SETTINGS_LIST_SIZE                    9
#define UI_CHAT_HISTORY_LIST_SIZE                5
#define UI_TOOLS_LIST_SIZE                       9

#define AUTO_OFF_MILLIS                          15000 // 15 seconds

#define MAX_UNREAD_MSGS                          32
#define BOOT_SCREEN_MILLIS                       2000

#define CHAT_HISTORY_RINGBUF_SIZE                15
#define UNREAD_COUNTER_MAX_ITEMS                 32

#define MAX_DISCOVERED_REPEATERS                 8
#define CONTACT_LOOKUP_BYTES                     6

#define DIRECT_SEND_ROUTE_ATTEMPTS               3
#define DIRECT_SEND_FLOOD_ATTEMPTS               3

// ================== PINS ==================

// Cap LoRa-1262 IO expander, audio codec (ES8311), keyboard (TCA8418)
#define PIN_I2C_INTERNAL_SDA                     8
#define PIN_I2C_INTERNAL_SCL                     9

// Cap LoRa-1262 IO expander
#define LORA_IOE_I2C_ADDRESS                     0x43

// LCD (ST7789V2)
#define TFT_SPI_HOST                             SPI3_HOST
#define PIN_TFT_SCK                              36
#define PIN_TFT_MOSI                             35
#define PIN_TFT_RST                              33
#define PIN_TFT_DC                               34
#define PIN_TFT_CS                               37
#define PIN_TFT_BL                               38

// Audio codec (ES8311)
#define AUDIO_I2C_ADDRESS                        0x18
#define PIN_AUDIO_SCLK                           41
#define PIN_AUDIO_ASDOUT                         46
#define PIN_AUDIO_LRCK                           43
#define PIN_AUDIO_DSDIN                          42
#define AUDIO_SAMPLE_RATE                        16000
#define AUDIO_MAX_SAFE_VOLUME                    20000.0f
#define AUDIO_BUFFER_SIZE                        256
/** Time to wait from codec init to first sound played */
#define AUDIO_WARMUP_MS                          1100

// Keyboard (TCA8418)
#define KEYBOARD_I2C_ADDRESS                     0x34
#define PIN_KEYBOARD_INT                         11
