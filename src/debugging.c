#include "debugging.h"
#include "types.h"

#include <stdio.h>
#include <stdarg.h>

void show_err(const char *fmt, ...) {
#ifdef DEBUG
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
#endif
}


void print_xxor_key(xxor_instance_t *key) {
    if (!key || !key->meta) {
        show_err("Invalid key structure.\n");
        return;
    }

    show_err("XXOR Key Metadata:\n");
    show_err("  Version: %d\n", key->meta->version);
    show_err("  Key Size: %lu bytes\n", (uint64_t)key->meta->key_size);
    show_err("  Used Key Count: %lu\n", (uint64_t)key->meta->used_key_cnt);
    show_err("  File Name: %s\n", key->file_name);
}
