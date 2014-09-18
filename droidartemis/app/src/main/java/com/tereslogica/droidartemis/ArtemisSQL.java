package com.tereslogica.droidartemis;

import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;

import java.util.ArrayList;
import java.util.List;

// http://myandroidtipsandtricks.blogspot.ca/2011/10/using-sqlite-database-in-android.html
// http://androidgreeve.blogspot.ca/2013/12/Android-Basic-SQLLite-Database-tutorial.html
// http://stackoverflow.com/questions/5310962/do-efficient-sqlite-inserts-with-android
// http://www.drdobbs.com/database/using-sqlite-on-android/232900584?pgno=2

public class ArtemisSQL extends SQLiteOpenHelper {

    private static final int VERSION = 1;
    private static final String NAME = "db";

    private static final String TOPICS = "topics";
    private static final String SHARES = "shares";

    private static final String KEY             = "_id";
    private static final String KEY_DEFAULT     = "NULL";
    private static final String KEY_TYPE        = "INTEGER PRIMARY KEY";
    private static final String KEY_DECL        = KEY+" "+KEY_TYPE;
    public  static final int    KEY_COL         = 0;
    private static final String TOPIC           = "topic";
    private static final String TOPIC_TYPE      = "TEXT NOT NULL";
    private static final String TOPIC_DECL      = TOPIC+" "+TOPIC_TYPE;
    public  static final int    TOPIC_COL       = 1;
    private static final String SHARE           = "share";
    private static final String SHARE_TYPE      = "TEXT NOT NULL UNIQUE";
    private static final String SHARE_DECL      = SHARE+" "+SHARE_TYPE;
    public  static final int    SHARE_COL       = 2; // only used in SHARES table
    private static final String SCOUNT          = "scount";
    private static final String SCOUNT_TYPE     = "INTEGER NOT NULL DEFAULT(0)";
    private static final String SCOUNT_DECL     = SCOUNT+" "+SCOUNT_TYPE;
    public  static final int    SCOUNT_COL      = 2;
    private static final String SSIZE           = "ssize";
    private static final String SSIZE_TYPE      = "INTEGER NOT NULL";
    private static final String SSIZE_DECL      = SSIZE+" "+SSIZE_TYPE;
    public  static final int    SSIZE_COL       = 3;
    private static final String TSIZE           = "tsize";
    private static final String TSIZE_TYPE      = "INTEGER NOT NULL";
    private static final String TSIZE_DECL      = TSIZE+" "+TSIZE_TYPE;
    public  static final int    TSIZE_COL       = 4;
    private static final String MESSAGE         = "message";
    private static final String MESSAGE_TYPE    = "TEXT NOT NULL";
    private static final String MESSAGE_DECL    = MESSAGE+" "+MESSAGE_TYPE;
    public  static final int    MESSAGE_COL     = 5;
    private static final String CALC_COMPLETE   = "complete";

    private void getTopicQueryParms( StringBuilder col, StringBuilder ord, SortOrder so ) {
        col.delete(0,ord.length());
        ord.delete(0,ord.length());
        switch( so ) {
            default:
            case MOSTRECENT:
                col.append(KEY);
                ord.append("DESC");
                break;
            case LEASTRECENT:
                col.append(KEY);
                ord.append("ASC");
                break;
            case MOSTCOMPLETE:
                col.append(CALC_COMPLETE);
                ord.append("DESC");
                break;
            case LEASTCOMPLETE:
                col.append(CALC_COMPLETE);
                ord.append("ASC");
                break;
        }
    }

    private void getShareQueryParms( StringBuilder col, StringBuilder ord, SortOrder so ) {
        col.delete(0,ord.length());
        ord.delete(0,ord.length());
        switch( so ) {
            default:
            case MOSTRECENT:
                ord.append(KEY);
                ord.append("DESC");
                break;
            case LEASTRECENT:
                ord.append(KEY);
                ord.append("ASC");
                break;
        }
    }

    /////////////////

    public enum SortOrder {
        MOSTRECENT, LEASTRECENT, MOSTCOMPLETE, LEASTCOMPLETE
    };

    public ArtemisSQL(Context context) {
        super(context, NAME, null, VERSION);
    }

    public ArtemisShare getShareInfo( String share ) {
        ArtemisShare oShare = null;
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery( "SELECT * FROM "+SHARES+" WHERE "+SHARE+" = '"+share+"' ;", null);
        if( cursor.moveToFirst() ) {
            oShare = new ArtemisShare();
            oShare.topic = cursor.getString(TOPIC_COL);
            oShare.share = cursor.getString(SHARE_COL);
        }
        cursor.close();
        db.close();
        return oShare;
    }

    public ArtemisTopic getTopicInfo( String topic ) {
        ArtemisTopic oTopic = null;
        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery( "SELECT * FROM "+TOPICS+" WHERE "+TOPIC+" = '"+topic+"' ;", null);
        if( cursor.moveToFirst() ) {
            oTopic = new ArtemisTopic( cursor );
        }
        cursor.close();
        db.close();
        return oTopic;
    }

