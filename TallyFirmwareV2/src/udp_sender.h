#ifndef UDP_SENDER_H
#define UDP_SENDER_H

#include <stdint.h>

#include "accelerometer.h"

void udp_sender_init(const char *ip_address, uint16_t port);
void udp_sender_send(const accelerometer_readings_t *readings);

#endif
