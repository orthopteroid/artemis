package com.tereslogica.droidartemis;

public class ArtemisLib {

    static {
        // only works with arm?
        System.loadLibrary("myNDKModule");
    }

    public final static int URI_A = 1;
    public final static int URI_B = 2;

    public native String nativeGetString();
    public native boolean nativeGetStatusOK();
    public native String nativeDecode( String recordArr ); // recordArr is \n delimited

    public native int[] nativeInfo( String record ); // type, shares, threshold
    public native String nativeTopic( String record );
    public native String nativeClue( String record );
    public native String nativeLocation( String record );

    ArtemisLib() {
        String sss = nativeGetString();
    }

}