    public void addShare( ArtemisShare oShare, ArtemisTopic oTopic ) {

        String topic = oTopic.topic;
        String share = oShare.share;
        Integer ssize = oTopic.ssize;
        Integer tsize = oTopic.tsize;

        SQLiteDatabase db = this.getWritableDatabase();

        int count = 0;
        Cursor cursor = db.rawQuery( "SELECT * FROM "+SHARES+" WHERE "+TOPIC+" = '"+topic+"' ;", null); // TODO: use aggregate function
        boolean nonZeroCollection = cursor.moveToFirst();
        if( nonZeroCollection ) count = cursor.getCount();
        cursor.close();

        db.execSQL("BEGIN;");
        db.execSQL("INSERT INTO "+SHARES+" VALUES ( "+KEY_DEFAULT+" , '"+topic+"' , '"+share+"' );"); // null for KEY
        if( nonZeroCollection ) {
            db.execSQL("UPDATE "+TOPICS+" SET "+SCOUNT+" = "+Integer.toString( 1 + count )+" WHERE "+TOPIC+" = '"+topic+"' ;");
        } else {
            // http://www.sqlite.org/autoinc.html
            // NULL will meet the INTEGER PRIMARY KEY requirement of KEY
            db.execSQL("INSERT INTO "+TOPICS+" VALUES ( "+KEY_DEFAULT+", '"+topic+"' , 1 , "+ssize.toString()+" , "+tsize.toString()+", ''  );");
        }
        db.execSQL("COMMIT;");
        db.close();
    }

    public void delTopic( ArtemisTopic oTopic ) {

        String topic = oTopic.topic;

        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("BEGIN;");
        db.execSQL("DELETE FROM "+SHARES+" WHERE "+TOPIC+" = '"+topic+"' );");
        db.execSQL("DELETE FROM "+TOPICS+" WHERE "+TOPIC+" = '"+topic+"' );");
        db.execSQL("COMMIT;");
        db.close();
    }

    public void updateMessage( ArtemisTopic oTopic ) {

        String topic = oTopic.topic;
        String message = oTopic.message;

        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("UPDATE "+TOPICS+" SET "+MESSAGE+" = '"+message+"' WHERE "+TOPIC+" = '"+topic+"';");
        db.close();
    }

    public Cursor getTopicsCursor( SortOrder so ) {
        SQLiteDatabase db = this.getReadableDatabase();
        List<ArtemisTopic> topicList = new ArrayList<ArtemisTopic>();
        StringBuilder col = new StringBuilder();
        StringBuilder ord = new StringBuilder();
        getTopicQueryParms(col, ord, so);
        Cursor cursor = db.rawQuery( "SELECT *, ROUND( "+SCOUNT+" / "+TSIZE+" , 2 ) AS '"+CALC_COMPLETE+"' FROM "+TOPICS+" ORDER BY "+col.toString()+" "+ord.toString()+";", null);
        boolean nonZeroCollection = cursor.moveToFirst();
        db.close();
        return cursor;
    }
/*
    public List<ArtemisTopic> getTopics( SortOrder so ) {
        SQLiteDatabase db = this.getReadableDatabase();
        List<ArtemisTopic> topicList = new ArrayList<ArtemisTopic>();
        StringBuilder col = new StringBuilder();
        StringBuilder ord = new StringBuilder();
        getTopicQueryParms( col, ord, so );
        Cursor cursor = db.rawQuery( "SELECT * FROM "+TOPICS+" ORDER BY "+col.toString()+" "+ord.toString()+";", null);
        if( cursor.moveToFirst() ) {
            do {
                ArtemisTopic oTopic = new ArtemisTopic( cursor );
                topicList.add(oTopic);
            } while (cursor.moveToNext());
        }
        db.close();
        cursor.close();
        return topicList;
    }

    public List<ArtemisShare> getShares( String topic, SortOrder so ) {
        SQLiteDatabase db = this.getReadableDatabase();
        List<ArtemisShare> shareList = new ArrayList<ArtemisShare>();
        StringBuilder col = new StringBuilder();
        StringBuilder ord = new StringBuilder();
        getShareQueryParms( col, ord, so );
        Cursor cursor = db.rawQuery( "SELECT * FROM "+SHARES+" WHERE "+TOPIC+" = '"+topic+"' ORDER BY "+col.toString()+" "+ord.toString()+";", null);
        if (cursor.moveToFirst()) {
            do {
                ArtemisShare share = new ArtemisShare( cursor );
                shareList.add(share);
            } while (cursor.moveToNext());
        }
        db.close();
        cursor.close();
        return shareList;
    }
*/
    public void reset() {
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("BEGIN;");
        db.execSQL("DROP TABLE IF EXISTS "+SHARES+" ;");
        db.execSQL("DROP TABLE IF EXISTS "+TOPICS+" ;");
        db.execSQL("COMMIT;");
        db.execSQL("BEGIN;");
        db.execSQL("CREATE TABLE "+SHARES+" ( "+KEY_DECL+" , "+TOPIC_DECL+" , "+SHARE_DECL+" );");
        db.execSQL("CREATE TABLE "+TOPICS+" ( "+KEY_DECL+" , "+TOPIC_DECL+" , "+SCOUNT_DECL+" , "+SSIZE_DECL+" , "+TSIZE_DECL+" , "+MESSAGE_DECL+" );");
        db.execSQL("COMMIT;");
        db.close();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        //db.execSQL("BEGIN;");
        db.execSQL("CREATE TABLE "+SHARES+" ( "+KEY_DECL+" , "+TOPIC_DECL+" , "+SHARE_DECL+" );");
        db.execSQL("CREATE TABLE "+TOPICS+" ( "+KEY_DECL+" , "+TOPIC_DECL+" , "+SCOUNT_DECL+" , "+SSIZE_DECL+" , "+TSIZE_DECL+" , "+MESSAGE_DECL+" );");
        //db.execSQL("COMMIT;");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
        db.execSQL("BEGIN;");
        db.execSQL("DROP TABLE IF EXISTS "+SHARES+" ;");
        db.execSQL("DROP TABLE IF EXISTS "+TOPICS+" ;");
        db.execSQL("COMMIT;");
        onCreate(db);
    }
}
