//
// Created by Fan Qu on 5/17/20.
//


#include <unistd.h>
#include <thread>
#include <chrono>
#include <cstring>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <cstdint>
#include "ArBackFramework.h"
#include "PipeBackend.h"
#include "arrowutils.h"
#include "SocketPlugin.h"

ArBackFramework::ArBackFramework():vpnFd(0), stopFlag(true) {

}

bool ArBackFramework::shouldStop() {
    return stopFlag;
}

void ArBackFramework::connectSocket() {

    static char ipv6AddressBuf[256];
    memset(ipv6AddressBuf, 0, sizeof(ipv6AddressBuf));
    int ipv6AddressLen = PipeBackend::getInstance()->readBytes(ipv6AddressBuf, 256);
    if(ipv6AddressLen < 0) {
        LOGE("IPV6 Address receive failed");
        std::exit(EXIT_FAILURE);
    }
    int port = 0;
    if(PipeBackend::getInstance()->readInt(port) != 0) {
        LOGE("Fail to receive port");
        std::exit(EXIT_FAILURE);
    }
    if(SocketPlugin::getInstance()->connectSocket(ipv6AddressBuf, port) < 0) {
        LOGE("Fail to connect socket");
//        std::exit(EXIT_FAILURE);
        PipeBackend::getInstance()->writeInt(204);
        return;
    }
    Msg tempMsg;
    tempMsg.type = 100;
    tempMsg.length = offsetof(Msg, data);
    if(sendMessage(tempMsg) < 0) {
        LOGE("Fail to send 100 connect msg");
        std::exit(EXIT_FAILURE);
    }
    LOGI("After send ip request");

    memset(&tempMsg, 0, sizeof(tempMsg));
    if(receiveMessage(tempMsg) < 0) {
        LOGE("Fail to receive ip info msg");
        std::exit(EXIT_FAILURE);
    }
    if(tempMsg.type != 101) {
        LOGE("respond msg type != 101, type = %d", tempMsg.type);
        std::exit(EXIT_FAILURE);
    }
    LOGI("receive ip info from server, %s", tempMsg.data);
    size_t dataLen = strlen(tempMsg.data);
    PipeBackend::getInstance()->writeInt(201);
//    char tempBuf[4096+8] = {0};
//    memcpy(tempBuf, tempMsg.data, dataLen);
    PipeBackend::getInstance()->writeBytes(tempMsg.data, dataLen);
    PipeBackend::getInstance()->writeInt(SocketPlugin::getInstance()->getSockFd());


}

int ArBackFramework::receiveMessage(Msg &msg) {
    if(SocketPlugin::getInstance()->readMsg(&msg, sizeof(int)) < 0) {
        LOGE("Error happens when read msg len");
        return -1;
    }
    if(SocketPlugin::getInstance()->readMsg(&msg.type, sizeof(msg.type)) < 0) {
        LOGE("Error happens when read msg type");
        return -1;
    }
    if(SocketPlugin::getInstance()->readMsg(&msg.data, msg.length - offsetof(Msg, data)) < 0) {
        LOGE("Error happens when read msg data");
        return -1;
    }
    return 0;
}

int ArBackFramework::sendMessage(const Msg &msg) {
//    if(SocketPlugin::getInstance()->writeMsg(&msg.length, sizeof(msg.length)) < 0) {
//        LOGE("Error happens when write msg len");
//        return -1;
//    }
//    if(SocketPlugin::getInstance()->writeMsg(&msg.type, sizeof(msg.type)) < 0) {
//        LOGE("Error happens when write msg type");
//        return -1;
//    }
//    if(SocketPlugin::getInstance()->writeMsg(&msg.data, msg.length - offsetof(Msg, data)) < 0) {
//        LOGE("Error happens when write msg data");
//        return -1;
//    }
    if(SocketPlugin::getInstance()->writeMsg(&msg, msg.length) < 0) {
        LOGE("Error happens when write msg");
        return -1;
    }
    return 0;
}

void ArBackFramework::readTunnel() {
    Msg tempMsg;
    LOGI("begin readTunnel");
    while(!stopFlag) {
        if(stopFlag) {
            break;
        }
        tempMsg.type = 102;
        ssize_t readLen = read(this->vpnFd, tempMsg.data, MSG_DATA_LEN);
        if(readLen < 0) {
            if (errno == EAGAIN)
                continue;
            LOGE("read from vpn tunnel failed, %s", strerror(errno));
//            this->stopFlag = true;
            if(!stopFlag) {
                stopRun();
            }
            break;
        }
        tempMsg.length = offsetof(Msg, data) + readLen;
        this->sendBytes += readLen;
        this->sendPackets++;
        if(this->sendMessage(tempMsg) < 0) {
            LOGE("send packet to server failed");
            continue;
        }

    }
    LOGI("read Tunnel return");


}

void ArBackFramework::initStatistics() {
    connectSeconds = 0; lastKeepAliveTime = time(NULL);
    sendPackets = 0; receivePackets = 0;
    sendBytes = 0; receiveBytes = 0;
}

static void readVpnTunnel() {
    ArBackFramework::getInstance()->readTunnel();
}

