#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_spiffs.h"
#include "esp_timer.h"
#include "layer3.h"

#define SPIFFS_BASE       "/spiffs"
#define INPUT_FILE_NAME     "/input.raw"

static const char *TAG =  "mp3_encoder";

void app_main(void)
{
    esp_log_level_set("*", ESP_LOG_INFO);
    esp_log_level_set(TAG, ESP_LOG_DEBUG);
    esp_log_level_set("MP3SHINE", ESP_LOG_DEBUG);
    gpio_set_direction(GPIO_NUM_0, GPIO_MODE_INPUT);

    esp_vfs_spiffs_conf_t conf = {
        .base_path = SPIFFS_BASE,
        .partition_label = NULL,
        .max_files = 2,
        .format_if_mount_failed = true,
    };
    ESP_ERROR_CHECK(esp_vfs_spiffs_register(&conf));

    FILE *fp = fopen(SPIFFS_BASE INPUT_FILE_NAME, "rb");
    if (fp) {
        ESP_LOGI(TAG, "Encoding '%s'", INPUT_FILE_NAME);
    } else {
        ESP_LOGE(TAG, "unable to open filename '%s'", INPUT_FILE_NAME);
    }

    shine_config_t config;
    shine_set_config_mpeg_defaults(&config.mpeg);

    config.mpeg.bitr = 256;
    config.wave.samplerate = 48000;
    config.wave.channels = 2;
    config.mpeg.mode = config.wave.channels == 2? JOINT_STEREO : MONO;

    ESP_LOGI(TAG, "Bit rate %d", config.mpeg.bitr);
    ESP_LOGI(TAG, "Sample rate %d", config.wave.samplerate);
    ESP_LOGI(TAG, "Channels %d", config.wave.channels);

    shine_t s;
    if(!(s = shine_initialise(&config))) {
        ESP_LOGI(TAG, "Falied to initialize shine!");
        return;
    }

    //samples_per_pass 48k mono 1152 / stereo 1152
    //samples_per_pass 24k mono 576 / stereo 576
    size_t buf_size = shine_samples_per_pass(s)*config.wave.channels*sizeof(uint16_t);
    ESP_LOGI(TAG, "Buffer size %d", buf_size);
    int16_t *buf = malloc(buf_size);

    int rbytes = 0, wbytes = 0, written = 0, to_send = 0;
    uint64_t start = esp_timer_get_time();
    //Start loop to handle encoding
    do {
        while(fread(buf, buf_size, 1, fp) == 1) {
            rbytes += buf_size;
            //Encode buffer
            unsigned char *data = shine_encode_buffer_interleaved(s, buf, &written);
            if(to_send)
                fwrite(data, written, 1, stdout);
            else wbytes += written;
        }
        if(to_send) {
            fflush(stdout);
            printf("Sent %d bytes\n", to_send);
        } else {
            float eseconds = (esp_timer_get_time() - start)/1000000.f;
            float oseconds = rbytes/(float)(sizeof(int16_t)*config.wave.channels*config.wave.samplerate);
            ESP_LOGI(TAG, "Encoded %d bytes into %d bytes in %f seconds", rbytes, wbytes, eseconds);
            ESP_LOGI(TAG, "Compression ratio %.1f%%, encoding took %.1f%% of input time",
                wbytes*100.f/rbytes, eseconds*100.f/oseconds);
            for(int i = 1; i < 5; i++) ESP_LOGD(TAG, "T[%d]=%ld", i, shine_get_counters()[i]-shine_get_counters()[i-1]);
            printf("Ready to send mp3 stream if '>' is received or GPIO0 is low (BOOT is pressed)\n");
        }
        while(getchar() != '>' && gpio_get_level(GPIO_NUM_0));
        fprintf(stdout, "Sending %d bytes\n", to_send = wbytes);
        rewind(fp);
    } while(to_send);
    fclose(fp);
}
