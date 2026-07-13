#include "audio_out.h"

#include <driver/i2s.h>

#include "../config.h"

AudioOut::AudioOut(uint8_t i2s_port, uint8_t bclk_pin, uint8_t lrclk_pin,
                   uint8_t dout_pin)
    : port_(i2s_port),
      bclk_pin_(bclk_pin),
      lrclk_pin_(lrclk_pin),
      dout_pin_(dout_pin) {}

bool AudioOut::begin() {
  i2s_config_t cfg = {};
  cfg.mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX);
  cfg.sample_rate = config::kAudioSampleRateHz;
  cfg.bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT;
  cfg.channel_format = I2S_CHANNEL_FMT_ONLY_LEFT;
  cfg.communication_format = I2S_COMM_FORMAT_STAND_I2S;
  cfg.intr_alloc_flags = ESP_INTR_FLAG_LEVEL1;
  cfg.dma_buf_count = 4;
  cfg.dma_buf_len = 256;
  cfg.use_apll = false;
  cfg.tx_desc_auto_clear = true;  // underruns play silence, not garbage

  const i2s_port_t port = static_cast<i2s_port_t>(port_);
  if (i2s_driver_install(port, &cfg, 0, nullptr) != ESP_OK) {
    return false;
  }

  i2s_pin_config_t pins = {};
  pins.mck_io_num = I2S_PIN_NO_CHANGE;
  pins.bck_io_num = bclk_pin_;
  pins.ws_io_num = lrclk_pin_;
  pins.data_out_num = dout_pin_;
  pins.data_in_num = I2S_PIN_NO_CHANGE;
  if (i2s_set_pin(port, &pins) != ESP_OK) {
    i2s_driver_uninstall(port);
    return false;
  }

  ready_ = true;
  return true;
}

size_t AudioOut::write(const int16_t* samples, size_t count) {
  if (!ready_ || count == 0) {
    return 0;
  }
  size_t written_bytes = 0;
  i2s_write(static_cast<i2s_port_t>(port_), samples, count * sizeof(int16_t),
            &written_bytes, 0 /* non-blocking */);
  return written_bytes / sizeof(int16_t);
}
