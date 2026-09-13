// libfakeloc_initzygote.cpp
//
// Reconstructed, compilable source for the zygote init entry library.
// Recovered from do/complete/libfakeloc_initzygote.c (arm) and
// libfakeloc_initzygote64.c (arm64); one portable JNI source covers both ABIs.
//
// Flow:
//   doRun(JavaVM**, arg) -> AttachCurrentThread -> init(env)
//   init(env):
//     - verify release signature (allowing -2 "no context yet" in zygote)
//     - build a DexClassLoader over /data/kail-loc/libfakeloc.so with an opt
//       directory of /data/kail-loc/zygote_dex and the system class loader as
//       parent
//     - load com.kail.location.inject.fakelocation.InjectDex
//     - call InjectDex.initZygote(systemClassLoader) reflectively

#include "fakeloc_common.h"

using namespace fakeloc;

static const char *kOptDir = "/data/kail-loc/zygote_dex";
static bool gInitLoaded = false;     // byte_5960 / byte_6E28

// ---------------------------------------------------------------------------
// init  (sub_1D34 / sub_23E0)
// ---------------------------------------------------------------------------
static void init(JNIEnv *env) {
  KLOGI(kLogTag, "InitZygote is Executing");

  if (!env) {
    KLOGI(kLogTag, "jni_env is NULL!!");
    return;
  }

  int sig = verifyReleaseSignature(env);
  if (sig != 0 && sig != -2)
    return;

  jstring optDir  = env->NewStringUTF(kOptDir);
  jstring dexPath = env->NewStringUTF(kPayloadPath);

  jclass dclClass = env->FindClass("dalvik/system/DexClassLoader");
  jmethodID dclCtor = env->GetMethodID(
      dclClass, "<init>",
      "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
  jmethodID dclLoad = env->GetMethodID(
      dclClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");

  jobject systemLoader = getSystemClassLoader(env);

  jobject loader = env->NewObject(dclClass, dclCtor, dexPath, optDir, nullptr, systemLoader);

  jstring injectClassName = env->NewStringUTF("com.kail.location.inject.fakelocation.InjectDex");
  jclass injectClass = (jclass)env->CallObjectMethod(loader, dclLoad, injectClassName);

  jmethodID initZygote = env->GetStaticMethodID(
      injectClass, "initZygote", "(Ljava/lang/Object;)[Ljava/lang/Object;");
  env->CallStaticObjectMethod(injectClass, initZygote, systemLoader);

  KLOGI(kLogTag, "InitZygote is finished");

  env->DeleteLocalRef(optDir);
  env->DeleteLocalRef(dexPath);
  env->DeleteLocalRef(dclClass);
  env->DeleteLocalRef(systemLoader);
  env->DeleteLocalRef(injectClassName);
}

// ---------------------------------------------------------------------------
// doRun  (sub_205C / sub_2940)
// ---------------------------------------------------------------------------
extern "C" __attribute__((visibility("default"))) void doRun(JavaVM **vmPtr, const char *arg) {
  (void)arg;
  if (gInitLoaded) {
    KLOGE(kLogTag, "-- Already loaded");
    return;
  }
  gInitLoaded = true;

  if (!vmPtr) {
    KLOGE(kLogTag, "JavaVM** == NULL");
    return;
  }
  JavaVM *vm = *vmPtr;
  if (!vm) {
    KLOGE(kLogTag, "JavaVM* == NULL");
    return;
  }

  JNIEnv *env = nullptr;
  if (vm->AttachCurrentThread(&env, nullptr) != JNI_OK) {
    KLOGE(kLogTag, "AttachCurrentThread (main) != JNI_OK");
    return;
  }
  init(env);
}
