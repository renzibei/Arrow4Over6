//
// Created by Fan Qu on 5/14/20.
//
#include "arrowutils.h"
#include <unistd.h>
#include <cstring>
#include <cerrno>

ssize_t readnBytes(int fd, void *buf, size_t nbyte) {
    size_t hasRead = 0;
    char* dest = (char*)buf;
    while(hasRead < nbyte) {
        ssize_t tempLen = read(fd, dest + hasRead, nbyte - hasRead);
        if(tempLen < 0) {
            LOGE("Error happens when read fd , %s", strerror(errno));
            return -1;
        }
        hasRead += tempLen;
    }
    return 0;
}

ssize_t writenBytes(int fd, const void *buf, size_t nbyte) {
    size_t hasWritten = 0;
    const char* src = (const char*)buf;
    while(hasWritten < nbyte) {
        ssize_t tempLen = write(fd, src + hasWritten, nbyte - hasWritten);
        if(tempLen < 0) {
            LOGE("Errors when write fd, %s", strerror(errno));
            return -1;
        }
        hasWritten += tempLen;
    }
    return 0;
}
