//
// Created by Fan Qu on 5/14/20.
//

#ifndef ARROW4OVER6_PIPEBACKEND_H
#define ARROW4OVER6_PIPEBACKEND_H

#include <mutex>
#include "Singleton.h"

//#define AW_READ_BUFFER_SIZE 4096
//#define AW_WRITE_BUFFER_SIZE 4096

class PipeBackend: public Singleton<PipeBackend> {

private:

    friend Singleton<PipeBackend>;
//    static PipeBackend* _instance;
//    static std::mutex singleMutex, readMutex, writeMutex;
    static std::mutex readMutex, writeMutex;
    static char* pipeDirPath;

    char* toPipeFrontPath;
    char* fromPipeFrontPath;

    static constexpr size_t WRITE_BUFFER_SIZE = 1 << 14;
    static constexpr size_t READ_BUFFER_SIZE = 1 << 14;

    char readBuffer[READ_BUFFER_SIZE];
    char writeBuffer[WRITE_BUFFER_SIZE];

    int toFrontEndFd, fromFrontEndFd;

    PipeBackend();

    void initPipeName(const char *dirPath);

    int makePipe();

    int openPipe();

    // read len at the beginning, return 0 if succeed
    int readLength(int &len);



    int writeLength(int len);



    ~PipeBackend();

public:

    // use setPipeDirPath first
//    static PipeBackend* getInstance() {
//        if(_instance == nullptr) {
//            std::lock_guard<std::mutex> lockGuard(singleMutex);
//            if(_instance == nullptr)
//                _instance = new PipeBackend();
//        }
//        return _instance;
//    }

    //before getInstance, should set PipeDirPath
    static void setPipeDirPath(const char* dirPath);

    //read n bytes to readBuffer, return n
    int readBytes();

    int readBytes(char* buf, int bufLen);

    //read Int to x, return 0 if succeed
    int readInt(int &x);

    //write len bytes from writeBuffer, return 0 if succeed
    int writeBytes(const char* msg, int len);

    int writeInt(int dataInt);
};


#endif //ARROW4OVER6_PIPEBACKEND_H
