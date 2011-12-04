#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

enum MEDIA_TYPES    {
    MEDIA_IMAGE         =   0x00,
    MEDIA_MOVIE         =   0x01
};

typedef uint32_t    schedule_version_t;
typedef uint64_t    media_size_t;

#define AGENT_ID_LEN                128

typedef struct {
    uint8_t hash[128];
    uint8_t mediaType;
    media_size_t size;
    media_size_t offset;
} __attribute__((packed)) GetMediaDataCmd;

typedef struct {
    uint8_t hash[128];
    uint8_t mediaType;
} __attribute__((packed)) GetMediaSizeCmd;

#endif // TYPES_H
