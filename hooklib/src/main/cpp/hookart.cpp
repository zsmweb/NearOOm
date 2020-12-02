//

#include <android/log.h>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>
#include <fcntl.h>
#include <jni.h>
#include <string>
#include <unistd.h>
#include <wait.h>
#include "xhook.h"
#include "kwai_dlfcn.h"
#include <unwind.h>
#include <cxxabi.h>

#ifndef LOG_TAG
#define LOG_TAG "NHookArt"
#endif
#define ALOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__)
#define ALOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG, __VA_ARGS__)

typedef std::string (*PrettySize)(int64_t byte_count);
static PrettySize oldPrettySize;

typedef void (*PSetIdealFootprint)(size_t target_footprint);
static PSetIdealFootprint SetIdealFootprint = 0;// _ZN3art2gc4Heap17SetIdealFootprintEm;

static JavaVM* g_jvm = nullptr;//全局虚拟机对象
static jmethodID reportException;//methodid
static jobject hookArtClassObj;//类的全局引用（不用全局引用会报错）
static bool hasReport = false;//只汇报一次

void doReport(int type) {
    JNIEnv *env;
    int needDetach = false;
    //获取当前native线程是否有没有被附加到jvm环境中
    int getEnvStat = g_jvm->GetEnv(reinterpret_cast<void **>(&env), JNI_VERSION_1_6);
    if (getEnvStat == JNI_EDETACHED) {
        //如果没有， 主动附加到jvm环境中，获取到env
        if (g_jvm->AttachCurrentThread(&env, NULL) != 0) {
            ALOGE("attach thread failed!");
            return;
        }
        needDetach = JNI_TRUE;
    }

    //调用java层report函数
    env->CallStaticVoidMethod(static_cast<jclass>(hookArtClassObj), reportException, type);

    //释放当前线程
    if(needDetach) {
        g_jvm->DetachCurrentThread();
    }
    env = NULL;
}

struct android_backtrace_state
{
    void **current;
    void **end;
};

_Unwind_Reason_Code android_unwind_callback(struct _Unwind_Context* context,
                                            void* arg)
{
    android_backtrace_state* state = (android_backtrace_state *)arg;
    uintptr_t pc = _Unwind_GetIP(context);
    if (pc)
    {
        if (state->current == state->end)
        {
            return _URC_END_OF_STACK;
        }
        else
        {
            *state->current++ = reinterpret_cast<void*>(pc);
        }
    }
    return _URC_NO_REASON;
}

int mines(void* p1, void* p2) {
    int ret = 0x101;
    ret = (long)p2 - (long)p1;
    return ret;
}

void dump_stack(void)
{
    const int max = 100;
    void* buffer[max];

    android_backtrace_state state;
    state.current = buffer;
    state.end = buffer + max;

    _Unwind_Backtrace(android_unwind_callback, &state);

    int count = (int)(state.current - buffer);

    if (count >= 3) {
        //确定是从SetIdealFootprint掉进Prettysize来的
        int diff = mines((void*)SetIdealFootprint, buffer[2]);
        if (diff > 0 && diff <= 0x100) {
            ALOGD("do print stack ：%d, for %p, %p", mines((void*)SetIdealFootprint, buffer[2]), SetIdealFootprint, buffer[2]);
            if (!hasReport) {
                //判断是从SetIdealFootprint掉进来便汇报异常，在java层去抓取hprof
                doReport(0);
                hasReport = true;
            }

            for (int idx = 0; idx < count; idx++)
            {
                const void* addr = buffer[idx];
                const char* symbol = "";

                Dl_info info;
                if (dladdr(addr, &info) && info.dli_sname)
                {
                    symbol = info.dli_sname;
                }
                int status = 0;
                char *demangled = __cxxabiv1::__cxa_demangle(symbol, 0, 0, &status);

                ALOGD("%03d: %p %s",
                      idx,
                      addr,
                      (NULL != demangled && 0 == status) ?
                      demangled : symbol);

                if (NULL != demangled)
                    free(demangled);
            }
        }
    }
}

std::string HookPrettySize(int64_t byte_count) {
    ALOGD( "Prettysize (%lld)", byte_count);
    dump_stack();
    return oldPrettySize(byte_count);
}

static PrettySize hookPrettySize = &HookPrettySize;

extern "C" {

JNIEXPORT int JNICALL
Java_com_test_ems_hook_HookArt_initHook(JNIEnv *env, jclass jClass) {

    xhook_enable_debug(0);

    int ret = xhook_register("libart.so", "_ZN3art10PrettySizeEl", (void *) hookPrettySize,
                             NULL);
    ALOGE( "hook result %d",
                        ret);

    {
        void *libHandle = kwai::linker::DlFcn::dlopen("libartbase.so", RTLD_NOW);
        if (libHandle == nullptr) {
            ALOGE( "dlopen result %p",
                                libHandle);
            return -1;
        }

        oldPrettySize = (PrettySize) kwai::linker::DlFcn::dlsym(libHandle, "_ZN3art10PrettySizeEl");
        if (oldPrettySize == nullptr) {
            ALOGE( "old PrettySize is %p!",
                                oldPrettySize);
            kwai::linker::DlFcn::dlclose(libHandle);
            return -1;
        }

        ALOGD("oldPrettySize = %p", oldPrettySize);
        kwai::linker::DlFcn::dlclose(libHandle);
    }

    {
        void *libcHandle = kwai::linker::DlFcn::dlopen("libart.so", RTLD_NOW);
        if (libcHandle == nullptr) {
            ALOGE( "dlopen 2 result %p",
                                libcHandle);
            return -1;
        }
        SetIdealFootprint = (PSetIdealFootprint)kwai::linker::DlFcn::dlsym(libcHandle,
                                                       "_ZN3art2gc4Heap17SetIdealFootprintEm");
        if (SetIdealFootprint == nullptr) {
            ALOGE( "old SetIdealFootprint is %p!",
                                SetIdealFootprint);
            kwai::linker::DlFcn::dlclose(libcHandle);
            return -1;
        }

        ALOGD("SetIdealFootprint = %p", SetIdealFootprint);

        kwai::linker::DlFcn::dlclose(libcHandle);
    }

    ALOGD("HookPrettySize = %p", hookPrettySize);

    xhook_refresh(0);
    xhook_clear();
    {
        hookArtClassObj = env->NewGlobalRef(jClass);
        env->GetJavaVM(&g_jvm);
        reportException = env->GetStaticMethodID(jClass, "doReport", "(I)V");
        if (reportException == 0) {
            ALOGE("failed to get function doReport!");
        }
        ALOGE("HOOKART CLASS: %p, REPORT FUNC: %p", hookArtClassObj, reportException);
    }
    return ret;
}
}