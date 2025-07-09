#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>


static const uint8_t XOR_KEY_MAGIC[] = {0x58, 0x58, 0x4F, 0x52}; // "XXOR"
static const uint8_t XXOR_Version = 0xFF;

#pragma pack(push)
#pragma pack(1)
typedef struct xxor_meta {
    uint8_t magic_bytes[8];
    uint8_t version;
    uint64_t key_size;
    uint64_t used_key_cnt;
} xxor_meta_t;

#define METADATA_SIZE (sizeof(xxor_meta_t))

#pragma pack(pop)


typedef struct xxor_instance {
    xxor_meta_t *meta;
    char *file_name;
    FILE *file;
} xxor_instance_t;


typedef bool (*fill_rand_buff_fn)(size_t, uint8_t *);

typedef struct u8_arr {
    uint8_t *data;
    uint32_t size;
} u8_arr_t;
