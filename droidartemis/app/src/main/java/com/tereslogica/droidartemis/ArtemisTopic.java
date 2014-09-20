package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.view.View;
import android.widget.TextView;

import java.util.Arrays;

public class ArtemisTopic implements Comparable<ArtemisTopic> {

    // in schema order
    public String topic;
    public String clues;
    public String location;
    public int scount;
    public int ssize;
    public int tsize;
    public String message;

    public boolean readyToSetMessage() {
        return message.length() == 0 && scount >= tsize;
    }

    public ArtemisTopic( String _topic, int _ssize, int _tsize, String _clue, String _location ) {
        topic = _topic;
        scount = 1;
        ssize = _ssize;
        tsize = _tsize;
        message = "";
        clues = _clue;
        location = _location;
    }

    public ArtemisTopic(Cursor cursor) {
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
        scount = cursor.getInt( ArtemisSQL.SCOUNT_COL );
        ssize = cursor.getInt( ArtemisSQL.SSIZE_COL );
        tsize = cursor.getInt( ArtemisSQL.TSIZE_COL );
        message = cursor.getString( ArtemisSQL.MESSAGE_COL );
        clues = cursor.getString( ArtemisSQL.CLUES_COL );
        location = cursor.getString( ArtemisSQL.LOCATION_COL );
    }

    public void addClue( String clue ) {
        clues = clues+", "+clue;
    }

    public void incCount() {
        scount++;
    }

    public void configureView(View listItem) {
        ((TextView) listItem.getTag( R.id.loctopic )).setText( location+"/"+topic );
        ((TextView) listItem.getTag( R.id.details )).setText( Integer.toString( scount )+"/"+Integer.toString( tsize )+" ("+Integer.toString( ssize )+")" );
        ((TextView) listItem.getTag( R.id.message )).setText( message );
        ((TextView) listItem.getTag( R.id.clues )).setText( clues );
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
