package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.graphics.Paint;
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
    public String message;  // but not shown on topic_page
    public int mindicator;

    public ArtemisTopic( String _topic, int _ssize, int _tsize, String _clue, String _location, int _mindicator ) {
        topic = _topic;
        scount = 0; // might be zero, if this share is an arecord
        ssize = _ssize;
        tsize = _tsize;
        message = "?";
        clues = _clue;
        location = _location;
        mindicator = _mindicator;
    }

    public ArtemisTopic(Cursor cursor) {
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
        scount = cursor.getInt( ArtemisSQL.SCOUNT_COL );
        ssize = cursor.getInt( ArtemisSQL.SSIZE_COL );
        tsize = cursor.getInt( ArtemisSQL.TSIZE_COL );
        message = cursor.getString( ArtemisSQL.MESSAGE_COL );
        clues = cursor.getString( ArtemisSQL.CLUES_COL );
        location = cursor.getString( ArtemisSQL.LOCATION_COL );
        mindicator = cursor.getInt( ArtemisSQL.MINDICATOR_COL );
    }

    public void addClue( String clue ) {
        clues = clues+", "+clue;
    }

    public void incCount() {
        scount++;
    }

    public void setMIndicator() { mindicator = 1; }

    ///////////

    public static void configureTags( View rowView ) {
        rowView.setTag( R.id.topic, ((TextView) rowView.findViewById( R.id.topic )) );
        rowView.setTag( R.id.details, ((TextView) rowView.findViewById( R.id.details )) );
        rowView.setTag( R.id.clues, ((TextView) rowView.findViewById( R.id.clues )) );
    }

    public void configureView( View listItem ) {
        ((TextView) listItem.getTag( R.id.topic )).setText( topic );

        // http://fupeg.blogspot.ca/2010/01/strikethrough-android.html
        String detailsString = Integer.toString( scount )+"/"+Integer.toString( tsize )+"/"+Integer.toString( ssize );
        TextView detailsView = ((TextView) listItem.getTag( R.id.details ));
        int mibits = detailsView.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG; // add
        if( mindicator != 0 ) { mibits ^= Paint.STRIKE_THRU_TEXT_FLAG; }        // remove
        if( scount >= tsize ) { mibits |= Paint.FAKE_BOLD_TEXT_FLAG; }
        detailsView.setPaintFlags( mibits );
        detailsView.setText(detailsString);

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
