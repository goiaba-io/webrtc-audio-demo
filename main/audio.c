#include "audio.h"

#include <opus.h>
#include <stdatomic.h>
#include <string.h>

#include "esp_log.h"

#define OPUS_SAMPLE_RATE 8000

#define OPUS_OUT_BUFFER_SIZE 1276

#define OPUS_ENCODER_BITRATE 30000
#define OPUS_ENCODER_COMPLEXITY 0

#define READ_BUFFER_SIZE (2560)
#define RESAMPLE_BUFFER_SIZE ((READ_BUFFER_SIZE / 4) / 2)

static uint8_t encode_resample_buffer[RESAMPLE_BUFFER_SIZE];
static int16_t decode_resample_buffer[READ_BUFFER_SIZE];

static const char *TAG = "audio";

void convert_int32_to_int16_and_downsample(int32_t *in, int16_t *out,
    size_t count) {
    for (size_t j = 0, i = 0; i < count; i += 4, j++) {
        int32_t s = in[i] >> 16;
        if (s > INT16_MAX) s = INT16_MAX;
        if (s < INT16_MIN) s = INT16_MIN;
        out[j] = (int16_t)s;
    }
}

bool is_playing = false;
void convert_16k8_to_32k32(int16_t *in_buf, size_t in_samples,
    int32_t *out_buf) {
    size_t out_index = 0;
    bool any_set = false;
    for (size_t i = 0; i < in_samples; i++) {
        if (in_buf[i] != -1 && in_buf[i] != 0) {
            any_set = true;
        }

        int32_t s = ((int32_t)in_buf[i]) << 16;
        out_buf[out_index++] = s;
        out_buf[out_index++] = s;
        out_buf[out_index++] = s;
        out_buf[out_index++] = s;
    }
    is_playing = any_set;
}

opus_int16 *output_buffer = NULL;
OpusDecoder *opus_decoder = NULL;

void init_audio_decoder() {
    int decoder_error = 0;
    opus_decoder = opus_decoder_create(OPUS_SAMPLE_RATE, 1, &decoder_error);
    if (decoder_error != OPUS_OK) {
        printf("Failed to create OPUS decoder");
        return;
    }

    output_buffer = (opus_int16 *)malloc(RESAMPLE_BUFFER_SIZE);
}

void audio_decode(uint8_t *data, size_t size, audio_write_cb_t i2c_write_cb) {
    int decoded_samples = opus_decode(opus_decoder,
        data,
        size,
        output_buffer,
        RESAMPLE_BUFFER_SIZE,
        0);

    if (decoded_samples > 0) {
        // convert_16k8_to_32k32((int16_t *)output_buffer,
        //     RESAMPLE_BUFFER_SIZE / sizeof(int16_t),
        //     (int32_t *)decode_resample_buffer);

        size_t bytes;
        esp_err_t err =
            i2c_write_cb((int16_t *)output_buffer, READ_BUFFER_SIZE, &bytes);
        if (err != ESP_OK) {
            ESP_LOGE(TAG,
                "audio write callback failed: %s",
                esp_err_to_name(err));
        }
    }
}

OpusEncoder *opus_encoder = NULL;
uint8_t *encoder_output_buffer = NULL;

void init_audio_encoder() {
    int encoder_error;
    opus_encoder = opus_encoder_create(OPUS_SAMPLE_RATE,
        1,
        OPUS_APPLICATION_VOIP,
        &encoder_error);
    if (encoder_error != OPUS_OK) {
        printf("Failed to create OPUS encoder");
        return;
    }

    if (opus_encoder_init(opus_encoder,
            OPUS_SAMPLE_RATE,
            1,
            OPUS_APPLICATION_VOIP) != OPUS_OK) {
        printf("Failed to initialize OPUS encoder");
        return;
    }

    opus_encoder_ctl(opus_encoder, OPUS_SET_BITRATE(OPUS_ENCODER_BITRATE));
    opus_encoder_ctl(opus_encoder,
        OPUS_SET_COMPLEXITY(OPUS_ENCODER_COMPLEXITY));
    opus_encoder_ctl(opus_encoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE));
    encoder_output_buffer = (uint8_t *)malloc(OPUS_OUT_BUFFER_SIZE);
}

void decode_audio(uint8_t *read_buffer, size_t bytes_read,
    audio_send_cb_t audio_send_cb) {
    if (is_playing) {
        memset(read_buffer, 0, bytes_read);
    }
    convert_int32_to_int16_and_downsample((int32_t *)&read_buffer,
        (int16_t *)&encode_resample_buffer,
        READ_BUFFER_SIZE / sizeof(uint32_t));
    int encoded_size = opus_encode(opus_encoder,
        (const opus_int16 *)encode_resample_buffer,
        RESAMPLE_BUFFER_SIZE / sizeof(uint16_t),
        encoder_output_buffer,
        OPUS_OUT_BUFFER_SIZE);
    if (encoded_size < 0) {
        ESP_LOGE(TAG,
            "Failed to encode audio: %s",
            opus_strerror(encoded_size));
    } else {
        audio_send_cb(encoder_output_buffer, encoded_size);
    }
}
