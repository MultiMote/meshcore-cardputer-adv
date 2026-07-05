#include "CardputerSpeaker.h"

#include "globals.h"

#include <Wire.h>
#include <driver/i2s.h> // because i2s_std.h not exists for some reason

void CardputerSpeaker::initI2C() {
  // From M5Unified
  writeReg(0x00, 0x80); // 0x00 RESET/  CSM POWER ON
  writeReg(0x01, 0xB5); // 0x01 CLOCK_MANAGER/ MCLK=BCLK
  writeReg(0x02, 0x18); // 0x02 CLOCK_MANAGER/ MULT_PRE=3
  writeReg(0x0D, 0x01); // 0x0D SYSTEM/ Power up analog circuitry
  writeReg(0x12, 0x00); // 0x12 SYSTEM/ power-up DAC - NOT default
  writeReg(0x13, 0x10); // 0x13 SYSTEM/ Enable output to HP drive - NOT default
  writeReg(0x32, 0xBF); // 0x32 DAC/ DAC volume (0xBF == ±0 dB )
  writeReg(0x37, 0x08); // 0x37 DAC/ Bypass DAC equalizer - NOT default

  // Codec is not ready immediately, it need some time to stabilize
  busy_time = millis() + AUDIO_WARMUP_MS;
  i2c_initialized = true;
}

void CardputerSpeaker::initI2S() {
  i2s_config_t i2s_config = { .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
                              .sample_rate = AUDIO_SAMPLE_RATE,
                              .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
                              .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
                              .communication_format = I2S_COMM_FORMAT_STAND_I2S,
                              .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
                              .dma_buf_count = 4,
                              .dma_buf_len = AUDIO_BUFFER_SIZE,
                              .use_apll = false,
                              .tx_desc_auto_clear = true,
                              .fixed_mclk = 0 };

  i2s_pin_config_t pin_config = {
    .bck_io_num = PIN_AUDIO_SCLK,
    .ws_io_num = PIN_AUDIO_LRCK,
    .data_out_num = PIN_AUDIO_DSDIN,
    .data_in_num = I2S_PIN_NO_CHANGE // Mic is not needed
  };

  i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
  i2s_set_pin(I2S_NUM_0, &pin_config);

  i2s_initialized = true;
}

bool CardputerSpeaker::writeReg(uint8_t reg, uint8_t val) {
  Wire1.beginTransmission(AUDIO_I2C_ADDRESS);
  Wire1.write(reg);
  Wire1.write(val);
  return Wire1.endTransmission() == 0;
}

void CardputerSpeaker::begin() {
  if (!i2s_initialized) {
    initI2S();
  }

  if (!i2c_initialized) {
    initI2C();
  }
}

void CardputerSpeaker::playTone(int32_t frequency_hz, uint32_t duration_ms, float volume) {
  begin();

  static double current_phase = 0.0;

  const double phase_increment = 2.0 * M_PI * frequency_hz / AUDIO_SAMPLE_RATE;
  const int16_t target_amplitude = static_cast<int16_t>(volume * AUDIO_MAX_SAFE_VOLUME);

  uint64_t total_samples_to_play = (uint64_t)AUDIO_SAMPLE_RATE * duration_ms / 1000ULL;
  uint64_t samples_played = 0;

  int16_t audio_buffer[AUDIO_BUFFER_SIZE];
  size_t bytes_written;

  while (samples_played < total_samples_to_play) {
    uint64_t remaining_samples = total_samples_to_play - samples_played;
    int samples_in_chunk =
        (int)(remaining_samples < AUDIO_BUFFER_SIZE ? remaining_samples : AUDIO_BUFFER_SIZE);

    for (int i = 0; i < samples_in_chunk; ++i) {
      audio_buffer[i] = static_cast<int16_t>(sin(current_phase) * target_amplitude);

      current_phase += phase_increment;
      if (current_phase >= 2.0 * M_PI) {
        current_phase -= 2.0 * M_PI;
      }
    }

    i2s_write(I2S_NUM_0, audio_buffer, samples_in_chunk * sizeof(int16_t), &bytes_written, portMAX_DELAY);

    samples_played += samples_in_chunk;
  }
}

void CardputerSpeaker::sleep() {
  if (i2c_initialized) {
    writeReg(0x0E, 0xFF); // SYSTEM
    writeReg(0x12, 0x02); // SYSTEM
    writeReg(0x0D, 0xFA); // POWER
    i2c_initialized = false;
  }

  if (i2s_initialized) {
    i2s_driver_uninstall(I2S_NUM_0);
    i2s_initialized = false;
  }
}

void CardputerSpeaker::queueTone(int32_t frequency_hz, uint32_t duration_ms, float volume) {
  begin();
  queue_tone.frequency_hz = frequency_hz;
  queue_tone.duration_ms = duration_ms;
  queue_tone.volume = volume;
  queue_tone.done = false;
}

void CardputerSpeaker::processQueue() {
  if (!queue_tone.done) {
    if (millis() > busy_time) {
      queue_tone.done = true;
      playTone(queue_tone.frequency_hz, queue_tone.duration_ms, queue_tone.volume);
    }
  }
}
