//
// Created by Fan Qu on 5/17/20.
//

#ifndef ARROW4OVER6_ARBACKFRAMEWORK_H
#define ARROW4OVER6_ARBACKFRAMEWORK_H

#include "Singleton.h"
#include "arrowutils.h"

class ArBackFramework: public Singleton<ArBackFramework> {
private:
    friend Singleton<ArBackFramework>;

    std::atomic<bool> stopFlag;
    std::atomic<time_t> connectSeconds, lastKeepAliveTime;
    std::atomic<int> sendPackets, receivePackets;
    std::atomic<int64_t> sendBytes, receiveBytes;



    int vpnFd;

    ArBackFramework();

    void connectSocket();

    void vpnProcess();

    void initStatistics();

    void stopRun();

    int receiveMessage(Msg &msg);
    int sendMessage(const Msg& msg);

    ~ArBackFramework() {};

public:
    void run(const char* appDirPath);
    void readTunnel();
    void writeTunnel();
    void timerProcess();
    bool shouldStop();
};


#endif //ARROW4OVER6_ARBACKFRAMEWORK_H
