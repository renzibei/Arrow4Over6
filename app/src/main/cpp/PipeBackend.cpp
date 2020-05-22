//
// Created by Fan Qu on 5/14/20.
//
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include "PipeBackend.h"
#include "arrowutils.h"

#define TO_PIPE_FRONT_FILE_NAME "ToPipeFront"
#define FROM_PIPE_FRONT_FILE_NAME "FromPipeFront"

//PipeBackend* PipeBackend::_instance = nullptr;
char* PipeBackend::pipeDirPath = nullptr;
//const char* PipeBackend::toPipeFrontPath =  "ToPipeFront";
//const char* PipeBackend::fromPipeFrontPath = "FromPipeFront";
std::mutex PipeBackend::readMutex, PipeBackend::writeMutex;

PipeBackend::PipeBackend():
fromFrontEndFd(0), toFrontEndFd(0), toPipeFrontPath(nullptr), fromPipeFrontPath(nullptr) {
    initPipeName(pipeDirPath);
    if(makePipe() != 0) {
        return;
    }
    if(openPipe() != 0) {
        return;
    }

}

PipeBackend::~PipeBackend() {
    if(toPipeFrontPath != nullptr) {
        delete toPipeFrontPath;
    }
    if(fromPipeFrontPath != nullptr) {
        delete fromPipeFrontPath;
    }
    if(pipeDirPath != nullptr) {
        delete pipeDirPath;
        pipeDirPath = nullptr;
    }
}

void PipeBackend::setPipeDirPath(const char *dirPath) {
    size_t dirPathLen = strlen(dirPath);
    pipeDirPath = new char[dirPathLen + 1];
    memcpy(pipeDirPath, dirPath, dirPathLen + 1);
}

void PipeBackend::initPipeName(const char *dirPath) {
    if(dirPath == nullptr) {
        LOGE("dirPath is null");
        exit(EXIT_FAILURE);
    }
    size_t dirPathLen = strlen(dirPath), toPipeFrontNameLen = strlen(TO_PIPE_FRONT_FILE_NAME), fromPipeFrontNameLen = strlen(FROM_PIPE_FRONT_FILE_NAME);
    size_t toPipeFrontPathLen = dirPathLen + toPipeFrontNameLen;
    size_t fromPipeFrontPathLen = dirPathLen + fromPipeFrontNameLen;
    this->toPipeFrontPath = new char[toPipeFrontPathLen + 2];
    memcpy(this->toPipeFrontPath, dirPath, dirPathLen);
    this->toPipeFrontPath[dirPathLen] = '/';
    memcpy(this->toPipeFrontPath + dirPathLen + 1, TO_PIPE_FRONT_FILE_NAME, toPipeFrontNameLen + 1);
    this->fromPipeFrontPath = new char[fromPipeFrontPathLen + 2];
    memcpy(this->fromPipeFrontPath, dirPath, dirPathLen);
    this->fromPipeFrontPath[dirPathLen] = '/';
    memcpy(this->fromPipeFrontPath + dirPathLen + 1, FROM_PIPE_FRONT_FILE_NAME, fromPipeFrontNameLen + 1);
    LOGI("toPipeFrontPath is %s", toPipeFrontPath);
    LOGI("fromPipeFrontPath is %s", fromPipeFrontPath);
}

int PipeBackend::readBytes(char* buf, int bufLen) {
    int readLen = 0;
    int tempRet = readLength(readLen);
    if(tempRet != 0) {
        return -1;
    }
    if(readLen > bufLen) {
        LOGE("target read length greater than buffer length");
        return -1;
    }
    std::lock_guard<std::mutex> lockGuard(PipeBackend::readMutex);
    int hasRead = 0;
    while(hasRead < readLen) {
        int tempReadLen = read(this->fromFrontEndFd, buf, readLen - hasRead);
        if(tempReadLen < 0) {
            LOGE("Fail to readBytes, %s", strerror(errno));
            return -1;
        }
        hasRead += tempReadLen;
    }
    assert(hasRead == readLen);
    return readLen;
}

