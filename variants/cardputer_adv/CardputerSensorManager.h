#pragma once

#include <helpers/SensorManager.h>
#include <helpers/sensors/LocationProvider.h>

class CardputerSensorManager : public SensorManager {
private:
  bool gps_active = false;
  LocationProvider *_location;

public:
  CardputerSensorManager(LocationProvider &location) : _location(&location) {}
  bool begin() override;
  bool querySensors(uint8_t requester_permissions, CayenneLPP &telemetry) override;
  void loop() override;
  int getNumSettings() const override;
  const char *getSettingName(int i) const override;
  const char *getSettingValue(int i) const override;
  bool setSettingValue(const char *name, const char *value) override;
  LocationProvider *getLocationProvider() override { return _location; }
  void start_gps();
  void stop_gps();
  void gps_standby(bool active);
};
