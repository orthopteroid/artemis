package com.tereslogica.droidartemis;

public class ArtemisLib {

    static {
        // only works with arm?
        System.loadLibrary("myNDKModule");
    }

    public native String nativeGetString();
    public native boolean nativeGetStatusOK();
    public native String nativeDecode( String data );
    //public native String nativeDecodeB64( String data );
    public native int[] nativeShareInfo( String share ); // 0 = shares, 1 = threshold
    public native String nativeShareField( String share, String field, int num );
    public native String nativeShareClue( String share );
    public native String nativeShareLocation( String share );

    ArtemisLib() {
        String sss = nativeGetString();
    }

}