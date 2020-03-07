#include <jni.h>
#include <string>
#include <pthread.h>
#include <android/log.h>
#include <stdlib.h>
#include <unistd.h>

#define LOGE(FORMAT,...) __android_log_print(ANDROID_LOG_ERROR, "native-lib", FORMAT, ##__VA_ARGS__);
#define LOGD(FORMAT,...) __android_log_print(ANDROID_LOG_DEBUG, "native-lib", FORMAT, ##__VA_ARGS__);

extern "C"
{
    void * startThread(void *) {
        LOGE("test");
        return NULL;
    }

    JNIEXPORT jstring JNICALL Java_com_example_threadcreatetest_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "Hello from C++";
    return env->NewStringUTF(hello.c_str());
    }

    JNIEXPORT void JNICALL Java_com_example_threadcreatetest_MainActivity_threadLoopJNI(JNIEnv *pEnv, jobject thiz) {
        pthread_t threadId = 0;
        while (true) {
            pthread_create(&threadId, NULL, startThread, NULL);
//            sleep(1);
//            LOGE("Main Thread");
//            pthread_join(threadId, NULL);
        }
    }
}