void ArBackFramework::writeTunnel() {
    LOGI("begin writeTunnel");
    Msg tempMsg;
    while(!stopFlag) {
        if(this->receiveMessage(tempMsg) < 0) {
            LOGE("receive Message from socket failed");
            if(!stopFlag) {
                stopRun();
            }
//            this->stopFlag = true;
            break;
        }
        LOGI("receive msg , type = %d", tempMsg.type);
        if(tempMsg.type == 101) {
            LOGW("receive 101 type again");
        }
        else if(tempMsg.type == 103) {
            size_t dataLen = tempMsg.length - offsetof(Msg, data);
            if(writenBytes(this->vpnFd, tempMsg.data, dataLen) < 0) {
                LOGE("write data to tunnel failed");
            }
            receivePackets++;
            receiveBytes += dataLen;
        }
        else if(tempMsg.type == 104) {
            lastKeepAliveTime = time(NULL);
        }
    }
    LOGI("write Tunnel end");
}

static void writeVpnTunnel() {
    ArBackFramework::getInstance()->writeTunnel();
}

void ArBackFramework::timerProcess() {
    LOGI("begin timerProcess");
    time_t lastSendTime = time(NULL);
    time_t startTime = lastSendTime;
    while(!stopFlag) {

        time_t nowTime = time(NULL);
        int connectSeconds = nowTime - startTime;
        int64_t tempSendBytes = sendBytes, tempReceiveBytes = receiveBytes;
        int uploadSpeed = tempSendBytes / (connectSeconds + 1);
        int downloadSpeed = tempReceiveBytes / (connectSeconds + 1);

        LOGI("sendBytes %lld, uploadSpeed %d, receiveBytes %lld, downloadSpeed %d", tempSendBytes, uploadSpeed, tempReceiveBytes, downloadSpeed);
        PipeBackend::getInstance()->writeInt(202);
        PipeBackend::getInstance()->writeInt(connectSeconds);
        PipeBackend::getInstance()->writeInt(uploadSpeed);
        PipeBackend::getInstance()->writeInt(downloadSpeed);
        PipeBackend::getInstance()->writeInt(sendPackets);
        PipeBackend::getInstance()->writeInt(receivePackets);
        PipeBackend::getInstance()->writeInt(tempSendBytes);
        PipeBackend::getInstance()->writeInt(tempReceiveBytes);

        if(nowTime - lastKeepAliveTime >= 60) {
            LOGW("Time out 60s");
            this->stopRun();
        }
        if(nowTime - lastSendTime >= 20) {
            Msg tempMsg;
            LOGI("send keep-alive packet");
            tempMsg.type = 104;
            tempMsg.length = offsetof(Msg, data);
            sendMessage(tempMsg);
            lastSendTime = nowTime;
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOGI("timeProcess end");
}

static void timerCount() {
    ArBackFramework::getInstance()->timerProcess();
}

void ArBackFramework::vpnProcess() {
    int tempVpnFd = 0;
    if(PipeBackend::getInstance()->readInt(tempVpnFd) < 0) {
        LOGE("vpn fd not valid");
        return;
    }
    this->vpnFd = tempVpnFd;
    this->initStatistics();
    this->stopFlag = false;
    std::thread readVpnThread(readVpnTunnel);
    std::thread writeVpnThread(writeVpnTunnel);
    std::thread timerCountThread(timerCount);
    readVpnThread.detach();
    writeVpnThread.detach();
    timerCountThread.detach();
//    readVpnThread.join();
//    writeVpnThread.join();
//    timerCountThread.join();
}




void ArBackFramework::stopRun() {
    LOGI("Stop Run backend 3 thread");
    this->stopFlag = true;
    PipeBackend::getInstance()->writeInt(203);
    SocketPlugin::getInstance()->closeSocket();
    if(close(vpnFd) < 0) {
        LOGE("close vpnFd error, %s", strerror(errno));
    }
}

/**
 * readCode = 101: start Connect, receive (ipv6 address (string), port int)
 *          = 102: receive vpn_fd int
 *          = 103 end connect
 *
 * writeCode = 201: send ip info, write [ipv4Addr, router, dns1, dns2, dns3] sockfd
 * writeCode = 202: send statical data, write connect time (int), upload speed (int)/Byte/s, download speed (int) Byte/s, uploadPackets (int), downloadPackets (int), upload bandwidth (int), download bandwidth (int)
 * writeCode = 203: send stop Flag, write null
 * writeCode = 204: connect socket failed
 *
 * @param appDirPath
 */

void ArBackFramework::run(const char* appDirPath) {
    PipeBackend::setPipeDirPath(appDirPath);
    PipeBackend *pipeBackend = PipeBackend::getInstance();

    int readCode = 0;
    while(true) {
        if(pipeBackend->readInt(readCode) != 0) {
            this->stopFlag = true;
            LOGE("Fail to read readCode");
            std::exit(EXIT_FAILURE);
        }
        if(readCode == 101) {
            connectSocket();
        }
        else if(readCode == 102) {
            vpnProcess();
        }
        else if(readCode == 103) {
            stopRun();
        }
        else {
            LOGE("readCode not in enum");
            std::exit(EXIT_FAILURE);
        }
    };
}