#include "sdAudio.h"
#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "pico/audio_i2s.h"
#include "hardware/clocks.h"
#include "hardware/dma.h"
#include "sd_card.h"
#include "ff.h"
#include "pinout.h"

// WAV Header struct (moved here for consistency, though you already have it)
struct wave_file_header_t {
    char blockID[4];
    uint32_t totalSize;
    char typeHeader[4];
    char fmt[4];
    uint32_t headerLen;
    uint16_t typeOfFormat;
    uint16_t numbeOfChannel;
    uint32_t sampleRate;
    uint32_t byteRate;
    uint16_t blockAlign;
    uint16_t bitsPerSample;
    char dataHeader[4];
    uint32_t dataSize;
} wave_file_header;

// Globals for SD card playback state
static FIL sd_audio_file;
static audio_buffer_pool_t *producer_pool = NULL;
static volatile bool sd_audio_playing = false;

static void stopPlayback()
{
    audio_i2s_deinit();
}

// Your main function to start playing a WAV file
void sdAudioPlayFile(char *path) {
    printf("SdAudio: Playing %s\n", path);

    // Stop any current playback first
    if (sd_audio_playing) {
        stopPlayback();
    }

    FRESULT fr = f_open(&sd_audio_file, path, FA_READ);
    if (fr != FR_OK) {
        printf("SdAudio: Failed to open file %s, error: %d\n", path, fr);
        return;
    }

    UINT br;
    fr = f_read(&sd_audio_file, &wave_file_header, 44, &br); // Read WAV header
    if (fr != FR_OK || br != 44) {
        printf("SdAudio: Failed to read WAV header or header too short, error: %d\n", fr);
        f_close(&sd_audio_file);
        return;
    }

    // Basic WAV header validation
    if (strncmp(wave_file_header.blockID, "RIFF", 4) != 0 ||
        strncmp(wave_file_header.typeHeader, "WAVE", 4) != 0 ||
        strncmp(wave_file_header.fmt, "fmt ", 4) != 0) {
        printf("SdAudio: Not a valid WAV file format.\n");
        f_close(&sd_audio_file);
        return;
    }
    printf("WAV: Sample Rate: %lu, Channels: %u, Bits per Sample: %u\n",
           wave_file_header.sampleRate, wave_file_header.numbeOfChannel, wave_file_header.bitsPerSample);

    static audio_format_t audio_format = {0};
    audio_format.format = AUDIO_BUFFER_FORMAT_PCM_S16;
    audio_format.sample_freq = wave_file_header.sampleRate;
    audio_format.channel_count = wave_file_header.numbeOfChannel;

    static struct audio_buffer_format producer_format = {
            .format = &audio_format,
            .sample_stride = 2
    };

    producer_pool = audio_new_producer_pool(&producer_format, 2, SAMPLES_PER_BUFFER);
    const struct audio_format *output_format;
    
    // Configure I2S
    struct audio_i2s_config config = {
        .data_pin = I2S_PIO_DIN,
        .clock_pin_base = I2S_PIO_BLK,
        .dma_channel = 3,
        .pio_sm = I2S_PIO_SM,
    };
    
    output_format = audio_i2s_setup(&audio_format, &config);
    if (!output_format) {
        panic("PicoAudio: Unable to open audio device.\n");
    }

    bool ok = audio_i2s_connect(producer_pool);
    assert(ok);
    audio_i2s_set_enabled(true);
    sd_audio_playing = true; // Set flag to start processing in the main loop
}

// This function will be called repeatedly in main loop
void sdAudioProcess() {
    if (!sd_audio_playing || NULL == producer_pool) {
        return; // Not playing, do nothing
    }

    // Try to get a free audio buffer non-blocking
    struct audio_buffer *buffer = take_audio_buffer(producer_pool, false);
    if (buffer) {
        UINT br;
        // Read data into the buffer's bytes
        FRESULT fr = f_read(&sd_audio_file, buffer->buffer->bytes, buffer->max_sample_count * (wave_file_header.bitsPerSample / 8), &br);
        
        if (fr != FR_OK || br == 0) {
            // End of file or read error
            f_close(&sd_audio_file);
            stopPlayback();
            sd_audio_playing = false;
            printf("SdAudio: Playback finished or error.\n");
            return;
        }

        buffer->sample_count = br / (wave_file_header.bitsPerSample / 8);
        give_audio_buffer(producer_pool, buffer);
    }

    if (!sd_audio_playing) {
        stopPlayback();
    }
    // If buffer is NULL, it means no buffer is currently available,
    // so we wait for the next iteration of the loop. This is non-blocking.
}