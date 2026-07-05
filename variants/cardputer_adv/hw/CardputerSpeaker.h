#pragma once

#include <stdint.h>

/** Basic ES8311 blocking driver */
class CardputerSpeaker {
private:
  struct queue_tone_t {
    int32_t frequency_hz = 0;
    uint32_t duration_ms = 0;
    float volume = 0;
    bool done = true;
  };

  bool i2c_initialized = false;
  bool i2s_initialized = false;

  uint32_t busy_time = 0;
  queue_tone_t queue_tone;

  void initI2C();
  void initI2S();
  bool writeReg(uint8_t reg, uint8_t val);

public:
  CardputerSpeaker() {}

  void begin();
  void playTone(int32_t frequency_hz, uint32_t duration_ms, float volume = 0.5f);
  void sleep();

  /** Queue a tone. Will be played when processQueue() is called and the codec read. */
  void queueTone(int32_t frequency_hz, uint32_t duration_ms, float volume = 0.5f);
  void processQueue();
};
