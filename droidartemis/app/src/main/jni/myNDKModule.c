#include "com_tereslogica_droidartemis_ArtemisLib.h"

#include "library.h"

// adb logcat

char* szLocation = "foo.bar";

static int rc = 0;

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeGetString(JNIEnv * env, jobject obj)
{
    library_init();
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
/*
//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cMessage_out %X", (unsigned int)cMessage_out );
    if( cMessage_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "%s", cMessage_out );
    }
#endif
*/
    jMessage_out = (*env)->NewStringUTF( env, cMessage_out );

    library_free( &cMessage_out );

    (*env)->ReleaseStringUTFChars( env, jSharesNLArr, cSharesNLArr );

    return jMessage_out;
}
/*
JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeDecodeB64(JNIEnv * env, jobject obj, jstring jString)
{
    byte* cString_out = 0;
    jstring jString_out;

    const char *cString = (*env)->GetStringUTFChars(env, jString, 0);

    rc = library_b64_decoder( &cString_out, cString );

//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cString_out %X", (unsigned int)cString_out );
    if( cString_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "%s", cString_out );
    }
#endif

    jString_out = (*env)->NewStringUTF( env, cString_out );

    library_free( &cString_out );

    (*env)->ReleaseStringUTFChars( env, jString, cString );

    return jString_out;
}
*/

JNIEXPORT jintArray JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareInfo(JNIEnv * env, jobject obj, jstring jShare)
{
    const char *cShare = (*env)->GetStringUTFChars(env, jShare, 0);

    word16 shares, threshold;
    rc = library_uri_shareinfo( &shares, &threshold, (byteptr)cShare );

    (*env)->ReleaseStringUTFChars( env, jShare, cShare );

    jintArray shareInfo = (*env)->NewIntArray( env, 2 );
    jint *shareInfoData = (*env)->GetIntArrayElements( env, shareInfo, NULL);

    shareInfoData[0] = shares;
    shareInfoData[1] = threshold;

    // http://publib.boulder.ibm.com/infocenter/javasdk/v1r4m2/index.jsp?topic=%2Fcom.ibm.java.doc.diagnostics.142j9%2Fhtml%2Fusemodeflag.html
    (*env)->ReleaseIntArrayElements( env, shareInfo, shareInfoData, JNI_COMMIT );

    return shareInfo;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareField(JNIEnv * env, jobject obj, jstring jShare, jstring jField, jint jFieldNum)
{
    byte* cField_out = 0;
    jstring jField_out;

    if( jFieldNum < 0 ) { rc=-1; return 0; }

    const char *cShare = (*env)->GetStringUTFChars(env, jShare, 0);
    const char *cField = (*env)->GetStringUTFChars(env, jField, 0);
    word16 uFieldNum = (word16)jFieldNum;
/*
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cShare %X", (unsigned int)cShare );
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField %X", (unsigned int)cField );
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "uFieldNum %u", uFieldNum );
#endif
*/
    rc = library_uri_field( &cField_out, (byteptr)cShare, (byteptr)cField, uFieldNum );
/*
//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField_out %X", (unsigned int)cField_out );
    if( cField_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "cField_out %s", cField_out );
    }
#endif
*/
    jField_out = (*env)->NewStringUTF( env, cField_out );

    library_free( &cField_out );

    (*env)->ReleaseStringUTFChars( env, jShare, cShare );
    (*env)->ReleaseStringUTFChars( env, jField, cField );

    return jField_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareClue(JNIEnv * env, jobject obj, jstring jShare)
{
    byte* cClue_out = 0;
    jstring jClue_out;

    const char *cShare = (*env)->GetStringUTFChars(env, jShare, 0);
/*
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cShare %X", (unsigned int)cShare );
#endif
*/
    rc = library_uri_clue( &cClue_out, (byteptr)cShare );
/*
//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cClue_out %X", (unsigned int)cClue_out );
    if( cClue_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "cClue_out %s", cClue_out );
    }
#endif
*/
    jClue_out = (*env)->NewStringUTF( env, cClue_out );

    library_free( &cClue_out );

    (*env)->ReleaseStringUTFChars( env, jShare, cShare );

    return jClue_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeShareLocation(JNIEnv * env, jobject obj, jstring jShare)
{
    byte* cLocation_out = 0;
    jstring jLocation_out;

    const char *cShare = (*env)->GetStringUTFChars(env, jShare, 0);
/*
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cShare %X", (unsigned int)cShare );
    if( cShare) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "cShare %s", cShare );
    }
#endif
*/
    rc = library_uri_location( &cLocation_out, (byteptr)cShare );
/*
//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cLocation_out %X", (unsigned int)cLocation_out );
    if( cLocation_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "cLocation_out %s", cLocation_out );
    }
#endif
*/
    jLocation_out = (*env)->NewStringUTF( env, cLocation_out );

    library_free( &cLocation_out );

    (*env)->ReleaseStringUTFChars( env, jShare, cShare );

    return jLocation_out;
}
