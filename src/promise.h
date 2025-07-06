#pragma once
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "utils.h"

typedef enum {
    PROMISE_PENDING,
    PROMISE_FULFILLED,
    PROMISE_REJECTED
} promise_state_t;

#define DECLARE_PROMISE_TYPE(T, name) \
typedef struct promise_##name { \
    promise_state_t state; \
    T value; \
} promise_##name##_t;\
\
static inline promise_##name##_t *new_##name##_promise() { \
    promise_##name##_t* prms = calloc(1, sizeof(T));\
    prms->state = PROMISE_PENDING;\
    return prms;\
}\
static inline bool promise_##name##_resolve(promise_##name##_t *prms) { \
    while(prms->state == PROMISE_PENDING) { \
        sleep_ms(10);\
    }\
    return prms->state == PROMISE_FULFILLED;\
}

#define promise_t(T) promise_##T##_t
#define new_promise(name) new_##name##_promise()
#define await_promise(name, prms) promise_##name##_resolve(prms)

DECLARE_PROMISE_TYPE(int, int)
DECLARE_PROMISE_TYPE(uint8_t*, u8_arr)
