package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.location.Location;
import java.sql.Date;

public class ArtemisShare {

    // in schema order
    public String share;
    public String topic;
    //public Date date;
    //public Location loc;

    public ArtemisShare() {}

    public ArtemisShare( String _share, String _topic ) {
        share = _share;
        topic = _topic;
    }

    public ArtemisShare( Cursor cursor ) {
        share = cursor.getString( ArtemisSQL.SHARE_COL );
        topic = cursor.getString( ArtemisSQL.TOPIC_COL );
    }

}
