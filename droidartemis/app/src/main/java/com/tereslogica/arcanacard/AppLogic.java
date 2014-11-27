package com.tereslogica.arcanacard;

import android.content.Context;
import android.content.DialogInterface;
import android.database.Cursor;

/**
 * Created by john on 21/10/14.
 */
public class AppLogic {

    Context cxt;

    public final static int RC_OK =         0x0000;
    public final static int RC_ERROR =      0x0001;
    public final static int RC_DUPLICATE =  0x0002;
    public final static int RC_NEW =        0x0004;
    //
    public final static int RC_MESSAGE =    0x0010;
    public final static int RC_KEY =        0x0020;
    public final static int RC_TOPIC =      0x0040;
    //
    public final static int RC_CLUE =       0x0100;
    public final static int RC_UNLOCKED =   0x0200;

    public static boolean RCTest( int rc, int bit )     { return (rc & bit) == bit; }
    public static int RCStrip( int rc, int bit )        { return (rc & ~bit); }

    ///////////////////

    private static AppLogic instance = null;

    private AppLogic( Context _cxt ) { cxt = _cxt; }

    static void Init( Context _cxt ) {
        if( instance == null ) { instance = new AppLogic( _cxt ); }
    }
    static void Cleanup() {
        instance = null;
    }
    static AppLogic Get() {
        return instance;
    }

    ///////////////////

    private int addToken_helper(String sToken) {
        int rc = RC_OK;
        try {
            String uri = cxt.getResources().getString(R.string.app_loc);
            if( ArtemisLib.Get().nativeValidate( uri, sToken ) ) { rc = RC_ERROR; throw new Exception(); }

            ArtemisShare oShare = ArtemisSQL.Get().getShareInfo( sToken );
            if( oShare != null ) { rc = RC_DUPLICATE; throw new Exception(); } // got it already

            int[] shareInfo = ArtemisLib.Get().nativeInfo( sToken );
            if( ArtemisLib.Get().nativeDidFail() ) { rc = RC_ERROR; throw new Exception(); }

            String topic = ArtemisLib.Get().nativeTopic(sToken);
            if( ArtemisLib.Get().nativeDidFail() ) { rc = RC_ERROR; throw new Exception(); }

            String clue = ArtemisLib.Get().nativeClue(sToken);
            if( ArtemisLib.Get().nativeDidFail() ) { rc = RC_ERROR; throw new Exception(); }

            if( clue.length() > 0 ) { rc |= RC_CLUE; }

            oShare = new ArtemisShare( sToken, topic, shareInfo[0] ); // sql api uses these

            if( shareInfo[0] == ArtemisLib.URI_A ) {
                rc |= RC_NEW | RC_MESSAGE;
            } else {
                rc |= RC_NEW | RC_KEY;
            }

            boolean newTopic = false;
            ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );
            if( oTopic == null ) {
                // if topic not found in db, then create new topic and sToken objects for db
                String location = ArtemisLib.Get().nativeLocation(sToken);
                if( ArtemisLib.Get().nativeDidFail() ) { rc = RC_ERROR; throw new Exception(); }

                oTopic = new ArtemisTopic( topic, shareInfo[1], shareInfo[2], clue, location, 0 );
                newTopic = true;

                rc |= RC_TOPIC;
            } else {
                // topic already present. append share clue to topic.
                oTopic.addClue( clue );
            }

            if( shareInfo[0] == ArtemisLib.URI_B ) {
                oTopic.incCount(); // only B Types contribute towards the count
            } else {
                oTopic.setIndicateURIA(); // A types set the indicator
            }

            if( newTopic ) {
                ArtemisSQL.Get().addShareAndTopic(oShare, oTopic);
            } else {
                ArtemisSQL.Get().addShareUpdateTopic(oShare, oTopic);
            }
        } catch( Exception e ) {
            // do nothing
        }
        return rc;
    }

    private boolean checkForMessage( String topic ) {
        StringBuilder sb = new StringBuilder(80);
        Cursor cursor = ArtemisSQL.Get().getShareTopicCursor( topic, ArtemisSQL.SortOrder.UNNATURAL );
        if (cursor.moveToFirst()) {
            do {
                if( sb.length() > 0 ) { sb.append('\n'); }
                sb.append( cursor.getString( ArtemisSQL.SHARE_COL ) );
            } while( cursor.moveToNext() );
        }
        cursor.close();

        String uri = cxt.getResources().getString(R.string.app_loc);
        String oldMess = ArtemisSQL.Get().getTopicInfo( topic ).cleartext;
        String newMess = ArtemisLib.Get().nativeDecode( uri, sb.toString() );
        if( ArtemisLib.Get().nativeDidFail() == false ) {
            if( oldMess.compareTo( newMess ) != 0 ) {
                ArtemisSQL.Get().updateTopicMessage(topic, newMess);
                return true;
            }
        }
        return false;
    }

    ///////////////

    public int addToken(String token) {
        int rc = addToken_helper( token );

        String topic = ArtemisLib.Get().nativeTopic(token);
        if( checkForMessage( topic ) ) { rc |= RC_UNLOCKED; }

        return rc;
    }

    public int addTokenArray(String[] tokenArr) {
        int rc = RC_OK;

        for( String token : tokenArr ) {
            rc |= addToken_helper( token ); // integrate messages, except errors
        }

        if( tokenArr[0].length() > 0 ) {
            String topic = ArtemisLib.Get().nativeTopic(tokenArr[0]);
            if( checkForMessage( topic ) ) { rc |= RC_UNLOCKED; }
        }

        return rc;
    }

    public void provideUserFeedback( int rc, String share ) {
        if( RCTest(rc, RC_ERROR) ) {
            Notifier.ShowOk( cxt, R.string.dialog_err_decode, null );
            return;
        }

        // always show clue first
        if( RCTest(rc, RC_NEW | RC_CLUE) ) {
            final int rcf = rc;
            final String sharef = share;
            final String cluef = ArtemisLib.Get().nativeClue( share );

            Notifier.ShowMessageOk( cxt, R.string.dialog_info_clue, cluef, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int _which) {
                    provideUserFeedback( RCStrip( rcf, RC_NEW | RC_CLUE ), sharef );
                }
            });
            return;
        }

        // show message
        if( RCTest(rc, RC_UNLOCKED) ) {
            String topic = ArtemisLib.Get().nativeTopic( share );
            ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );

            Notifier.ShowMessageOk(cxt, R.string.dialog_info_decode, oTopic.cleartext, null);
            return;
        }

        if( RCTest(rc, RC_NEW | RC_TOPIC) ) {
            String topic = ArtemisLib.Get().nativeTopic( share );

            Notifier.ShowMessageOk(cxt, R.string.dialog_info_topic, topic, null);
            return;
        }
    }
}
