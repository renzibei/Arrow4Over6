//
// Created by Fan Qu on 5/14/20.
//

#ifndef ARROW4OVER6_ARROWUTILS_H
#define ARROW4OVER6_ARROWUTILS_H

#include <android/log.h>
#include <sys/types.h>

#define D_TAG "AW_CXX_DEBUG"
#define I_TAG "AW_CXX_INFO"
#define W_TAG "AW_CXX_WARN"
#define E_TAG "AW_CXX_ERROR"
#define V_TAG "AW_CXX_VERBOSE"

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, D_TAG ,__VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, I_TAG ,__VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN, W_TAG ,__VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR, E_TAG ,__VA_ARGS__)
#define LOGV(...) __android_log_print(ANDROID_LOG_FATAL,V_TAG ,__VA_ARGS__)

#define MSG_DATA_LEN 4096

struct Msg{
    int length;
    char type;
    char data[MSG_DATA_LEN];
};

ssize_t readnBytes(int fd, void *buf, size_t nbyte);
ssize_t writenBytes(int fd, const void *buf, size_t nbyte);

#endif //ARROW4OVER6_ARROWUTILS_H
