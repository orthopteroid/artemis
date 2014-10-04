#include "com_tereslogica_droidartemis_ArtemisLib.h"

#include "library.h"
#include <string.h>

// adb logcat

char* szUnknown = "?? ?? ??";
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

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeDecode(JNIEnv * env, jobject obj, jstring jRecordArr)
{
    byte* cMessage_out = 0;
    jstring jMessage_out;

    const char *cRecordArr = (*env)->GetStringUTFChars(env, jRecordArr, 0);

#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cRecordArr %s", cRecordArr );
#endif

    rc = library_uri_decoder( &cMessage_out, (byte*)szLocation, (byte*)cRecordArr );

//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk
#if defined(_DEBUG)
    __android_log_print(ANDROID_LOG_INFO, "libartemis", "cMessage_out %X", (unsigned int)cMessage_out );
    if( cMessage_out) {
        __android_log_print(ANDROID_LOG_INFO, "libartemis", "%s", cMessage_out );
    }
#endif

    if( cMessage_out )
    {
        jMessage_out = (*env)->NewStringUTF( env, cMessage_out );
        library_free( &cMessage_out );
    } else {
        jMessage_out = (*env)->NewStringUTF( env, szUnknown );
    }

    (*env)->ReleaseStringUTFChars( env, jRecordArr, cRecordArr );

    return jMessage_out;
}

JNIEXPORT jintArray JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeInfo(JNIEnv * env, jobject obj, jstring jRecord)
{
    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    word16 rtype, shares, threshold;
    rc = library_uri_info( &rtype, &shares, &threshold, (byteptr)cRecord );

    (*env)->ReleaseStringUTFChars( env, jRecord, cRecord );

    jintArray shareInfo = (*env)->NewIntArray( env, 3 );
    jint *shareInfoData = (*env)->GetIntArrayElements( env, shareInfo, NULL);

    shareInfoData[0] = rtype;
    shareInfoData[1] = shares;
    shareInfoData[2] = threshold;

    // http://publib.boulder.ibm.com/infocenter/javasdk/v1r4m2/index.jsp?topic=%2Fcom.ibm.java.doc.diagnostics.142j9%2Fhtml%2Fusemodeflag.html
    (*env)->ReleaseIntArrayElements( env, shareInfo, shareInfoData, JNI_COMMIT );

    return shareInfo;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeTopic(JNIEnv * env, jobject obj, jstring jRecord)
{
    byte* cTopic_out = 0;
    jstring jTopic_out;

    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    rc = library_uri_topic( &cTopic_out, (byteptr)cRecord );

    jTopic_out = (*env)->NewStringUTF( env, cTopic_out );

    library_free( &cTopic_out );

    (*env)->ReleaseStringUTFChars( env, jRecord, cRecord );

    return jTopic_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeClue(JNIEnv * env, jobject obj, jstring jRecord)
{
    byte* cClue_out = 0;
    jstring jClue_out;

    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    rc = library_uri_clue( &cClue_out, (byteptr)cRecord );

    jClue_out = (*env)->NewStringUTF( env, cClue_out );

    library_free( &cClue_out );

    (*env)->ReleaseStringUTFChars( env, jRecord, cRecord );

    return jClue_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_droidartemis_ArtemisLib_nativeLocation(JNIEnv * env, jobject obj, jstring jRecord)
{
    byte* cLocation_out = 0;
    jstring jLocation_out;

    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    rc = library_uri_location( &cLocation_out, (byteptr)cRecord );

    jLocation_out = (*env)->NewStringUTF( env, cLocation_out );

    library_free( &cLocation_out );

    (*env)->ReleaseStringUTFChars( env, jRecord, cRecord );

    return jLocation_out;
}
