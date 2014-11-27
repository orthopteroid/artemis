package com.tereslogica.arcanacard;

public class ArtemisLib {

    static {
        // only works with arm?
        System.loadLibrary("ArtemisLib");
    }

    // value is used as a collating order on share_page
    public final static int URI_A = 1;
    public final static int URI_B = 2;

    public native void nativeInit();
    public native void nativeCleanup();

    public native boolean nativeDidFail();

    public native boolean nativeValidate( String location, String recordArr );
    public native String nativeDecode( String location, String recordArr ); // recordArr is \n delimited

    public native String nativeEncode( int keys, int locks, String location, String clues, String message );

    public native int[] nativeInfo( String record ); // type, shares, threshold
    public native String nativeTopic( String record );
    public native String nativeClue( String record );
    public native String nativeLocation( String record );

    ///////////////////

    private static ArtemisLib instance = null;

    private ArtemisLib() {}

    static void Init() {
        if( instance == null ) { instance = new ArtemisLib(); }
        instance.nativeInit();
    }
    static void Cleanup() {
        instance.nativeCleanup();
        instance = null;
    }
    static ArtemisLib Get() {
        return instance;
    }
}
