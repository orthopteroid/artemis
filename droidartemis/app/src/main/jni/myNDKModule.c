#include "com_tereslogica_droidartemis_ArtemisLib.h"

#include "library.h"

// adb logcat

char* szLocation = "foo.bar";

static int rc = 0;

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeGetString(JNIEnv * env, jobject obj)
{
    library_init();
    library_test();
    return (*env)->NewStringUTF( env, "hello\nworld" );
}

JNIEXPORT jboolean JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeGetStatusOK(JNIEnv * env, jobject obj)
{
    jboolean jok = rc ? JNI_FALSE : JNI_TRUE;
    return jok;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeDecode(JNIEnv * env, jobject obj, jstring jSharesNLArr)
{
    byte* cMessage_out = 0;
    jstring jMessage_out;

    const char *cSharesNLArr = (*env)->GetStringUTFChars(env, jSharesNLArr, 0);

    rc = library_uri_decoder( &cMessage_out, szLocation, (byte*)cSharesNLArr );

//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cMessage_out %X", (unsigned int)cMessage_out );
    if( cMessage_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "%s", cMessage_out );
    }
#endif

    jMessage_out = (*env)->NewStringUTF( env, cMessage_out );

    library_free( &cMessage_out );

    (*env)->ReleaseStringUTFChars( env, jSharesNLArr, cSharesNLArr );

    return jMessage_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareField(JNIEnv * env, jobject obj, jstring jShare, jstring jField, jint jFieldNum)
{
    byte* cField_out = 0;
    jstring jField_out;

    if( jFieldNum < 0 ) { rc=-1; return 0; }

    const char *cShare = (*env)->GetStringUTFChars(env, jShare, 0);
    const char *cField = (*env)->GetStringUTFChars(env, jField, 0);
    word16 uFieldNum = (word16)jFieldNum;

#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cShare %X", (unsigned int)cShare );
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField %X", (unsigned int)cField );
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "uFieldNum %u", uFieldNum );
#endif

    rc = library_uri_field( &cField_out, cShare, cField, uFieldNum );

//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField_out %X", (unsigned int)cField_out );
    if( cField_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField_out %s", cField_out );
    }
#endif

    jField_out = (*env)->NewStringUTF( env, cField_out );

    library_free( &cField_out );

    (*env)->ReleaseStringUTFChars( env, jShare, cShare );
    (*env)->ReleaseStringUTFChars( env, jField, cField );

    return jField_out;
}

