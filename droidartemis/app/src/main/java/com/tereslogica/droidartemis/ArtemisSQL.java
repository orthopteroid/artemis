package com.tereslogica.droidartemis;

import android.content.ContentValues;
import android.content.Context;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.util.Log;

// http://myandroidtipsandtricks.blogspot.ca/2011/10/using-sqlite-database-in-android.html
// http://androidgreeve.blogspot.ca/2013/12/Android-Basic-SQLLite-Database-tutorial.html
// http://www.drdobbs.com/database/using-sqlite-on-android/232900584?pgno=2

public class ArtemisSQL extends SQLiteOpenHelper {

    private static final int VERSION = 1;
    private static final String NAME = "db";

    private static final String TOPICS = "topics";
    private static final String SHARES = "shares";

    private static final String KEY             = "_id";
    private static final String KEY_TYPE        = "INTEGER PRIMARY KEY AUTOINCREMENT";
    private static final String TOPIC           = "topic";
    private static final String TOPIC_TYPE      = "TEXT NOT NULL UNIQUE";
    private static final String SHARE           = "share";
    private static final String SHARE_TYPE      = "TEXT NOT NULL UNIQUE";
    private static final String MESSAGE         = "message";
    private static final String MESSAGE_TYPE    = "TEXT NOT NULL DEFAULT(' ')";

    ContentValues   item = new ContentValues();

    public ArtemisSQL(Context context) {
        super(context, NAME, null, VERSION);
    }

    public void addShare( String topic, String share ) {
        String message = "";
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("INSERT INTO "+SHARES+" VALUES ( '"+share+"' , '"+topic+"' );");
        db.execSQL("INSERT INTO "+TOPICS+" VALUES ( '"+topic+"' , '"+message+"' );");
        db.close();
    }

    public void delTopic( String topic ) {
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("DELETE FROM "+SHARES+" WHERE "+TOPIC+" = '"+topic+"' );");
        db.execSQL("DELETE FROM "+TOPICS+" WHERE "+TOPIC+" = '"+topic+"' );");
        db.close();
    }

    public void updateMessage( String topic, String message ) {
        SQLiteDatabase db = this.getWritableDatabase();
        db.execSQL("UPDATE "+TOPICS+" SET "+MESSAGE+" = '"+message+"' WHERE "+TOPIC+" = '"+topic+"';");
        db.close();
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        db.execSQL("CREATE TABLE "+SHARES+" ( "+KEY+" "+KEY_TYPE+" , "+SHARE+" "+SHARE_TYPE+" , "+TOPIC+" "+TOPIC_TYPE+");");
        db.execSQL("CREATE TABLE "+TOPICS+" ( "+KEY+" "+KEY_TYPE+" , "+TOPIC+" "+TOPIC_TYPE+" , "+MESSAGE+" "+MESSAGE_TYPE+" );");
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {
    }
}
