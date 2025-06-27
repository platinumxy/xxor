#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>

#include "key_management.h"
#include "debugging.h"
#include "types.h"

xxor_instance_t* load_file(const char *path) {
    FILE *file = fopen(path, "rb+");
    if (!file) {
        show_err("Failed to open file\n");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    uint64_t file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    if (file_size < sizeof(xxor_instance_t) + 1 ) {
        show_err("File is too small\n");
        return NULL;
    }

    xxor_instance_t *instance = unsafe_load_file(file, path);
    if (instance == NULL || !validate_instance(instance)) {
        show_err("Unknown issue occurred when reading the key\n");
        free_xxor_key(instance);
        return NULL;
    }
    return instance;
}

bool validate_instance(const xxor_instance_t *instance) {
    if (instance == NULL || instance->meta == NULL) { return false; }
    if (access(instance->file_name, F_OK) != 0) { return false; }

    const uint32_t fpos = ftell(instance->file);
    fseek(instance->file, 0, SEEK_END);
    const uint64_t len = ftell(instance->file) - METADATA_SIZE;
    fseek(instance->file, fpos, SEEK_SET);

    if (instance->meta->key_size > len) { return false; }

    if (key_is_exhausted(instance)) { return false; }

    return true;
}

xxor_instance_t* unsafe_load_file(FILE *file, const char *path) {
    xxor_meta_t *meta = calloc(1, METADATA_SIZE);
    fseek(file, 0, SEEK_SET);
    if (fread(meta, 1, METADATA_SIZE, file) != METADATA_SIZE) {
        show_err("Failed to read file\n");
        free(meta);
        return NULL;
    }

    xxor_instance_t *instance = calloc(1, sizeof(xxor_instance_t));
    instance->meta = meta;
    instance->file_name = path;
    instance->file = file;

    return instance;
}

bool key_is_exhausted(const xxor_instance_t *instance) {
    return instance->meta->used_key_cnt >= instance->meta->key_size;
}

void free_xxor_key(xxor_instance_t *instance) {
    if (instance == NULL) { return; }
    free(instance->meta);
    fclose(instance->file);
    free(instance);
}

char update_saved_xxor_key(xxor_instance_t *key) {
    printf("TODO");
    return 0;
}

uint8_t* read_key_bytes(xxor_instance_t *instance, size_t len, uint8_t* err_code) {
    if (!(instance && instance->file && instance->meta)) {
        *err_code = 1 << 0;
        return NULL;
    }

    if (len + instance->meta->used_key_cnt  > instance->meta->key_size) {
        *err_code = 1 << 1;
        return NULL;
    }

    uint8_t *buffer = malloc(len);
    if (!buffer) {
        *err_code = 1 << 2;
        return NULL;
    }

    if (fread(buffer, sizeof(uint8_t), len, instance->file) != len) {
        free(buffer);
        *err_code = 1 << 3;
        return NULL;
    }

    instance->meta->used_key_cnt += len;

    if (update_saved_xxor_key(instance) != 1) {
        free(buffer);
        *err_code = 1 << 4;
        return NULL;
    }

    return buffer;
}

void dbg_print_err_read_key_bytes(uint32_t err_code) {
    switch (err_code) {
        case 1 << 0:
            show_err("Invalid key structure or file.\n");
            break;
        case 1 << 1:
            show_err("Requested amount exceeds key size.\n");
            break;
        case 1 << 2:
            show_err("Failed to allocate memory for reading bytes\n");
            break;
        case 1 << 3:
            show_err("Failed to read bytes from file\n");
            break;
        case 1 << 4:
            show_err("Failed to update saved key\n");
            break;
        default:
            show_err("Unknown error: %d\n", err_code);
    }
}

xxor_meta_t *gen_new_xxor_key(size_t len, uint8_t* buffer, fill_rand_buff_fn func) {
    if (func == NULL || buffer == NULL) { return NULL;}
    xxor_meta_t *meta = calloc(1, sizeof(xxor_meta_t));

    if (!func(len, buffer)) {
        show_err("Unable to gen %d random numbers", len);
        return NULL;
    }

    memcpy(meta->magic_bytes, XOR_KEY_MAGIC, sizeof(XOR_KEY_MAGIC));
    meta->version = XXOR_Version;
    meta->key_size = len;
    meta->used_key_cnt = 0;

    return meta;
}

bool save_new_xxor_key(const char* file_name, const uint8_t* buffer, const xxor_meta_t *meta) {
    FILE *file = fopen(file_name, "w+b");
    if (!file) { return false; }

    if (fwrite(meta, 1, METADATA_SIZE, file) != METADATA_SIZE) {
        fclose(file);
        return false;
    }

    if (fwrite(buffer, 1, meta->key_size, file) != meta->key_size) {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}
