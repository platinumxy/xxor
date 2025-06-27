#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/random.h>
#include <stdbool.h>

#include "main.h"

#include "debugging.h"
#include "key_management.h"


bool dev_rand_fill(const size_t len, uint8_t* buffer) {
    getrandom(buffer, len, 0);
    return true;
}

int main(int argc, char *argv[]) {    const char *file_name = "../../testing/example.key\x00";
    size_t len = 0xAAAA;
    uint8_t *otp_buffer = calloc(len, sizeof(uint8_t));
    xxor_meta_t* metadata =  gen_new_xxor_key(len, otp_buffer, dev_rand_fill);
    if (save_new_xxor_key(file_name, otp_buffer, metadata)) {
        printf("Saved the key\n");
    } else {
        printf("Failed to save the key\n");
    }
    free(metadata);
    free(otp_buffer);

    xxor_instance_t* instance = load_file(file_name);
    printf("Loaded the instance\n");
    print_xxor_key(instance);

    for (int j = 0; j < 10; j++) {
        uint8_t err;
        uint8_t* buff = read_key_bytes(instance, 10, &err);
        if (buff == NULL) {
            printf("Failed to read key\n");
            dbg_print_err_read_key_bytes(err);
        } else {
            for (int i = 0; i < 10; i++) {
                printf("%02x ", buff[i]);
            }
            printf("\n");
        }
        free(buff);
    }
    print_xxor_key(instance);
    free_xxor_key(instance);
} 