#include "GpsPage.h"

int GpsPage::render(CardputerDisplay &lcd) {
#if ENV_INCLUDE_GPS == 1
  LocationProvider *nmea = sensors.getLocationProvider();
  char buf[50];
  int y = 18;
  bool gps_state = _p->getUiTask()->getGPSState();
  strcpy(buf, gps_state ? "gps on" : "gps off");
  lcd.drawTextLeftAlign(0, y, buf);
  if (nmea == NULL) {
    y = y + UI_TEXT_LINE_HEIGHT;
    lcd.drawTextLeftAlign(0, y, "Can't access GPS");
  } else {
    strcpy(buf, nmea->isValid() ? "fix" : "no fix");
    lcd.drawTextRightAlign(lcd.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    lcd.drawTextLeftAlign(0, y, "sat");
    sprintf(buf, "%d", nmea->satellitesCount());
    lcd.drawTextRightAlign(lcd.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    lcd.drawTextLeftAlign(0, y, "pos");
    sprintf(buf, "%.4f %.4f", nmea->getLatitude() / 1000000., nmea->getLongitude() / 1000000.);
    lcd.drawTextRightAlign(lcd.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
    lcd.drawTextLeftAlign(0, y, "alt");
    sprintf(buf, "%.2f", nmea->getAltitude() / 1000.);
    lcd.drawTextRightAlign(lcd.width() - 1, y, buf);
    y = y + UI_TEXT_LINE_HEIGHT;
  }
#endif
  return 5000;
}

bool GpsPage::handleInput(Keyboard::Event &e) {
#if ENV_INCLUDE_GPS == 1
  if (e.key == Keyboard::KEY_RETURN) {
    _p->getUiTask()->toggleGPS();
    return true;
  }
#endif
  return false;
}
