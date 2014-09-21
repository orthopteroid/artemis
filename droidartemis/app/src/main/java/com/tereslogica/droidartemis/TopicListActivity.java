package com.tereslogica.droidartemis;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;

// http://www.androidhive.info/2011/10/android-listview-tutorial/
public class TopicListActivity extends FragmentActivity {

    public static final int ACTIVITY_SCAN = 0;

    ////////////////

    private static final boolean forceFakeScanner = true;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;

    private ArtemisLib artemisLib;
    private ArtemisSQL artemisSql;
    private Notifier notifier;
    private FakeScanner fs;

    private ArrayList<ArtemisTopic> al = new ArrayList<ArtemisTopic>();
    private TLAAdapter tlaa;

    ////////////////////////////////////

    private class TLALoader extends AsyncTask<Cursor,Void,Long> {

        private ArrayList<ArtemisTopic> tlal_al = new ArrayList<ArtemisTopic>();

        @Override
        protected void onPreExecute() {
            tlal_al.clear();
        }

        @Override
        protected Long doInBackground(Cursor ... cursors) {
            if( cursors == null ) return null;
            if( cursors.length != 1 ) return null;
            if( cursors[0] == null ) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                do {
                    tlal_al.add( new ArtemisTopic( cursor ) );
                } while( cursor.moveToNext() );
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            al.clear();
            for( ArtemisTopic at : tlal_al ) al.add( at ); // addAll
            tlaa.notifyDataSetChanged();
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class TLAAdapter extends ArrayAdapter<ArtemisTopic> {
        LayoutInflater inflater;

        public TLAAdapter( Context cxt ) {
            super( cxt, R.layout.list_item, R.id.loctopic, al );
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if( convertView == null ) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.list_item, parent, false);
                ArtemisTopic.configureTags( rowView );
            } else {
                rowView = convertView;
            }
            al.get( position ).configureView( rowView );
            return rowView;
        }
    }

    //////////////////////
    // http://developer.android.com/reference/android/app/Activity.html#ActivityLifecycle

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_my);

        notifier = new Notifier( this );
        artemisSql = new ArtemisSQL( this );
        artemisLib = new ArtemisLib();

        ////////////////

        OnItemClickListener oicl = new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String product = ((TextView) view.findViewById( R.id.loctopic ) ).getText().toString();
                Intent i = new Intent(getApplicationContext(), TopicActivity.class);
                i.putExtra("product", product);
                startActivity(i);
            }
        };

        tlaa = new TLAAdapter( getApplicationContext() );
        ListView lv = (ListView) findViewById( R.id.list_container );
        lv.setAdapter(tlaa);
        lv.setOnItemClickListener(oicl);

        ////////////////

        refreshListView();
    }

    ///////////////////////////

    public void refreshListView() {
        Cursor cursor = artemisSql.getTopicsCursor( sortOrder );
        if( cursor != null ) {
            (new TLALoader()).execute(cursor);
        }
    }

    public void onClickSort(View v) {
        DialogInterface.OnClickListener ocl = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int _which) {
                ArtemisSQL.SortOrder orderings[] = {
                        ArtemisSQL.SortOrder.MOSTRECENT,
                        ArtemisSQL.SortOrder.LEASTRECENT,
                        ArtemisSQL.SortOrder.MOSTCOMPLETE,
                        ArtemisSQL.SortOrder.LEASTCOMPLETE
                };
                sortOrder = orderings[_which];
                refreshListView();
            }
        };
        notifier.showOptions( R.array.dialog_sortshares, ocl);
    }

    public void onClickPurge(View v) {
        artemisSql.delAll();
        refreshListView();
    }

    public void onClickScan(View v) {

        // http://stackoverflow.com/questions/3103196/android-os-build-data-examples-please
        if( forceFakeScanner || Build.BRAND.equalsIgnoreCase("generic") ) {

            if( fs == null) {
                fs = new FakeScanner();
            }
            addScannedItem( fs.nextItem() );

        } else {

            // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
            try {
                Intent intent = new Intent("com.google.zxing.client.android.SCAN");
                intent.putExtra("SCAN_MODE", "QR_CODE_MODE");
                startActivityForResult( intent, ACTIVITY_SCAN );
            } catch (Exception e1) {
                notifier.showOk(R.string.dialog_noscanner);
            }

        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        switch( requestCode ) {
            case ACTIVITY_SCAN:
                if (resultCode == RESULT_OK) {
                    addScannedItem( data.getStringExtra("SCAN_RESULT") );
                } else if (resultCode == RESULT_CANCELED) {
                    //handle cancel
                }
                break;
            default:
                break;
        }
    }

    ///////////////////////////

    String zoo = new String();

    public void addScannedItem( String share ) {
        ArtemisShare oShare = artemisSql.getShareInfo( share );
        if( oShare != null ) return; // got it already
        Log.d("libartemis", "share " + share );
        //
        String topic = artemisLib.nativeShareField(share, "tp", 0);
        String clue = artemisLib.nativeShareClue(share);
        //
        oShare = new ArtemisShare( share, topic );
        ArtemisTopic oTopic = artemisSql.getTopicInfo( topic );
        if( oTopic == null ) {
            String location = artemisLib.nativeShareLocation( share );
            int[] shareInfo = artemisLib.nativeShareInfo( share );
            oTopic = new ArtemisTopic( topic, shareInfo[0], shareInfo[1], clue, location );
            artemisSql.addShareAndTopic( oShare, oTopic );
        } else {
            oTopic.addClue( clue );
            oTopic.incCount();
            //
            String foo = new String();
            Cursor cursor = artemisSql.getShareTopicCursor( topic );
            if (cursor.moveToFirst()) {
                do {
                    if( foo.length() > 0 ) foo += "\n";
                    foo += cursor.getString( ArtemisSQL.SHARE_COL );
                } while( cursor.moveToNext() );
            }
            cursor.close();
            oTopic.message = "foo!";//artemisLib.nativeDecode( foo );
            //
            artemisSql.addShareUpdateTopic( oShare, oTopic );
        }
        //
        refreshListView();
    }
    public void removeScannedItem( String item ) {
        Log.d("libartemis", "remove " + item);
    }
}
