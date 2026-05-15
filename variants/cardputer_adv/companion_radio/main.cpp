#include "MyMesh.h"
#include "UITask.h"

#include <Arduino.h>
#include <Mesh.h>
#include <SPIFFS.h>
#include <helpers/esp32/SerialBLEInterface.h>

static uint32_t _atoi(const char *sp) {
  uint32_t n = 0;
  while (*sp && *sp >= '0' && *sp <= '9') {
    n *= 10;
    n += (*sp++ - '0');
  }
  return n;
}

DataStore store(SPIFFS, rtc_clock);
SerialBLEInterface serial_interface;

UITask ui_task(&board, &serial_interface);

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

  SPIFFS.begin(true);
  store.begin();
  the_mesh.begin(true);

  serial_interface.begin(BLE_NAME_PREFIX, the_mesh.getNodePrefs()->node_name, the_mesh.getBLEPin());
  the_mesh.startInterface(serial_interface);
  sensors.begin();

#if ENV_INCLUDE_GPS == 1
  the_mesh.applyGpsPrefs();
  // todo:
  // cap lora-1262 do not have gps power management and gps drains battery even when not in use
  // PCAS12 command can disable gps for 1-65535 seconds
  // we can use this command for power management
  // this example will disable gps for 60 seconds
  // Serial1.print("$PCAS12,60*28\r\n");
#endif

  ui_task.begin(disp, &sensors,
                the_mesh.getNodePrefs()); // still want to pass this in as dependency, as prefs might be moved
}

void loop() {
  the_mesh.loop();
  sensors.loop();
  board.loop();
  ui_task.loop();
  rtc_clock.tick();
}
