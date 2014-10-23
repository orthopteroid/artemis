package com.tereslogica.droidartemis;

import android.database.Cursor;

/**
 * Created by john on 21/10/14.
 */
public class AppLogic {

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

    public void addItem( String share ) {
        ArtemisShare oShare = ArtemisSQL.Get().getShareInfo( share );
        if( oShare != null ) return; // got it already
        //
        int[] shareInfo = ArtemisLib.Get().nativeInfo( share );
        String topic = ArtemisLib.Get().nativeTopic(share);
        String clue = ArtemisLib.Get().nativeClue(share);
        //
        oShare = new ArtemisShare( share, topic, shareInfo[0] ); // sql api uses these
        //
        ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );
        if( oTopic == null ) {
            // if topic not found in db, then create new topic and share objects for db
            //
            String location = ArtemisLib.Get().nativeLocation(share);
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
            String recordArr = share;
            Cursor cursor = ArtemisSQL.Get().getShareTopicCursor( topic, ArtemisSQL.SortOrder.UNNATURAL );
            if (cursor.moveToFirst()) {
                do {
                    String othershare = cursor.getString( ArtemisSQL.SHARE_COL );
                    if( recordArr.length() > 0 ) { recordArr += "\n"; }
                    recordArr += othershare;
                } while( cursor.moveToNext() );
            }
            cursor.close();
            //
            oTopic.message = ArtemisLib.Get().nativeDecode( recordArr ); // szLocation baked into library
            //
            ArtemisSQL.Get().addShareUpdateTopic( oShare, oTopic );
        }
    }

}
