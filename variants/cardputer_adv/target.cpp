#include "target.h"

#include <Arduino.h>
#include <helpers/sensors/MicroNMEALocationProvider.h>

CardputerAdvBoard board;
m5::PI4IOE5V6408_Class ioe(0x43, 400000, &m5::In_I2C);
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, SPI);
WRAPPER_CLASS radio_driver(radio, board);
ESP32RTCClock rtc_clock;

MicroNMEALocationProvider nmea = MicroNMEALocationProvider(Serial1, &rtc_clock);
CardputerSensorManager sensors = CardputerSensorManager(nmea);

DISPLAY_CLASS display;
MomentaryButton user_btn(PIN_USER_BTN, 1000, true);

bool radio_init() {
  rtc_clock.begin();

  if (ioe.begin()) {
    ioe.setDirection(0, true);      // Output
    ioe.setHighImpedance(0, false); // Disable high-impedance so pin can actually drive
    ioe.digitalWrite(0, true);      // High Level
  } else {
    MESH_DEBUG_PRINTLN("Cap LoRa-1262 not found");
  }

  return radio.std_init(&SPI);
}

uint32_t radio_get_rng_seed() {
  return radio.random(0x7FFFFFFF);
}

void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr) {
  radio.setFrequency(freq);
  radio.setSpreadingFactor(sf);
  radio.setBandwidth(bw);
  radio.setCodingRate(cr);
}

void radio_set_tx_power(int8_t dbm) {
  radio.setOutputPower(dbm);
}

mesh::LocalIdentity radio_new_identity() {
  RadioNoiseListener rng(radio);
  return mesh::LocalIdentity(&rng); // create new random identity
}
