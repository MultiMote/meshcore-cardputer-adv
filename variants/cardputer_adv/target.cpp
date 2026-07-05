#include "target.h"

#include <Arduino.h>
#include <helpers/sensors/MicroNMEALocationProvider.h>

CardputerAdvBoard board;
RADIO_CLASS radio = new Module(P_LORA_NSS, P_LORA_DIO_1, P_LORA_RESET, P_LORA_BUSY, SPI);
WRAPPER_CLASS radio_driver(radio, board);
ESP32RTCClock rtc_clock;

MicroNMEALocationProvider nmea = MicroNMEALocationProvider(Serial1, &rtc_clock);
CardputerSensorManager sensors = CardputerSensorManager(nmea);

DISPLAY_CLASS display;

bool radio_init() {
  rtc_clock.begin();

  Wire1.beginTransmission(LORA_IOE_I2C_ADDRESS);  // PI4IOE5V6408ZTAEX Address
  Wire1.write(0x03); // Direction
  Wire1.write(0x01); // Output
  uint8_t err = Wire1.endTransmission();

  if (err != 0) {
    MESH_DEBUG_PRINTLN("Cap LoRa-1262 not found");
  }

  Wire1.beginTransmission(LORA_IOE_I2C_ADDRESS);
  Wire1.write(0x07); // High-impedance
  Wire1.write(0x00); // Disable high-impedance so pin can actually drive
  Wire1.endTransmission();

  Wire1.beginTransmission(LORA_IOE_I2C_ADDRESS);
  Wire1.write(0x05); // IO set
  Wire1.write(0x01); // High Level
  Wire1.endTransmission();

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
