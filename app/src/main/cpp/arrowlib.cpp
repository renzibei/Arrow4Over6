#include <jni.h>
#include <string>
#include <thread>
#include "arrowutils.h"
#include "PipeBackend.h"
#include "ArBackFramework.h"

void testPipe() {
    PipeBackend::getInstance()->writeInt(101);
    int recvInt = 0;
    int readRet = PipeBackend::getInstance()->readInt(recvInt);
    if(readRet == 0) {
        LOGI("backend Receive int %d", recvInt);
    }
    else {
        LOGE("backend fail to receive int");
    }
}

void runLoop(std::string appDirPath) {
    ArBackFramework::getInstance()->run(appDirPath.c_str());
}

extern "C" JNIEXPORT jint JNICALL
Java_com_gf_arrow4over6_MainActivity_startBackend(
        JNIEnv* env,
        jobject /* this */, jstring appDir) {

    LOGI("Receive?");
    std::string hello = "Hello from C++";
    std::string appDirStr = env->GetStringUTFChars(appDir, NULL);
    std::thread runThread(runLoop, appDirStr);
    runThread.detach();

//    std::thread testThread(testPipe);
//    testThread.detach();
    return 0;
}
