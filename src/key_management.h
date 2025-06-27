#pragma once
#include <stdbool.h>

#include "types.h"

xxor_instance_t *load_file(const char *path);

bool validate_instance(const xxor_instance_t *instance);

xxor_instance_t *unsafe_load_file(FILE *file, char *path);

void free_xxor_key(xxor_instance_t *instance);

bool key_is_exhausted(const xxor_instance_t *instance);

uint8_t* read_key_bytes(xxor_instance_t *instance, size_t len, uint8_t* err_code);

void dbg_print_err_read_key_bytes(uint32_t err_code);

xxor_meta_t *gen_new_xxor_key(size_t len, uint8_t buffer[], fill_rand_buff_fn func);

bool save_new_xxor_key(const char* file_name, const uint8_t* buffer, const xxor_meta_t *meta);

bool update_saved_xxor_key(const xxor_instance_t *instance);