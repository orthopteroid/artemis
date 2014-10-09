package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.graphics.Paint;
import android.location.Location;
import android.view.View;
import android.widget.TextView;

import java.sql.Date;

public class ArtemisShare {

    // in schema order
    public String share;
    public String topic;
    public int stype;
    //public Date date;
    //public Location loc;

    public ArtemisShare() {}

    public ArtemisShare( String _share, String _topic, int _stype ) {
        share = _share;
        topic = _topic;
        stype = _stype;
    }

    public ArtemisShare( Cursor cursor ) {
        share = cursor.getString( ArtemisSQL.SHARE_COL );
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
        stype = cursor.getInt( ArtemisSQL.STYPE_COL );
    }

    ///////////

    public static void configureTags( View rowView ) {
        rowView.setTag( R.id.share, ((TextView) rowView.findViewById( R.id.share )) );
    }

    public void configureView( View listItem ) {
        ((TextView) listItem.getTag( R.id.share )).setText( share );
    }

}
