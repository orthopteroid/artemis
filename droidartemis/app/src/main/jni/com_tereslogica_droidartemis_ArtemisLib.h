/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class com_tereslogica_droidartemis_ArtemisLib */

#ifndef _Included_com_tereslogica_droidartemis_ArtemisLib
#define _Included_com_tereslogica_droidartemis_ArtemisLib
#ifdef __cplusplus
extern "C" {
#endif
#undef com_tereslogica_droidartemis_ArtemisLib_MODE_PRIVATE
#define com_tereslogica_droidartemis_ArtemisLib_MODE_PRIVATE 0L
#undef com_tereslogica_droidartemis_ArtemisLib_MODE_WORLD_READABLE
#define com_tereslogica_droidartemis_ArtemisLib_MODE_WORLD_READABLE 1L
#undef com_tereslogica_droidartemis_ArtemisLib_MODE_WORLD_WRITEABLE
#define com_tereslogica_droidartemis_ArtemisLib_MODE_WORLD_WRITEABLE 2L
#undef com_tereslogica_droidartemis_ArtemisLib_MODE_APPEND
#define com_tereslogica_droidartemis_ArtemisLib_MODE_APPEND 32768L
#undef com_tereslogica_droidartemis_ArtemisLib_BIND_AUTO_CREATE
#define com_tereslogica_droidartemis_ArtemisLib_BIND_AUTO_CREATE 1L
#undef com_tereslogica_droidartemis_ArtemisLib_BIND_DEBUG_UNBIND
#define com_tereslogica_droidartemis_ArtemisLib_BIND_DEBUG_UNBIND 2L
#undef com_tereslogica_droidartemis_ArtemisLib_BIND_NOT_FOREGROUND
#define com_tereslogica_droidartemis_ArtemisLib_BIND_NOT_FOREGROUND 4L
#undef com_tereslogica_droidartemis_ArtemisLib_CONTEXT_INCLUDE_CODE
#define com_tereslogica_droidartemis_ArtemisLib_CONTEXT_INCLUDE_CODE 1L
#undef com_tereslogica_droidartemis_ArtemisLib_CONTEXT_IGNORE_SECURITY
#define com_tereslogica_droidartemis_ArtemisLib_CONTEXT_IGNORE_SECURITY 2L
#undef com_tereslogica_droidartemis_ArtemisLib_CONTEXT_RESTRICTED
#define com_tereslogica_droidartemis_ArtemisLib_CONTEXT_RESTRICTED 4L
#undef com_tereslogica_droidartemis_ArtemisLib_RESULT_CANCELED
#define com_tereslogica_droidartemis_ArtemisLib_RESULT_CANCELED 0L
#undef com_tereslogica_droidartemis_ArtemisLib_RESULT_OK
#define com_tereslogica_droidartemis_ArtemisLib_RESULT_OK -1L
#undef com_tereslogica_droidartemis_ArtemisLib_RESULT_FIRST_USER
#define com_tereslogica_droidartemis_ArtemisLib_RESULT_FIRST_USER 1L
#undef com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_DISABLE
#define com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_DISABLE 0L
#undef com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_DIALER
#define com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_DIALER 1L
#undef com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SHORTCUT
#define com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SHORTCUT 2L
#undef com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SEARCH_LOCAL
#define com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SEARCH_LOCAL 3L
#undef com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SEARCH_GLOBAL
#define com_tereslogica_droidartemis_ArtemisLib_DEFAULT_KEYS_SEARCH_GLOBAL 4L

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeGetString(JNIEnv *, jobject);

JNIEXPORT jboolean JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeGetStatusOK(JNIEnv * env, jobject obj);

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeDecode(JNIEnv * env, jobject obj, jstring jSharesNLArr);

JNIEXPORT jintArray JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareInfo(JNIEnv * env, jobject obj, jstring jShare);

//JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeDecodeB64(JNIEnv * env, jobject obj, jstring jString);

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareField(JNIEnv * env, jobject obj, jstring jShare, jstring jField, jint jFieldNum);

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareClue(JNIEnv * env, jobject obj, jstring jShare);

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareLocation(JNIEnv * env, jobject obj, jstring jShare);

#ifdef __cplusplus
}
#endif
#endif