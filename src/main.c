#include <stdio.h>
#include <stdlib.h>
#include <sys/random.h>
#include <stdbool.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>


#include "main.h"
#include "utils.h"
#include "promise.h"
#include "network_manager.h"
#include "circular_queue.h"

bool dev_rand_fill(const size_t len, uint8_t *buffer) {
    getrandom(buffer, len, 0);
    return true;
}

DECLARE_QUEUE_TYPE(int, int)

void CircularQueue_test() {
    CQueue(int) *q = queue_init(int);
    int i = 0;
    while (queue_push(int, q, &i)) {
        i++;
    }


    while (queue_pop(int, q, &i)) {
        printf("%d ", i);
    }
    printf("\n\n");
    i = 0xFF;
    while (queue_push(int, q, &i)) {
        i--;
    }

    while (queue_pop(int, q, &i)) {
        printf("%d ", i);
    }
}


void *promise_testing_resolve_later(void *args) {
    promise_t(int) *prms = (promise_t(int) *) args;
    sleep(3);
    prms->value = 123451;
    prms->state = PROMISE_FULFILLED;
    return NULL;
}

void promise_testing() {
    promise_t(int) *prms = new_promise(int);
    pthread_t thread;
    pthread_create(&thread, NULL, promise_testing_resolve_later, prms);
    printf("Awaiting...\n");
    fflush(stdout);
    await_promise(int, prms);
    printf("%d\n", prms->value);
    if (prms->state == PROMISE_FULFILLED) {
        printf("FULFILLED\n");
    } else {
        printf("UNFULFILLED\n");
    }
}


int main(int argc, char *argv[]) {
    promise_testing();
}