int PipeBackend::readBytes() {
    int readLen = 0;
    int tempRet = readLength(readLen);
    if(tempRet != 0) {
        return -1;
    }
    std::lock_guard<std::mutex> lockGuard(PipeBackend::readMutex);
    int hasRead = 0;
    while(hasRead < readLen) {
        int tempReadLen = read(this->fromFrontEndFd, this->readBuffer, readLen - hasRead);
        if(tempReadLen < 0) {
            LOGE("Fail to readBytes, %s", strerror(errno));
            return -1;
        }
        hasRead += tempReadLen;
    }
    return readLen;
}

int PipeBackend::readLength(int &len) {
    std::lock_guard<std::mutex> lockGuard(PipeBackend::readMutex);
    int readLen = 0;
    int tempRet = read(this->fromFrontEndFd, &readLen, sizeof(readLen));
    if(tempRet != 4) {
        LOGE("Fail to read readLen, %s", strerror(errno));
        return -1;
    }
    if(readLen == 0) {
        LOGE("readLen equals zero, %s", strerror(errno));
        return -1;
    }
    len = readLen;
    return 0;
}

int PipeBackend::readInt(int &x) {
    int readLen = 0;
    int tempRet = readLength(readLen);
    if(tempRet != 0) {
        return -1;
    }
    assert(readLen == 4);
    tempRet = readLength(x);
    if(tempRet != 0) {
        return -1;
    }
    return 0;
}

int PipeBackend::writeLength(int len) {
    std::lock_guard<std::mutex> lockGuard(PipeBackend::writeMutex);
    int tempRet = write(this->toFrontEndFd, &len, sizeof(len));
    if(tempRet != 4) {
        LOGE("Fail to write begin len, %s", strerror(errno));
        return -1;
    }
    return 0;
}

int PipeBackend::writeInt(int dataInt) {
    int tempRet = writeLength(sizeof(int));
    if(tempRet != 0) {
        return -1;
    }
    tempRet = writeLength(dataInt);
    if(tempRet != 0) {
        return -1;
    }
    return 0;
}

int PipeBackend::writeBytes(const char* msg, int len) {
    int tempRet = writeLength(len);
    if(tempRet < 0) {
        return -1;
    }
    std::lock_guard<std::mutex> lockGuard(PipeBackend::writeMutex);
    int hasWritten = 0;
    while(hasWritten < len) {
        int tempWrite = write(this->toFrontEndFd, msg + hasWritten, (size_t)(len - hasWritten));
        if(tempWrite < 0) {
            LOGE("Fail to writeBytes, %s", strerror(errno));
            return -1;
        }
        hasWritten += tempWrite;
    }
    return 0;
}



int PipeBackend::openPipe() {
    int fromFrontFd = open(fromPipeFrontPath, O_RDWR | O_TRUNC);
    if(fromFrontFd == -1) {
        LOGE("Fail to open %s, %s", fromPipeFrontPath, strerror(errno));
    }
    int toFrontFd = open(toPipeFrontPath, O_RDWR | O_TRUNC);
    if(toFrontFd == -1) {
        LOGE("Fail to open %s, %s", toPipeFrontPath, strerror(errno));
    }
    if(fromFrontFd != -1 && toFrontFd != -1) {
        this->fromFrontEndFd = fromFrontFd;
        this->toFrontEndFd = toFrontFd;
        return 0;
    }
    else {
        return -1;
    }
}

int PipeBackend::makePipe() {
    int toFrontRet = mkfifo(toPipeFrontPath, 0666);
    int errno1 = 0, errno2 = 0;
    if(toFrontRet != 0) {
        errno1 = errno;
        if(errno == EEXIST) {
            LOGI("ToFrontPipeFile exists");
        }
        else {
            LOGE("Fail to make %s,  %s", toPipeFrontPath,  strerror(errno));
        }
    }
    int fromFrontRet = mkfifo(fromPipeFrontPath, 0666);
    if(fromFrontRet != 0 ) {
        errno2 = errno;
        if(errno == EEXIST) {
            LOGI("FromFrontPipeFile exists");
        }
        else {
            LOGE("Fail to make %s, %s", fromPipeFrontPath, strerror(errno));
        }

    }
    if((!toFrontRet || errno1 == EEXIST) && (!fromFrontRet || errno2 == EEXIST)) {
        LOGI("Success to make fifo");
        LOGI("toFrontPipePath: %s", toPipeFrontPath);
        LOGI("fromFrontPipePath: %s", fromPipeFrontPath);
        return 0;
    }
    else {
        LOGE("Fail to create fifo");
        return -1;
    }
}