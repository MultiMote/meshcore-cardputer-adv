#pragma once

#define RADIOLIB_STATIC_ONLY 1
#include "CardputerSensorManager.h"

#include <CardputerAdvBoard.h>
#include <RadioLib.h>
#include <helpers/AutoDiscoverRTCClock.h>
#include <helpers/SensorManager.h>
#include <helpers/radiolib/CustomSX1262Wrapper.h>
#include <helpers/radiolib/RadioLibWrappers.h>
#include <helpers/ui/MomentaryButton.h>
#include <hw/CardputerDisplay.h>

extern CardputerAdvBoard board;
extern WRAPPER_CLASS radio_driver;
extern ESP32RTCClock rtc_clock;
extern CardputerSensorManager sensors;
extern DISPLAY_CLASS display;
extern MomentaryButton user_btn;

bool radio_init();
uint32_t radio_get_rng_seed();
void radio_set_params(float freq, float bw, uint8_t sf, uint8_t cr);
void radio_set_tx_power(int8_t dbm);
mesh::LocalIdentity radio_new_identity();
