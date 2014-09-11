#include "com_tereslogica_droidartemis_MyActivity.h"

#include "library.h"

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_MyActivity_getStringFromNative(JNIEnv * env, jobject obj)
{
    library_init();
    return (*env)->NewStringUTF( env, "hello\nworld" );
}