package com.tereslogica.acanashare;

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
    public String cleartext;  // but not shown on topic_page
    public int indicateURIA;

    public String sc_str, ss_str, ts_str, m_str;

    public ArtemisTopic( String _topic, int _ssize, int _tsize, String _clue, String _location, int _mindicator ) {
        topic = _topic;
        scount = 0; // might be zero, if this share is an arecord
        ssize = _ssize;
        tsize = _tsize;
        cleartext = "?";
        clues = _clue;
        location = _location;
        indicateURIA = _mindicator;
    }

    public ArtemisTopic(Cursor cursor) {
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
        scount = cursor.getInt( ArtemisSQL.SCOUNT_COL );
        ssize = cursor.getInt( ArtemisSQL.SSIZE_COL );
        tsize = cursor.getInt( ArtemisSQL.TSIZE_COL );
        cleartext = cursor.getString( ArtemisSQL.CLEARTEXT_COL);
        clues = cursor.getString( ArtemisSQL.CLUES_COL );
        location = cursor.getString( ArtemisSQL.LOCATION_COL );
        indicateURIA = cursor.getInt( ArtemisSQL.URIAFLAG_COL);

        sc_str = Integer.toString( scount );
        ss_str = Integer.toString( ssize );
        ts_str = Integer.toString( tsize );
        m_str = indicateURIA == 1 ? "M" : "";
    }

    public void addClue( String clue ) {
        if( clue.length() > 0 ) { clues = clues+", "+clue; }
    }

    public void incCount() {
        scount++;
    }

    public void setIndicateURIA() { indicateURIA = 1; }
    public boolean isURIAPresent() { return indicateURIA == 1; }

    ///////////

    public static void configureTags( View rowView ) {
        rowView.setTag( R.id.topic, ((TextView) rowView.findViewById( R.id.topic )) );
        rowView.setTag( R.id.details, ((TextView) rowView.findViewById( R.id.details )) );
        rowView.setTag( R.id.clues, ((TextView) rowView.findViewById( R.id.clues )) );
    }

    public void configureView( View rowView ) {
        ((TextView) rowView.getTag( R.id.topic )).setText( topic );

        StringBuilder sb = new StringBuilder(80);
        if( cleartext.length() > 0 ) { sb.append('*'); sb.append(' '); }
        sb.append(m_str); sb.append(' ');
        sb.append(sc_str); sb.append('K'); sb.append(' ');
        sb.append(ts_str); sb.append('L'); sb.append(' ');
        sb.append(ss_str); sb.append('T');
        ((TextView) rowView.getTag( R.id.details )).setText( sb.toString() );

        // http://fupeg.blogspot.ca/2010/01/strikethrough-android.html
        //TextView mindicatorView = ((TextView) rowView.getTag( R.id.indicateURIA ));
        //int mibits = mindicatorView.getPaintFlags() | Paint.STRIKE_THRU_TEXT_FLAG; // add
        //if( indicateURIA != 0 ) { mibits ^= Paint.STRIKE_THRU_TEXT_FLAG; }        // remove
        //if( scount >= tsize ) { mibits |= Paint.FAKE_BOLD_TEXT_FLAG; }
        //mindicatorView.setPaintFlags( mibits );

        ((TextView) rowView.getTag( R.id.clues )).setText( clues );
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
