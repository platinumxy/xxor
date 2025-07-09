#include "debugging.h"
#include "types.h"

#include <stdio.h>
#include <stdarg.h>
#include <dirent.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

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
    show_err("  Key Size: %lu bytes\n", (uint64_t) key->meta->key_size);
    show_err("  Used Key Count: %lu\n", (uint64_t) key->meta->used_key_cnt);
    show_err("  File Name: %s\n", key->file_name);
}

bool already_open() {
    // returns if an
    DIR *d = opendir("/proc");
    if (!d) return -1;

    pid_t self = getpid();
    char self_exe[PATH_MAX];
    ssize_t n = readlink("/proc/self/exe", self_exe, sizeof(self_exe) - 1);
    if (n < 0) return -1;
    self_exe[n] = 0;

    struct dirent *e;
    while ((e = readdir(d))) {
        if (!isdigit(e->d_name[0])) continue;
        pid_t pid = atoi(e->d_name);
        if (pid == self) continue;

        char path[64];
        snprintf(path, sizeof(path), "/proc/%d/exe", pid);

        char exe[PATH_MAX];
        ssize_t len = readlink(path, exe, sizeof(exe) - 1);
        if (len < 0) continue;
        exe[len] = 0;

        const char *name = strrchr(exe, '/');
        if (name && strcmp(name + 1, "xxor") == 0) {
            closedir(d);
            return 1;
        }
    }

    closedir(d);
    return 0;
}
