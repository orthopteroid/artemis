#include "com_tereslogica_droidartemis_MyActivity.h"

#include "library.h"

static int rc = 0;

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_MyActivity_nativeGetString(JNIEnv * env, jobject obj)
{
    library_init();
    library_test();
    return (*env)->NewStringUTF( env, "hello\nworld" );
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_MyActivity_nativeDecode(JNIEnv * env, jobject obj, jstring jSharesNLArr)
{
    byte* cMessage_out = 0;
    jstring jMessage_out;

    const char *cSharesNLArr = (*env)->GetStringUTFChars(env, jSharesNLArr, 0);

    rc = library_uri_decoder( &cMessage_out, "foo.bar", (byte*)cSharesNLArr );

//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cMessage_out %X", cMessage_out );
    if( cMessage_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "%s", cMessage_out );
    }
#endif

    jMessage_out = (*env)->NewStringUTF( env, cMessage_out );
    library_free( &cMessage_out );
    (*env)->ReleaseStringUTFChars( env, jSharesNLArr, cSharesNLArr );

    return jMessage_out;
}

JNIEXPORT jboolean JNICALL Java_com_tereslogica_droidartemis_MyActivity_nativeGetStatusOK(JNIEnv * env, jobject obj)
{
    jboolean jok = rc ? JNI_FALSE : JNI_TRUE;
    return jok;
}

