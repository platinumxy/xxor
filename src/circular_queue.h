#pragma once
#include <stdbool.h>
#include <string.h> // ignore lsp used for memcpy

#define QUEUE_SIZE 0xFFFF

typedef struct {
    int head;
    int tail;
    int cnt;
} queue_meta_t;

#define queue_full(q) (q->meta.cnt == QUEUE_SIZE)
#define queue_empty(q) (q->meta.cnt == 0)
#define queue_next_pos(pos) (((pos) + 1) % QUEUE_SIZE)

#define DECLARE_QUEUE_TYPE(T, name) \
typedef struct CircularQueue_##name {\
    queue_meta_t meta; \
    T data[QUEUE_SIZE]; \
} CircularQueue_##name##_t; \
\
static CircularQueue_##name##_t* queue_##name##_init() {\
     CircularQueue_##name##_t* q = calloc(1, sizeof(CircularQueue_##name##_t));\
     return q;\
}\
\
static bool queue_##name##_push(CircularQueue_##name##_t *q, T *val) { \
    if (queue_full(q)) {  return false; }\
    memcpy(&q->data[q->meta.tail], val, sizeof(T)); \
    q->meta.tail = (q->meta.tail + 1) % QUEUE_SIZE; \
    q->meta.cnt += 1; \
    return true; \
} \
\
static bool queue_##name##_pop(CircularQueue_##name##_t *q ,T *result) { \
    if (queue_empty(q)) { return false; }\
    memcpy(result, &q->data[q->meta.head], sizeof(T)); \
    q->meta.head = (q->meta.head + 1) % QUEUE_SIZE; \
    q->meta.cnt -= 1; \
    return true; \
}

#define CQueue(name) CircularQueue_##name##_t
#define queue_pop(name, q, result) queue_##name##_pop(q, result)
#define queue_push(name, q, val) queue_##name##_push(q, val)
#define queue_init(name) queue_##name##_init()
