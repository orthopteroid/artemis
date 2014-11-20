package com.tereslogica.arcanashare;

import android.database.Cursor;
import android.graphics.Bitmap;
import android.view.View;
import android.widget.ImageView;
import android.widget.TextView;

public class ArtemisShare {

    // in schema order
    public String share;
    public String topic;
    public int stype;
    //public Date date;
    //public Location loc;

    public Bitmap qrBitmap;
    public String qrClue;

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

    public ArtemisShare buildQRCode() throws Exception {
        int width = Prefs.GetInt( Prefs.QR_WIDTH, 300 );
        int height = Prefs.GetInt( Prefs.QR_WIDTH, 300 );
        qrBitmap = Util.StringToQRBitmap(share, width, height);
        qrClue = ArtemisLib.Get().nativeClue( share );
        return this;
    }

    ///////////

    public static void ConfigureTags(View rowView) {
        rowView.setTag( R.id.qrbitmapview, ((ImageView) rowView.findViewById( R.id.qrbitmapview)) );
        rowView.setTag( R.id.qrtextview, ((TextView) rowView.findViewById( R.id.qrtextview)) );
    }

    public void configureView( View rowView ) {
        ((ImageView) rowView.getTag( R.id.qrbitmapview)).setImageBitmap( qrBitmap );
        ((TextView) rowView.getTag( R.id.qrtextview)).setText( qrClue );
    }

}
