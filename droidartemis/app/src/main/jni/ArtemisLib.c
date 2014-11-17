#include "ArtemisLib.h"

#include "library.h"
#include "ar_codes.h"

#include <string.h>

// adb logcat
//http://stackoverflow.com/questions/10531050/redirect-stdout-to-logcat-in-android-ndk

#undef DEBUGPRINT
#define DEBUGPRINT(...) (0)

// unpin string copies
// http://stackoverflow.com/questions/5859673/should-you-call-releasestringutfchars-if-getstringutfchars-returned-a-copy

char* szBlank = "";

static int rc = 0;

//////////////////////////////

JNIEXPORT void JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeInit(JNIEnv * env, jobject obj)
{
    library_init();
    rc = 0;
}

JNIEXPORT void JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeCleanup(JNIEnv * env, jobject obj)
{
    library_cleanup();
}

JNIEXPORT jboolean JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeDidFail(JNIEnv * env, jobject obj)
{
    jboolean jok = ( rc != 0 ) ? JNI_TRUE : JNI_FALSE;
    return jok;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeDecode(JNIEnv * env, jobject obj, jstring jLoc, jstring jRecordArr)
{
    byte* cMessage_out = 0;
    jstring jMessage_out;

    const char *cLoc = (*env)->GetStringUTFChars(env, jLoc, 0);
    const char *cRecordArr = (*env)->GetStringUTFChars(env, jRecordArr, 0);

    DEBUGPRINT( "cLoc %s", cLoc );
    DEBUGPRINT( "cRecordArr %s", cRecordArr );

    rc = library_uri_decoder( &cMessage_out, (byte*)cLoc, (byte*)cRecordArr );

    DEBUGPRINT( "cMessage_out %X", (unsigned int)cMessage_out );
    if( cMessage_out) {
        DEBUGPRINT( "%s", cMessage_out );
    }

    if( rc == 0 && cMessage_out ) {
        jMessage_out = (*env)->NewStringUTF( env, cMessage_out );
    } else {
        jMessage_out = (*env)->NewStringUTF( env, szBlank );
    }
    if( cMessage_out ) { library_free( &cMessage_out ); }

    (*env)->ReleaseStringUTFChars( env, jRecordArr, cRecordArr );
    (*env)->ReleaseStringUTFChars( env, jLoc, cLoc );

    return jMessage_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeEncode(JNIEnv * env, jobject obj, jint jKeys, jint jLocks, jstring jLoc, jstring jClues, jstring jMess )
{
    byte* cShares_out = 0;
    jstring jShares_out;

    const char *cLoc = (*env)->GetStringUTFChars(env, jLoc, 0);
    const char *cMess = (*env)->GetStringUTFChars(env, jMess, 0);
    const char *cClues = (*env)->GetStringUTFChars(env, jClues, 0);

    word16 cKeys = (word16)jKeys;
    byte cLocks = (byte)jLocks;

    DEBUGPRINT( "jKeys %d", jKeys );
    DEBUGPRINT( "jLocks %d", jLocks );
    DEBUGPRINT( "cLoc %s", cLoc );
    DEBUGPRINT( "cClues %s", cClues );
    DEBUGPRINT( "cMess %s", cMess );

    rc = library_uri_encoder( &cShares_out, cKeys, cLocks, (byteptr)cLoc, (byteptr)cClues, (byteptr)cMess );

    DEBUGPRINT( "cShares_out %X", (unsigned int)cShares_out );
    if( cShares_out) {
        DEBUGPRINT( "%s", cShares_out );
    }

    if( rc == 0 && cShares_out ) {
        jShares_out = (*env)->NewStringUTF( env, cShares_out );
    } else {
        jShares_out = (*env)->NewStringUTF( env, szBlank );
    }
    if( cShares_out ) { library_free( &cShares_out ); }

    (*env)->ReleaseStringUTFChars( env, jClues, cClues );
    (*env)->ReleaseStringUTFChars( env, jMess, cMess );
    (*env)->ReleaseStringUTFChars( env, jLoc, cLoc );

    return jShares_out;
}

JNIEXPORT jintArray JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeInfo(JNIEnv * env, jobject obj, jstring jRecord)
{
    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    word16 rtype, shares;
    byte threshold;

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

JNIEXPORT jstring JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeTopic(JNIEnv * env, jobject obj, jstring jRecord)
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

JNIEXPORT jstring JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeClue(JNIEnv * env, jobject obj, jstring jRecord)
{
    byte* cClue_out = 0;
    jstring jClue_out;

    const char *cRecord = (*env)->GetStringUTFChars(env, jRecord, 0);

    rc = library_uri_clue( &cClue_out, (byteptr)cRecord );

    if( rc == 0 && cClue_out ) {
        jClue_out = (*env)->NewStringUTF( env, cClue_out );
    } else {
        jClue_out = (*env)->NewStringUTF( env, szBlank );
    }
    if( cClue_out ) { library_free( &cClue_out ); }

    (*env)->ReleaseStringUTFChars( env, jRecord, cRecord );

    return jClue_out;
}

JNIEXPORT jstring JNICALL Java_com_tereslogica_arcanashare_ArtemisLib_nativeLocation(JNIEnv * env, jobject obj, jstring jRecord)
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
