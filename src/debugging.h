#pragma once
#include "types.h"

#define DEBUG

void show_err(const char *fmt, ...);

void print_xxor_key(xxor_instance_t *key);

bool already_open();
