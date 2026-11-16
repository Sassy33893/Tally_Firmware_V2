#include "udp_sender.h"

#include <errno.h>
#include <stddef.h>
#include <stdio.h>

#include "esp_log.h"
#include "lwip/inet.h"
#include "lwip/sockets.h"

static const char *TAG = "UDP";
static int sender_socket = -1;
static struct sockaddr_in destination;

void udp_sender_init(const char *ip_address, uint16_t port)
{
    sender_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if (sender_socket < 0) {
        ESP_LOGE(TAG, "Could not create UDP socket");
        return;
    }

    destination.sin_family = AF_INET;
    destination.sin_port = htons(port);
    destination.sin_addr.s_addr = inet_addr(ip_address);
}

void udp_sender_send(const accelerometer_readings_t *readings)
{
    static unsigned int send_failures;

    if (sender_socket < 0) {
        return;
    }

    char payload[128];
    int payload_length = snprintf(
        payload, sizeof(payload), "%.2f,%.2f,%.2f,%.2f,%.2f,%.2f",
        accelerometer_conversion(readings->vp),
        accelerometer_conversion(readings->vn),
        accelerometer_conversion(readings->d34),
        accelerometer_conversion(readings->d35),
        accelerometer_conversion(readings->d32),
        accelerometer_conversion(readings->d33));

    if (payload_length > 0 && (size_t)payload_length < sizeof(payload)) {
        int sent = sendto(sender_socket, payload, payload_length, 0,
                          (struct sockaddr *)&destination, sizeof(destination));
        if (sent < 0) {
            send_failures++;
            if (send_failures == 1 || send_failures % 100 == 0) {
                ESP_LOGW(TAG, "UDP send failed (errno %d)", errno);
            }
        } else {
            send_failures = 0;
        }
    }
}
