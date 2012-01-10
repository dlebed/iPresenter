#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <stdint.h>

typedef enum {
    FILE_TYPE_IMAGE     =   0x01,
    FILE_TYPE_MOVIE     =   0x02
} FILE_TYPE;

enum AGENT_COMMANDS  {
    AGENT_GET_SCHEDULE_VERSION      =   0x00,
    AGENT_GET_SCHEDULE_DATA         =   0x01,
    AGENT_GET_MEDIA_SIZE            =   0x02,
    AGENT_GET_MEDIA_DATA            =   0x03
};

typedef uint64_t    media_size_t;
typedef uint32_t    schedule_version_t;


// Defines
#define PRESENTATION_VERSION_INVALID    0

#endif // TYPEDEFS_H
