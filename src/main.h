#pragma once
#include "types.h"

xxor_instance_t *load_file(const char *path);

bool validate_instance(const xxor_instance_t *instance);

xxor_instance_t *unsafe_load_file(FILE *file, const char *path);

void free_xxor_key(xxor_instance_t *instance);
