package com.tereslogica.droidartemis;

import android.database.Cursor;

import java.util.Arrays;

public class ArtemisTopic implements Comparable<ArtemisTopic> {

    // in schema order
    public String topic;
    public int scount;
    public int ssize;
    public int tsize;
    public String message;

    public ArtemisTopic() {}

    public ArtemisTopic( String _topic, int _scount, int _ssize, int _tsize, String _message ) {
        topic = _topic;
        scount = _scount;
        ssize = _ssize;
        tsize = _tsize;
        message = _message;
    }

    public ArtemisTopic( String _topic, int _ssize, int _tsize ) {
        topic = _topic;
        scount = 0;
        ssize = _ssize;
        tsize = _tsize;
        message = "";
    }

    public ArtemisTopic(Cursor cursor) {
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
        scount = cursor.getInt( ArtemisSQL.SCOUNT_COL );
        ssize = cursor.getInt( ArtemisSQL.SSIZE_COL );
        tsize = cursor.getInt( ArtemisSQL.TSIZE_COL );
        message = cursor.getString( ArtemisSQL.MESSAGE_COL );
    }

    @Override
    public int compareTo( ArtemisTopic o ) {
        return topic.compareTo( o.topic );
    }

    @Override
    public boolean equals( Object o ) {
        if (o == null) {
            return false;
        } else if (o instanceof ArtemisTopic) {
            ArtemisTopic t = (ArtemisTopic)o;
            if (topic != null && t.topic != null) {
                if (topic.length() == t.topic.length()) {
                    char[] cArr0 = topic.toCharArray();
                    char[] cArr1 = t.topic.toCharArray();
                    return Arrays.equals(cArr0, cArr1);
                } else {
                    return false;
                }
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
}
