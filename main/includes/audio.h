#pragma once
#include <stddef.h>  // for size_t

#include "esp_err.h"            // for esp_err_t
#include "freertos/FreeRTOS.h"  // for TickType_t

#ifdef __cplusplus
extern "C" {
#endif
typedef esp_err_t (
    *audio_write_cb_t)(int16_t* buf, size_t size, size_t* bytes_written);

typedef int (*audio_send_cb_t)(const uint8_t* buf, size_t size);

void init_audio_decoder();
void audio_decode(uint8_t* data, size_t size, audio_write_cb_t i2c_write_cb);

#ifdef __cplusplus
}
#endif
