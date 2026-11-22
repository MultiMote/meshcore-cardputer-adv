#if CUSTOM_CARDPUTER_UI
#define TASK_CLASS   CardputerUITask
#define THE_MESH_OBJ the_mesh_cp
#include "CardputerMesh.h"
#include "CardputerUITask.h"

#else
#define TASK_CLASS   UITask
#define THE_MESH_OBJ the_mesh
#include "UITask.h"

#include <MyMesh.h>
#endif

#include "CardputerDataStore.h"

#include <Arduino.h>
#include <Mesh.h>
#include <helpers/MultiSerialInterface.h>
#include <helpers/esp32/SerialBLEInterface.h>
#if CUSTOM_CARDPUTER_UI
#include <helpers/wifi/SerialWifiInterface.h>
#endif

static uint32_t _atoi(const char *sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

#ifdef USE_SD_CARD
#include <SD.h>
CardputerDataStore store(SD, rtc_clock);
#else
#include <SPIFFS.h>
CardputerDataStore store(SPIFFS, rtc_clock);
#endif

MultiSerialInterface interface_manager;
SerialBLEInterface bluetooth_interface;
#if CUSTOM_CARDPUTER_UI
SerialWifiInterface wifi_interface;
bool wifi_active = false;
unsigned long last_wifi_reconnect = 0;
#endif

TASK_CLASS ui_task(&board, &interface_manager);

StdRNG fast_rng;
SimpleMeshTables tables;

#if CUSTOM_CARDPUTER_UI
CardputerMesh the_mesh_cp(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);
#else
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store);
#endif

void displayMessage(DisplayDriver *disp, const char *msg) {
  if (disp) {
    disp->startFrame();
    disp->setTextSize(2);
    disp->drawTextCentered(disp->width() / 2, 28, msg);
    disp->endFrame();
  }
}

void halt() {
  while (1) {
  }
}

void setup() {
  Serial.begin(115200);
  board.begin();

  DisplayDriver *disp = NULL;
  if (display.begin()) {
    disp = &display;
    displayMessage(disp, "Loading...");
    disp->endFrame();
  }

  if (!radio_init()) {
    displayMessage(disp, "Radio init failed");
    halt();
  }

  fast_rng.begin(radio_get_rng_seed());

#ifdef USE_SD_CARD
  // SPI setup done in radio.std_init so not calling SPI.begin
  if (!SD.begin(PIN_SD_CS, SPI, 25000000)) {
    displayMessage(disp, "Insert SD Card");
    halt();
  }
  if (disp) {
    display.tryLoadUserFont();
  }
  board.readKeyboardLayout(SD);
#else
  SPIFFS.begin(true);
#endif

  store.begin();
  THE_MESH_OBJ.begin();

  // MeshCore 1.18 leaves the active BLE PIN setup to the application.
#ifdef BLE_PIN_CODE
  uint32_t pin = THE_MESH_OBJ.getNodePrefs()->ble_pin;
  if (pin == 0) {
    pin = disp != NULL && BLE_PIN_CODE == 123456 ? fast_rng.nextInt(100000, 999999) : BLE_PIN_CODE;
  }
  THE_MESH_OBJ.setBLEPin(pin);
#endif

#if CUSTOM_CARDPUTER_UI
  NodePrefs *prefs = THE_MESH_OBJ.getNodePrefs();
  wifi_active = prefs->wifi_enabled && prefs->getWifiSSID()[0];
  if (wifi_active) {
    board.setInhibitSleep(true);
    WiFi.mode(WIFI_STA);
    WiFi.setAutoReconnect(true);
    WiFi.begin(prefs->getWifiSSID(), prefs->wifi_pwd);
    wifi_interface.begin(TCP_PORT);
    interface_manager.addInterface(InterfaceType::WiFi, &wifi_interface);
    last_wifi_reconnect = millis();
  } else
#endif
  {
    bluetooth_interface.begin(BLE_NAME_PREFIX, THE_MESH_OBJ.getNodePrefs()->node_name,
                              THE_MESH_OBJ.getBLEPin());
    interface_manager.addInterface(InterfaceType::Bluetooth, &bluetooth_interface);
  }

  THE_MESH_OBJ.startInterface(interface_manager);
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  THE_MESH_OBJ.applyGpsPrefs();
#endif

#if CUSTOM_CARDPUTER_UI
  ui_task.begin(disp, &sensors, THE_MESH_OBJ.getNodePrefs(), THE_MESH_OBJ.getCustomNodePrefs());
#else
  ui_task.begin(disp, &sensors, THE_MESH_OBJ.getNodePrefs());
#endif
  THE_MESH_OBJ.setListener(&ui_task);
  board.onBootComplete();
}

void loop() {
  THE_MESH_OBJ.loop();
  interface_manager.loop();
  sensors.loop();
  ui_task.loop();
  rtc_clock.tick();
  board.loop();

#if CUSTOM_CARDPUTER_UI
  // Wi-Fi must stay awake to accept companion connections, including after a link loss.
  if (wifi_active && WiFi.status() != WL_CONNECTED &&
      millis() - last_wifi_reconnect >= 10000) {
    WiFi.reconnect();
    last_wifi_reconnect = millis();
  }
  if (!wifi_active && ui_task.powerSaveEnabled() && millis() > ui_task.getAutoOffTime()) {
    board.enterLightSleep();
  }
#endif
}
