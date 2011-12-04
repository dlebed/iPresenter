#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

enum MEDIA_TYPES    {
    MEDIA_IMAGE         =   0x01,
    MEDIA_MOVIE         =   0x02
};

enum AGENT_COMMANDS  {
    AGENT_GET_SCHEDULE_VERSION      =   0x00,
    AGENT_GET_SCHEDULE_DATA         =   0x01,
    AGENT_GET_MEDIA_SIZE            =   0x02,
    AGENT_GET_MEDIA_DATA            =   0x03
};

typedef uint32_t    schedule_version_t;
typedef uint64_t    media_size_t;

#define AGENT_ID_LEN                128



#endif // TYPES_H
