#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "accelerometer.h"

void app_main(void)
{
    accelerometer_init();
    while (1) {
        accelerometer_readings_t readings;
        accelerometer_read(&readings);
        ESP_LOGI("ACCEL", "VP=%.2f g VN=%.2f g D34=%.2f g D35=%.2f g D32=%.2f g D33=%.2f g",
                 accelerometer_conversion(readings.vp),
                 accelerometer_conversion(readings.vn),
                 accelerometer_conversion(readings.d34),
                 accelerometer_conversion(readings.d35),
                 accelerometer_conversion(readings.d32),
                 accelerometer_conversion(readings.d33));
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}