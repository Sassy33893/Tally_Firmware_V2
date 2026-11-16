#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "accelerometer.h"
#include "udp_sender.h"
#include "wifi_ap.h"


#define IP_ADDRESS "192.168.4.2"
#define PORT 5198

void app_main(void)
{
    accelerometer_init();
    wifi_ap_start();
    udp_sender_init(IP_ADDRESS, PORT);

    int log_counter = 0;
    while (1) {
        accelerometer_readings_t readings;
        accelerometer_read(&readings);
        udp_sender_send(&readings);

        if (++log_counter >= 100) {
            ESP_LOGI("ACCEL", "VP=%.2f g VN=%.2f g D34=%.2f g D35=%.2f g D32=%.2f g D33=%.2f g",
                     accelerometer_conversion(readings.vp),
                     accelerometer_conversion(readings.vn),
                     accelerometer_conversion(readings.d34),
                     accelerometer_conversion(readings.d35),
                     accelerometer_conversion(readings.d32),
                     accelerometer_conversion(readings.d33));
            log_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}