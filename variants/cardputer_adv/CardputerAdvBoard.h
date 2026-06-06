#pragma once

#include <Arduino.h>
#include <M5Cardputer.h>
#include <helpers/ESP32Board.h>
#include <driver/rtc_io.h>


class CardputerAdvBoard : public ESP32Board {
private:
public:
  CardputerAdvBoard() {}

  void begin() {
    ESP32Board::begin();

    auto cfg = M5.config();
    cfg.internal_mic = false;
    cfg.internal_imu = false;
    cfg.pmic_button = false;

    M5Cardputer.begin(cfg, true);
    M5Cardputer.Speaker.begin();

    esp_reset_reason_t reason = esp_reset_reason();
    if (reason == ESP_RST_DEEPSLEEP) {
      long wakeup_source = esp_sleep_get_ext1_wakeup_status();
      if (wakeup_source & (1 << P_LORA_DIO_1)) { // received a LoRa packet (while in deep sleep)
        startup_reason = BD_STARTUP_RX_PACKET;
      }

      rtc_gpio_hold_dis((gpio_num_t)P_LORA_NSS);
      rtc_gpio_deinit((gpio_num_t)P_LORA_DIO_1);
    }
  }

  void enterDeepSleep(uint32_t secs, int pin_wake_btn = -1) {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // Make sure the DIO1 and NSS GPIOs are hold on required levels during deep sleep
    rtc_gpio_set_direction((gpio_num_t)P_LORA_DIO_1, RTC_GPIO_MODE_INPUT_ONLY);
    rtc_gpio_pulldown_en((gpio_num_t)P_LORA_DIO_1);

    rtc_gpio_hold_en((gpio_num_t)P_LORA_NSS);

    if (pin_wake_btn < 0) {
      esp_sleep_enable_ext1_wakeup((1L << P_LORA_DIO_1),
                                   ESP_EXT1_WAKEUP_ANY_HIGH); // wake up on: recv LoRa packet
    } else {
      esp_sleep_enable_ext1_wakeup((1L << P_LORA_DIO_1) | (1L << pin_wake_btn),
                                   ESP_EXT1_WAKEUP_ANY_HIGH); // wake up on: recv LoRa packet OR wake btn
    }

    if (secs > 0) {
      esp_sleep_enable_timer_wakeup(secs * 1000000);
    }

    // Finally set ESP32 into sleep
    esp_deep_sleep_start(); // CPU halts here and never returns!
  }

  void enterLightSleep() {
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);

    // prevent speaker clicking after wakeup
    // begin() will automatically called after first sound played
    M5Cardputer.Speaker.end();

    // LoRa have active-high interrupt output, keyboard and user button have active-low,
    // so we have to use both ext0 and ext1
    esp_sleep_enable_ext0_wakeup((gpio_num_t)P_LORA_DIO_1, 1); // LoRa
    esp_sleep_enable_ext1_wakeup((1L << PIN_KEYBOARD_INT) | (1L << PIN_USER_BTN),
                                 ESP_EXT1_WAKEUP_ANY_LOW); // keyboard + G0

    Serial.flush();
    delay(10);
    Serial.end(); // USB CDC does not exits light sleep normally so turning it off

    esp_light_sleep_start();

    delay(10);
    Serial.begin(115200);
  }

  void powerOff() override { enterDeepSleep(0); }

  uint16_t getBattMilliVolts() override { return M5Cardputer.Power.getBatteryVoltage(); }

  void beep() { M5Cardputer.Speaker.tone(4000, 50); }

  const char *getManufacturerName() const override { return "Cardputer-ADV"; }
};
