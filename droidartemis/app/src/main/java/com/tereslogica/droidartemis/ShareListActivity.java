package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;

public class ShareListActivity extends Activity {
    private String topic;
    private String aRecord = "";
    private String message;

    private ArtemisSQL artemisSql;
    private Notifier notifier;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.NATURAL;

    private ArrayList<ArtemisShare> shareArrayList = new ArrayList<ArtemisShare>();
    private ShareArrayAdapter saa;

    public static final int ACTIVITY_COMPLETE = 0;

    ////////////////////////////////////

    private class ShareArrayLoader extends AsyncTask<Cursor,Void,Long> {

        private ArrayList<ArtemisShare> al = new ArrayList<ArtemisShare>();

        @Override
        protected void onPreExecute() {
            al.clear();
        }

        @Override
        protected Long doInBackground(Cursor ... cursors) {
            if( cursors == null ) return null;
            if( cursors.length != 1 ) return null;
            if( cursors[0] == null ) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                do {
                    ArtemisShare oShare = new ArtemisShare( cursor );
                    if( oShare.stype == ArtemisLib.URI_A ) {
                        aRecord = oShare.share;
                    } else {
                        al.add( oShare );
                    }
                } while( cursor.moveToNext() );
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            shareArrayList.clear();
            for( ArtemisShare item : al) shareArrayList.add( item ); // addAll
            saa.notifyDataSetChanged();

            ArtemisTopic oTopic = artemisSql.getTopicInfo( topic );

            String message = oTopic.message;
            if( oTopic.message.length() == 0 ) { message = getResources().getString( R.string.sharelist_needmorekeys ); }
            if( aRecord.length() == 0 ) { message = getResources().getString( R.string.sharelist_messagedecodeerror ); }
            ((TextView) findViewById( R.id.message ) ).setText( message );

            String secret = aRecord;
            if( aRecord.length() == 0 ) { secret = getResources().getString( R.string.sharelist_nosecret ); }
            ((TextView) findViewById( R.id.secret ) ).setText( secret );
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class ShareArrayAdapter extends ArrayAdapter<ArtemisShare> {
        LayoutInflater inflater;

        public ShareArrayAdapter( Context cxt ) {
            super( cxt, R.layout.share_item, R.id.share, shareArrayList );
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if( convertView == null ) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.share_item, parent, false);
                ArtemisShare.configureTags( rowView );
            } else {
                rowView = convertView;
            }
            shareArrayList.get(position).configureView( rowView );
            return rowView;
        }
    }

    ////////////////////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.share_page);

        notifier = new Notifier( this );
        artemisSql = new ArtemisSQL( this );

        topic = getIntent().getStringExtra( "topic" );

        saa = new ShareArrayAdapter( getApplicationContext() );
        ListView lv = (ListView) findViewById( R.id.key_list );
        lv.setAdapter(saa);

        refreshListView();
    }

    ///////////////////////////

    public void refreshListView() {
        Cursor cursor = artemisSql.getShareTopicCursor( topic, sortOrder );
        if( cursor != null ) {
            (new ShareArrayLoader()).execute(cursor);
        }
    }

    ///////////////////////////

    private boolean continueSharing;
    public void onClickShare(View v) {

        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        try {
            continueSharing = true;
            if( aRecord.length() > 0 ) {
                Intent intent = new Intent("com.google.zxing.client.android.ENCODE");
                intent.putExtra("ENCODE_FORMAT", "QR_CODE");
                intent.putExtra("ENCODE_TYPE", "TEXT_TYPE");
                intent.putExtra("ENCODE_DATA", aRecord);
                intent.putExtra("ENCODE_SHOW_CONTENTS", false);
                startActivityForResult(intent, ACTIVITY_COMPLETE);
            }
            for( ArtemisShare item : shareArrayList )
            {
                if( continueSharing == false ) { break; }
                Intent intent = new Intent("com.google.zxing.client.android.ENCODE");
                intent.putExtra("ENCODE_FORMAT", "QR_CODE");
                intent.putExtra("ENCODE_TYPE", "TEXT_TYPE");
                intent.putExtra("ENCODE_DATA", item.share);
                intent.putExtra("ENCODE_SHOW_CONTENTS", false);
                startActivityForResult(intent, ACTIVITY_COMPLETE);
            }
        } catch (Exception e1) {
            notifier.showOk(R.string.dialog_noscanner);
            continueSharing = false;
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        switch( requestCode ) {
            case ACTIVITY_COMPLETE:
                if (resultCode == RESULT_OK) {
                    break;
                } else if (resultCode == RESULT_CANCELED) {
                    continueSharing = false;
                    break;
                    //handle cancel
                }
                break;
            default:
                break;
        }
    }

    ///////////////////////////
}
