#include "sampler.h"

#include "accelerometer.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

static const char *TAG = "SAMPLER";

static session_t session; // 48 kB ring in .bss
static SemaphoreHandle_t session_mutex;
static TaskHandle_t sampler_task_handle;

session_t *sampler_lock(void)
{
    xSemaphoreTake(session_mutex, portMAX_DELAY);
    return &session;
}

void sampler_unlock(void)
{
    xSemaphoreGive(session_mutex);
}

// Runs in the esp_timer task. Only wakes the sampling task, so the ADC work happens on core 1.
static void on_tick(void *arg)
{
    (void)arg;
    xTaskNotifyGive(sampler_task_handle);
}

static void sampler_task(void *arg)
{
    (void)arg;
    uint16_t mv[WIRE_CHANNELS];
    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        // One timestamp per tick for all six channels; a late wake shows up as a >15 ms gap.
        int64_t now_us = esp_timer_get_time();
        session_t *s = sampler_lock();
        bool running = s->state == SESSION_RUNNING;
        sampler_unlock();
        if (!running) continue;
        accelerometer_read_mv(mv);
        s = sampler_lock();
        session_state_t before = s->state;
        session_tick(s, now_us, mv);
        bool ended = before == SESSION_RUNNING && s->state == SESSION_ENDING;
        uint32_t set_id = s->set_id, taken = s->taken;
        uint8_t reason = s->end_reason;
        sampler_unlock();
        if (ended)
            ESP_LOGI(TAG, "set %lu ended, reason %u after %lu samples",
                     (unsigned long)set_id, reason, (unsigned long)taken);
    }
}

void sampler_start(void)
{
    session_mutex = xSemaphoreCreateMutex();
    configASSERT(session_mutex);
    session_reset(&session);
    BaseType_t created = xTaskCreatePinnedToCore(sampler_task, "sampler", 4096, NULL,
                                                 configMAX_PRIORITIES - 2, &sampler_task_handle, 1);
    configASSERT(created == pdPASS);

    const esp_timer_create_args_t timer_args = {
        .callback = on_tick,
        .name = "sample_tick",
    };
    esp_timer_handle_t timer;
    ESP_ERROR_CHECK(esp_timer_create(&timer_args, &timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(timer, WIRE_SAMPLE_PERIOD_US));
}
