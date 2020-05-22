//
// Created by Fan Qu on 5/17/20.
//

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "SocketPlugin.h"
#include "arrowutils.h"
#include "PipeBackend.h"

int SocketPlugin::connectSocket(const char *addrStr, int port) {

    int socketFd = 0;
    if((socketFd = socket(AF_INET6, SOCK_STREAM, 0)) < 0) {
        LOGE("Fail to create socket, %s", strerror(errno));
        return -1;
    }
    struct sockaddr_in6 dest;
    memset(&dest, 0, sizeof(dest));
    dest.sin6_family = AF_INET6;
    dest.sin6_port = htons(port);
    LOGI("server ipv6 address %s", addrStr);
    if(inet_pton(AF_INET6, addrStr, &dest.sin6_addr) != 1) {
        LOGE("Fail to convert ipv6 Address, %s", strerror(errno));
        return -1;
    }
    LOGI("before connect socket");
    if(connect(socketFd, (struct sockaddr*) &dest, sizeof(dest)) < 0) {
        LOGE("Fail to connect socket, %s", strerror(errno));
        return -1;
    }
    LOGI("connect succeed");
    this->m_sockfd = socketFd;
    return 0;
}

int SocketPlugin::closeSocket() {

    int ret = close(this->m_sockfd);
    if(ret == -1) {
        LOGE("close socket failed", strerror(errno));
    }
    LOGI("close socket");
    return ret;
}

int SocketPlugin::getSockFd() {
    return this->m_sockfd;
}

int SocketPlugin::writeMsg(const void *msg, size_t msgLen) {
//    size_t hasWritten = 0;
//    const char* dest = (const char*)msg;
//    while(hasWritten < msgLen) {
//        ssize_t tempLen = write(m_sockfd, dest + hasWritten, msgLen - hasWritten);
//        if(tempLen < 0) {
//            LOGE("Errors when write socket, %s", strerror(errno));
//            return -1;
//        }
//        hasWritten += tempLen;
//    }
//    return 0;
    return writenBytes(this->m_sockfd, msg, msgLen);
}

int SocketPlugin::readMsg(void *buf, size_t msgLen) {
//    size_t hasRead = 0;
//    char* src = (char*)buf;
//    while(hasRead < msgLen) {
//        ssize_t tempLen = read(m_sockfd, src + hasRead, msgLen - hasRead);
//        if(tempLen < 0) {
//            LOGE("Error happens when read socket, %s", strerror(errno));
//            return -1;
//        }
//        hasRead += tempLen;
//    }
//    return 0;
    return readnBytes(this->m_sockfd, buf, msgLen);
}