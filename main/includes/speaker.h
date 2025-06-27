#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void spk_begin(void);
void spk_play(int16_t* src, size_t len_samples);
void spk_write(const void* buf, size_t size, size_t* bytes_written);

#ifdef __cplusplus
}
#endif
