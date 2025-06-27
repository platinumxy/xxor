#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdbool.h>

#include "main.h"

#include "key_management.h"


bool dev_rand_fill(const size_t len, uint8_t* buffer) {
    getrandom(buffer, len, 0);
    return true;
}

int main(int argc, char *argv[]) {    const char *file_name = "../../testing/example.key\x00";
    size_t len = 0xAAAA;
    uint8_t *otp_buffer = malloc(len);
    xxor_meta_t* metadata =  gen_new_xxor_key(len, otp_buffer, dev_rand_fill);
    if (save_new_xxor_key(file_name, otp_buffer, metadata)) {
        printf("Saved the key\n");
    } else {
        printf("Failed to save the key\n");
    }
} 