

#ifndef _OSA_FILE_H_
#define _OSA_FILE_H_

#include <osa.h>

int OSA_fileReadFile(char *fileName, Uint8 *addr, Uint32 readSize, Uint32 *actualReadSize);
int OSA_fileWriteFile(char *fileName, Uint8 *addr, Uint32 size);
int OSA_fileWriteVideoData(char *fileName, Uint8 *Yaddr, Uint8 *UVaddr, Uint32 width, Uint32 height, Uint32 pitch);

#define VI_DEBUG_MODE

#ifdef VI_DEBUG_MODE
#define VI_DEBUG(...) \
    do \
    { \
        fprintf(stderr, " DEBUG (%s|%s|%d): ", \
                __FILE__, __func__, __LINE__); \
        fprintf(stderr, __VA_ARGS__); \
    } \
    while(0)

#else
#define VI_DEBUG(...)
#endif

#endif /* _OSA_FILE_H_ */



