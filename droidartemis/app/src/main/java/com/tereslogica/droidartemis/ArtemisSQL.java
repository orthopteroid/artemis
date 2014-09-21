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
    private static final String MESSAGE_DEFAULT = "''";
    private static final String MESSAGE_TYPE    = "TEXT NOT NULL";
    private static final String MESSAGE_DECL    = MESSAGE+" "+MESSAGE_TYPE;
    public  static final int    MESSAGE_COL     = 5;
    private static final String CLUES           = "clues";
    private static final String CLUES_DEFAULT   = "''";
    private static final String CLUES_TYPE      = "TEXT NOT NULL";
    private static final String CLUES_DECL      = CLUES+" "+CLUES_TYPE;
    public  static final int    CLUES_COL       = 6;
    private static final String LOCATION        = "location";
    private static final String LOCATION_TYPE   = "TEXT NOT NULL";
    private static final String LOCATION_DECL   = LOCATION+" "+LOCATION_TYPE;
    public  static final int    LOCATION_COL    = 7;
    private static final String CALC_COMPLETE   = "complete";

    private static final String TOPICS = "topics";
    private static final String SHARES = "shares";
    private static final String SHARES_SCHEMA = KEY_DECL+" , "+TOPIC_DECL+" , "+SHARE_DECL;
    private static final String TOPICS_SCHEMA = KEY_DECL+" , "+TOPIC_DECL+" , "+SCOUNT_DECL+" , "
            +SSIZE_DECL+" , "+TSIZE_DECL+" , "+MESSAGE_DECL+" , "+CLUES_DECL+" , "+LOCATION_DECL;

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

    /////////////////

    public ArtemisTopic getTopicInfo( String topic ) {
        ArtemisTopic oTopic = null;
        SQLiteDatabase db = this.getReadableDatabase();
        String query = "SELECT * FROM "+TOPICS+" "+" WHERE "+TOPIC+" = ? ;";
        Cursor cursor = db.rawQuery(query, new String[] { topic } );
        if( cursor.moveToFirst() ) {
            oTopic = new ArtemisTopic( cursor );
        }
        cursor.close();
        db.close();
        return oTopic;
    }

    public void addShareUpdateTopic( ArtemisShare oShare, ArtemisTopic oTopic) {
        String topic = oTopic.topic;
        String clues = oTopic.clues;
        String message = oTopic.message;
        String count = Integer.toString( oTopic.scount );
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("BEGIN;");
        String query1 = "INSERT INTO "+SHARES+" VALUES ( "+KEY_DEFAULT+", ? , ? );";
        Cursor cursor1 = db.rawQuery( query1, new String [] { oShare.topic, oShare.share } );
        cursor1.moveToFirst();
        cursor1.close();
        String query2 = "UPDATE "+TOPICS+" SET "+SCOUNT+" = ? , "+CLUES+" = ? , "+MESSAGE+" = ? WHERE "+TOPIC+" = ? ;";
        Cursor cursor2 = db.rawQuery( query2, new String [] { count, clues, message, topic } );
        cursor2.moveToFirst();
        cursor2.close();
        db.execSQL("COMMIT;");
        db.close();
    }

    public Cursor getTopicsCursor( SortOrder so ) {
        StringBuilder col = new StringBuilder();
        StringBuilder ord = new StringBuilder();
        getTopicQueryParms(col, ord, so);
        //
        SQLiteDatabase db = this.getReadableDatabase();
        String query = "SELECT *, ROUND( "+SCOUNT+" / "+TSIZE+" , 2 ) AS '"+CALC_COMPLETE+"' FROM "+TOPICS+" ORDER BY "+col.toString()+" "+ord.toString()+" ;";
        Cursor cursor = db.rawQuery( query, null );
        cursor.moveToFirst();
        db.close();
        return cursor;
    }

    /////

    public Cursor getShareTopicCursor( String topic ) {
        SQLiteDatabase db = this.getReadableDatabase();
        String query = "SELECT * FROM "+SHARES+" "+"WHERE "+TOPIC+" = ? ;";
        Cursor cursor = db.rawQuery( query, new String[] { topic } );
        cursor.moveToFirst();
        db.close();
        return cursor;
    }

    //////

    public ArtemisShare getShareInfo( String share ) {
        ArtemisShare oShare = null;
        SQLiteDatabase db = this.getReadableDatabase();
        String query = "SELECT * FROM "+SHARES+" WHERE "+SHARE+" = ? ;";
        Cursor cursor = db.rawQuery( query, new String[] { share } );
        if( cursor.moveToFirst() ) {
            oShare = new ArtemisShare( cursor );
        }
        cursor.close();
        db.close();
        return oShare;
    }

    public void addShareAndTopic( ArtemisShare oShare, ArtemisTopic oTopic ) {
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("BEGIN;");
        String query1 = "INSERT INTO "+SHARES+" VALUES ( "+KEY_DEFAULT+", ?, ? );";
        Cursor cursor1 = db.rawQuery( query1, new String[] { oShare.topic, oShare.share } );
        cursor1.moveToFirst();
        cursor1.close();
        String query2 = "INSERT INTO "+TOPICS+" VALUES ( "+KEY_DEFAULT+", ?, ?, ?, ?, "+MESSAGE_DEFAULT+", ?, ? );";
        Cursor cursor2 = db.rawQuery( query2, new String[] {
                oTopic.topic,
                Integer.toString( oTopic.scount ), Integer.toString( oTopic.ssize ), Integer.toString( oTopic.tsize ),
                oTopic.clues, oTopic.location
        } );
        cursor2.moveToFirst();
        cursor2.close();
        db.execSQL("COMMIT;");
        db.close();
    }

    //////////////

    public void delAll() {
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("BEGIN;");
        db.execSQL("DROP TABLE IF EXISTS "+SHARES+" ;");
        db.execSQL("DROP TABLE IF EXISTS "+TOPICS+" ;");
        db.execSQL("COMMIT;");
        db.execSQL("BEGIN;");
        db.execSQL("CREATE TABLE "+SHARES+" ( "+SHARES_SCHEMA+" );");
        db.execSQL("CREATE TABLE "+TOPICS+" ( "+TOPICS_SCHEMA+" );");
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

    ////////////////

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE "+SHARES+" ( "+SHARES_SCHEMA+" );");
        db.execSQL("CREATE TABLE "+TOPICS+" ( "+TOPICS_SCHEMA+" );");
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
