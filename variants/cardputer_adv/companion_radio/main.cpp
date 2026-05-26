#if CUSTOM_CARDPUTER_UI
  #include "CardputerUITask.h"
  #define TASK_CLASS CardputerUITask
#else
  #include "UITask.h"
  #define TASK_CLASS UITask
#endif

#include <Arduino.h>
#include <Mesh.h>
#include <MyMesh.h>
#include <helpers/esp32/SerialBLEInterface.h>

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
DataStore store(SD, rtc_clock);
#else
  #include <SPIFFS.h>
DataStore store(SPIFFS, rtc_clock);
#endif

SerialBLEInterface serial_interface;

TASK_CLASS ui_task(&board, &serial_interface);

StdRNG fast_rng;
SimpleMeshTables tables;
MyMesh the_mesh(radio_driver, fast_rng, rtc_clock, tables, store, &ui_task);

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
  if (!SD.begin(PIN_SD_CS, SPI)) {
    if (disp) {
      disp->startFrame();
      disp->setTextSize(2);
      disp->drawTextCentered(disp->width() / 2, 28, "Insert SD Card");
      disp->endFrame();
    }
    halt();
  }
#else
  SPIFFS.begin(true);
#endif

  store.begin();
  the_mesh.begin(true);

  serial_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
  the_mesh.startInterface(serial_interface);
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  the_mesh.applyGpsPrefs();
#endif

  ui_task.begin(disp, &sensors, the_mesh.getNodePrefs());
}

void loop() {
  the_mesh.loop();
  sensors.loop();
  ui_task.loop();
  rtc_clock.tick();

#if CUSTOM_CARDPUTER_UI
  if (ui_task.isSleepEnabled() && millis() > ui_task.getAutoOffTime()) {
    board.enterLightSleep();
  }
#endif
}
