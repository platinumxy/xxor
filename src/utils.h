#pragma once
#include <stdint.h>
#include <time.h>

static inline void sleep_ms(uint64_t millis) {
    struct timespec ts;
    ts.tv_sec = millis / 1000;
    ts.tv_nsec = (millis % 1000) * 1000000;
    nanosleep(&ts, NULL);
}
