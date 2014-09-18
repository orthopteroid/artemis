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

    ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;

    ArtemisLib artemisLib;
    ArtemisSQL artemisSql;
    Notifier notifier;
    FakeScanner fs;

    ArrayList<ArtemisTopic> al = new ArrayList<ArtemisTopic>();
    TLAAdapter tla;

    ////////////////////////////////////

    private class TLALoader extends AsyncTask<Cursor,Void,Long> {
        private ArrayAdapter<ArtemisTopic> adapter;
        private ArrayList<ArtemisTopic> arraylist;
        private android.content.res.Resources resources;

        TLALoader(android.content.res.Resources res, ArrayAdapter<ArtemisTopic> aa, ArrayList<ArtemisTopic> arrl) {
            resources = res;
            adapter = aa;
            arraylist = arrl;
        }

        @Override
        protected void onPreExecute() {
            arraylist.clear();
            adapter.notifyDataSetChanged();
        }

        @Override
        protected Long doInBackground(Cursor ... cursors) {
            if( cursors == null ) return null;
            if( cursors.length != 1 ) return null;
            if( cursors[0] == null ) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                do {
                    arraylist.add( new ArtemisTopic( cursor ) );
                    //if (isCancelled()) break;
                } while( cursor.moveToNext() );
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            adapter.notifyDataSetChanged();
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class TLAAdapter extends ArrayAdapter<ArtemisTopic> {
        LayoutInflater inflater;

        public TLAAdapter() {
            super( getApplicationContext(), R.layout.list_item, R.id.loctopic, al );
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if( convertView == null ) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.list_item, parent, false);
                rowView.setTag( R.id.loctopic, ((TextView) rowView.findViewById( R.id.loctopic )) );
                rowView.setTag( R.id.details, ((TextView) rowView.findViewById( R.id.details )) );
                rowView.setTag( R.id.message, ((TextView) rowView.findViewById( R.id.message )) );
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
                String product = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                Intent i = new Intent(getApplicationContext(), TopicActivity.class);
                i.putExtra("product", product);
                startActivity(i);
            }
        };

        tla = new TLAAdapter();
        ListView lv = (ListView) findViewById( R.id.list_container );
        lv.setAdapter(tla);
        lv.setOnItemClickListener(oicl);

        ////////////////

        Cursor cursor = artemisSql.getTopicsCursor( sortOrder );
        if( cursor != null ) {
            (new TLALoader(getResources(), tla, al)).execute(cursor);
        }
    }

    ///////////////////////////

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
            }
        };
        notifier.showOptions( R.array.dialog_sortshares, ocl);
    }

    public void onClickPurge(View v) {
        artemisSql.reset();

        ////////////////

        Cursor cursor = artemisSql.getTopicsCursor( sortOrder );
        if( cursor != null ) {
            (new TLALoader(getResources(), tla, al)).execute(cursor);
        }
    }

    public void onClickScan(View v) {

        // http://stackoverflow.com/questions/3103196/android-os-build-data-examples-please
        if (Build.BRAND.equalsIgnoreCase("generic")) {

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
                    addScannedItem(data.getStringExtra("SCAN_RESULT"));
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
        Log.d("libartemis", "add " + share);
        String topic = artemisLib.nativeShareField(share, "tp", 0);
        Log.d("libartemis", "topic " + topic);
        //
        ArtemisTopic oTopic = artemisSql.getTopicInfo( topic );
        if( oTopic == null ) {
            int[] shareInfo = artemisLib.nativeShareInfo( share );
            oTopic = new ArtemisTopic( topic, shareInfo[0], shareInfo[1] );
        }
        //
        ArtemisShare oShare = artemisSql.getShareInfo( share );
        if( oShare == null ) {
            oShare = new ArtemisShare( share, topic ); // REVIEW: temporary object
            artemisSql.addShare( oShare, oTopic );
        }
        //
        if( !al.contains( oTopic ) ) {
            al.add( oTopic );
            tla.notifyDataSetChanged();
        }
        //
/*
        if( zoo.length() == 0 ) zoo = item; else zoo += "\n" + item;
        String foo = new String();
        sal.add( item );
        int i = 0;
        for( String s : sal ) {
            if( i == 0 ) foo = s; else foo += "\n" + s;
            i++;
        }
        // http://mhandroid.wordpress.com/2011/01/25/how-cc-debugging-works-on-android/
        // ./adb root ; ./adb shell stop ; ./adb shell setprop log.redirect-stdio true ; ./adb shell start
        String bar = artemisLib.nativeDecode(zoo);
        boolean ok = artemisLib.nativeGetStatusOK();
        if( ok == false ) bar = "*NODECODE*";
        Log.d("libartemis", "returned " + bar);
*/
    }
    public void removeScannedItem( String item ) {
        Log.d("libartemis", "remove " + item);
    }
}
