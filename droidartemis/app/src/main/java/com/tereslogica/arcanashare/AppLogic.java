package com.tereslogica.arcanashare;

import android.content.Context;
import android.database.Cursor;

/**
 * Created by john on 21/10/14.
 */
public class AppLogic {

    Context cxt;

    public boolean detectedError;
    public boolean detectedDecode;

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

    private void addToken_helper(String sToken) throws Exception {
        String uri = cxt.getResources().getString(R.string.app_loc);
        if( ArtemisLib.Get().nativeValidate( uri, sToken ) ) { detectedError = true; throw new Exception(); }

        ArtemisShare oShare = ArtemisSQL.Get().getShareInfo( sToken );
        if( oShare != null ) return; // got it already

        int[] shareInfo = ArtemisLib.Get().nativeInfo( sToken );
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }

        String topic = ArtemisLib.Get().nativeTopic(sToken);
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }

        String clue = ArtemisLib.Get().nativeClue(sToken);
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }

        oShare = new ArtemisShare( sToken, topic, shareInfo[0] ); // sql api uses these

        boolean newTopic = false;
        ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );
        if( oTopic == null ) {
            // if topic not found in db, then create new topic and sToken objects for db
            String location = ArtemisLib.Get().nativeLocation(sToken);
            if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }
            oTopic = new ArtemisTopic( topic, shareInfo[1], shareInfo[2], clue, location, 0 );
            newTopic = true;
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
    }

    private void checkForMessage( String topic ) {
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
        boolean decodeOK = ArtemisLib.Get().nativeDidFail() == false;
        boolean newMessage = oldMess.compareTo( newMess ) != 0;
        if( newMessage && decodeOK ) {
            ArtemisSQL.Get().updateTopicMessage(topic, newMess);
            detectedDecode = true;
        }
    }

    ///////////////

    public void addToken(String token) {
        detectedError = false;
        detectedDecode = false;
        //
        try {
            addToken_helper(token);

            String topic = ArtemisLib.Get().nativeTopic(token);
            checkForMessage(topic);
        } catch ( Exception e ) {
            // do nothing
        }
    }

    public void addTokenArray(String[] tokenArr) {
        detectedError = false;
        detectedDecode = false;
        //
        for( String token : tokenArr ) {
            try {
                addToken_helper(token);
            } catch ( Exception e ) {
                // do nothing, continue with all tokens
            }
        }

        if( tokenArr[0].length() > 0 ) {
            String topic = ArtemisLib.Get().nativeTopic(tokenArr[0]);
            checkForMessage(topic);
        }
    }

}
