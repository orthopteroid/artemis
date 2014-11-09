package com.tereslogica.droidartemis;

import android.database.Cursor;

/**
 * Created by john on 21/10/14.
 */
public class AppLogic {

    public boolean detectedError;
    public boolean detectedDecode;

    ///////////////////

    private static AppLogic instance = null;

    private AppLogic() {}

    static void Init() {
        if( instance == null ) { instance = new AppLogic(); }
    }
    static void Cleanup() {
        instance = null;
    }
    static AppLogic Get() {
        return instance;
    }

    ///////////////////

    private void addToken_helper(String sToken) throws Exception {
        ArtemisShare oShare = ArtemisSQL.Get().getShareInfo( sToken );
        if( oShare != null ) return; // got it already
        //
        int[] shareInfo = ArtemisLib.Get().nativeInfo( sToken );
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }
        String topic = ArtemisLib.Get().nativeTopic(sToken);
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }
        String clue = ArtemisLib.Get().nativeClue(sToken);
        if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }
        //
        oShare = new ArtemisShare( sToken, topic, shareInfo[0] ); // sql api uses these
        //
        ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );
        if( oTopic == null ) {
            // if topic not found in db, then create new topic and sToken objects for db
            //
            String location = ArtemisLib.Get().nativeLocation(sToken);
            if( ArtemisLib.Get().nativeDidFail() ) { detectedError = true; throw new Exception(); }
            oTopic = new ArtemisTopic( topic, shareInfo[1], shareInfo[2], clue, location, 0 );
            //
            if( shareInfo[0] == ArtemisLib.URI_B ) {
                oTopic.incCount(); // only B Types contribute towards the count
            } else {
                oTopic.setMIndicator(); // A types set the indicator
            }
            //
            ArtemisSQL.Get().addShareAndTopic( oShare, oTopic );
        } else {
            oTopic.addClue( clue );
            //&p[i], &q[i]
            if( shareInfo[0] == ArtemisLib.URI_B ) {
                oTopic.incCount(); // only B Types contribute towards the count
            } else {
                oTopic.setMIndicator(); // A types set the indicator
            }
            //
            String tokenArrStr = sToken;
            Cursor cursor = ArtemisSQL.Get().getShareTopicCursor( topic, ArtemisSQL.SortOrder.UNNATURAL );
            if (cursor.moveToFirst()) {
                do {
                    String othershare = cursor.getString( ArtemisSQL.SHARE_COL );
                    if( tokenArrStr.length() > 0 ) { tokenArrStr += "\n"; }
                    tokenArrStr += othershare;
                } while( cursor.moveToNext() );
            }
            cursor.close();
            //
            String oldMess = oTopic.message;
            oTopic.message = ArtemisLib.Get().nativeDecode( /* "arcanashare.webhop.net", */ tokenArrStr );
            boolean decodeOK = ArtemisLib.Get().nativeDidFail() == false;
            boolean newMessage = oldMess.compareTo( oTopic.message ) != 0;
            if( newMessage && decodeOK ) { detectedDecode = true; } // set, without exception
            //
            ArtemisSQL.Get().addShareUpdateTopic( oShare, oTopic );
        }
    }

    ///////////////

    public void addToken(String token) {
        detectedError = false;
        detectedDecode = false;
        //
        try {
            AppLogic.Get().addToken_helper(token);
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
                AppLogic.Get().addToken_helper(token);
            } catch ( Exception e ) {
                // do nothing, continue with all tokens
            }
        }
    }

}
