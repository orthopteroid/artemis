package com.tereslogica.arcanacard;

import android.app.Activity;
import android.content.Context;
import android.content.SharedPreferences;

/**
 * Created by john on 13/11/14.
 */
public class Prefs {
    public static final String TOPIC_SORTORDER = "topic-sort-order";
    public static final String EULA_VERSION = "eula-version-1";
    public static final String QR_WIDTH = "qr-width";
    public static final String QR_HEIGHT = "qr-height";
    public static final String HOWTO_NEW = "howto-new";
    public static final String HOWTO_INTRO = "howto-intro";
    public static final String HOWTO_TEST = "howto-test";
    public static final String HOWTO_AUTOFILL = "howto-autofill";

    public static int DEFAULT_WIDTH = 300;
    public static int DEFAULT_HEIGHT = 300;

    private static SharedPreferences preferences;

    //////////////

    private Prefs() {}
    static void Init( Activity _a ) {
        preferences = _a.getPreferences(Context.MODE_PRIVATE);
    }
    static void Cleanup() {}

    /////////////

    static int GetInt( String foo, int defvalue ) {
        return preferences.getInt( foo, defvalue );
    }
    static void SetInt( String foo, int newvalue ) {
        preferences.edit().putInt( foo, newvalue ).commit();
    }
    static boolean GetBool( String foo, boolean defvalue ) {
        return preferences.getBoolean( foo, defvalue );
    }
    static void SetBool( String foo, boolean newvalue ) {
        preferences.edit().putBoolean( foo, newvalue ).commit();
    }
    static boolean GetAndSetBool( String foo ) {
        boolean b = preferences.getBoolean( foo, false );
        if( !b ) { preferences.edit().putBoolean( foo, true ).commit(); }
        return b;
    }
}
