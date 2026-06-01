#if CUSTOM_CARDPUTER_UI
  #define TASK_CLASS   CardputerUITask
  #define THE_MESH_OBJ the_mesh_cp
  #include "CardputerMesh.h"
  #include "CardputerUITask.h"
  #include "KeyboardLayout.h"

#else
  #define TASK_CLASS   UITask
  #define THE_MESH_OBJ the_mesh
  #include "UITask.h"

  #include <MyMesh.h>
#endif

#include <Arduino.h>
#include <Mesh.h>
#include <helpers/esp32/SerialBLEInterface.h>
#include "CardputerDataStore.h"

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
KeyboardLayout layout;
#else
  #include <SPIFFS.h>
CardputerDataStore store(SPIFFS, rtc_clock);
#endif

SerialBLEInterface serial_interface;

TASK_CLASS ui_task(&board, &serial_interface);

StdRNG fast_rng;
SimpleMeshTables tables;

#if CUSTOM_CARDPUTER_UI
CardputerMesh the_mesh_cp(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);
#else
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);
#endif

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
    disp->startFrame();
    disp->setTextSize(2);
    disp->drawTextCentered(disp->width() / 2, 28, "Loading...");
    disp->endFrame();
  }

  if (!radio_init()) {
    halt();
  }

  fast_rng.begin(radio_get_rng_seed());

#ifdef USE_SD_CARD
  // SPI setup done in radio.std_init so not calling SPI.begin
  if (!SD.begin(PIN_SD_CS, SPI, 25000000)) {
    if (disp) {
      disp->startFrame();
      disp->setTextSize(2);
      disp->drawTextCentered(disp->width() / 2, 28, "Insert SD Card");
      disp->endFrame();
    }
    halt();
  }
  if (disp) {
    display.tryLoadUserFont();
  }
  layout.begin(SD);
#else
  SPIFFS.begin(true);
#endif

  store.begin();
  THE_MESH_OBJ.begin(true);

  serial_interface.begin(BLE_NAME_PREFIX, THE_MESH_OBJ.getNodePrefs()->node_name, THE_MESH_OBJ.getBLEPin());
  THE_MESH_OBJ.startInterface(serial_interface);
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  THE_MESH_OBJ.applyGpsPrefs();
#endif

#if CUSTOM_CARDPUTER_UI
  ui_task.begin(disp, &sensors, THE_MESH_OBJ.getNodePrefs(), THE_MESH_OBJ.getCustomNodePrefs(), &layout);
#else
  ui_task.begin(disp, &sensors, THE_MESH_OBJ.getNodePrefs());
#endif

}

void loop() {
  THE_MESH_OBJ.loop();
  sensors.loop();
  ui_task.loop();
  rtc_clock.tick();

#if CUSTOM_CARDPUTER_UI
  if (ui_task.powerSaveEnabled() && millis() > ui_task.getAutoOffTime()) {
    board.enterLightSleep();
  }
#endif
}
