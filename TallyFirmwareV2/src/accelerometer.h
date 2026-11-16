#ifndef ACCELEROMETER_H
#define ACCELEROMETER_H

typedef struct {
    int vp;
    int vn;
    int d34;
    int d35;
    int d32;
    int d33;
} accelerometer_readings_t;

void accelerometer_init(void);
void accelerometer_read(accelerometer_readings_t *readings);
float accelerometer_conversion(int raw);
#endif
