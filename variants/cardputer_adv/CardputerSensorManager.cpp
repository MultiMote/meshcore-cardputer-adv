#include "CardputerSensorManager.h"

void CardputerSensorManager::start_gps() {
  if (!gps_active) {
    Serial1.print("$PCAS10,0*1C\r\n"); // Hot reboot to exit standby mode
    delay(1000);
    gps_active = true;
  }
  _location->begin();
}

void CardputerSensorManager::stop_gps() {
  if (gps_active) {
    // Cap Lora-1262 does not have GPS switch pin
    // Instead we send a command to the GPS module to put it into standby for maximum possible value (65535 seconds)
    Serial1.print("$PCAS12,65535*1E\r\n");
    gps_active = false;
  }
  _location->stop();
}

bool CardputerSensorManager::begin() {
  Serial1.setPins(PIN_GPS_TX, PIN_GPS_RX);
  Serial1.begin(GPS_BAUD_RATE);

  delay(1000);

  // Check data available to determine if GPS active
  if (Serial1.available() > 0) {
    start_gps();
  }

  return true;
}

bool CardputerSensorManager::querySensors(uint8_t requester_permissions, CayenneLPP &telemetry) {
  if (requester_permissions & TELEM_PERM_LOCATION) { // does requester have permission?
    telemetry.addGPS(TELEM_CHANNEL_SELF, node_lat, node_lon, node_altitude);
  }
  return true;
}

void CardputerSensorManager::loop() {
  static long next_gps_update = 0;

  if (!gps_active) {
    return;
  }

  _location->loop();

  if (millis() > next_gps_update) {
    if (_location->isValid()) {
      node_lat = ((double)_location->getLatitude()) / 1000000.;
      node_lon = ((double)_location->getLongitude()) / 1000000.;
      node_altitude = ((double)_location->getAltitude()) / 1000.0;
      MESH_DEBUG_PRINTLN("VALID location: lat %f lon %f", node_lat, node_lon);
    } else {
      MESH_DEBUG_PRINTLN("INVALID location, waiting for fix");
    }
    MESH_DEBUG_PRINTLN("GPS satellites: %d", _location->satellitesCount());
    next_gps_update = millis() + 1000;
  }
}

int CardputerSensorManager::getNumSettings() const {
  return 1;
}

const char *CardputerSensorManager::getSettingName(int i) const {
  return i == 0 ? "gps" : NULL;
}

const char *CardputerSensorManager::getSettingValue(int i) const {
  if (i == 0) {
    return gps_active ? "1" : "0";
  }
  return NULL;
}

bool CardputerSensorManager::setSettingValue(const char *name, const char *value) {
  if (strcmp(name, "gps") == 0) {
    if (strcmp(value, "0") == 0) {
      stop_gps();
    } else {
      start_gps();
    }
    return true;
  }
  return false; // not supported
}
