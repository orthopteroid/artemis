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

    private ArrayList<ArtemisTopic> topicArrayList = new ArrayList<ArtemisTopic>();
    private TopicArrayAdapter taa;

    ////////////////////////////////////

    private class TopicArrayLoader extends AsyncTask<Cursor,Void,Long> {

        private ArrayList<ArtemisTopic> al = new ArrayList<ArtemisTopic>();

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
                    al.add(new ArtemisTopic(cursor));
                } while( cursor.moveToNext() );
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            topicArrayList.clear();
            for( ArtemisTopic item : al) topicArrayList.add( item ); // addAll
            taa.notifyDataSetChanged();
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class TopicArrayAdapter extends ArrayAdapter<ArtemisTopic> {
        LayoutInflater inflater;

        public TopicArrayAdapter(Context cxt) {
            super( cxt, R.layout.topic_item, R.id.loctopic, al );
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if( convertView == null ) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.topic_item, parent, false);
                ArtemisTopic.configureTags( rowView );
            } else {
                rowView = convertView;
            }
            topicArrayList.get(position).configureView( rowView );
            return rowView;
        }
    }

    //////////////////////
    // http://developer.android.com/reference/android/app/Activity.html#ActivityLifecycle

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.topic_page);

        notifier = new Notifier( this );
        artemisSql = new ArtemisSQL( this );
        artemisLib = new ArtemisLib();

        ////////////////

        OnItemClickListener oicl = new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String product = ((TextView) view.findViewById( R.id.loctopic ) ).getText().toString();
                Intent i = new Intent(getApplicationContext(), ShareListActivity.class);
                i.putExtra("product", product);
                startActivity(i);
            }
        };

        taa = new TopicArrayAdapter( getApplicationContext() );
        ListView lv = (ListView) findViewById( R.id.topic_list );
        lv.setAdapter(taa);
        lv.setOnItemClickListener(oicl);

        ////////////////

        refreshListView();
    }

    ///////////////////////////

    public void refreshListView() {
        Cursor cursor = artemisSql.getTopicsCursor( sortOrder );
        if( cursor != null ) {
            (new TopicArrayLoader()).execute(cursor);
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

    public void addScannedItem( String share ) {
        ArtemisShare oShare = artemisSql.getShareInfo( share );
        if( oShare != null ) return; // got it already
        //
        int[] shareInfo = artemisLib.nativeInfo( share );
        String topic = artemisLib.nativeTopic(share);
        String clue = artemisLib.nativeClue(share);
        //
        oShare = new ArtemisShare( share, topic ); // sql api uses these
        //
        ArtemisTopic oTopic = artemisSql.getTopicInfo( topic );
        if( oTopic == null ) {
            // if topic not found in db, then create new topic and share objects for db
            //
            String location = artemisLib.nativeLocation(share);
            oTopic = new ArtemisTopic( topic, shareInfo[1], shareInfo[2], clue, location, 0 );
            //
            if( shareInfo[0] == artemisLib.URI_B ) {
                oTopic.incCount(); // only B Types contribute towards the count
            } else {
                oTopic.setMIndicator(); // A types set the indicator
            }
            //
            artemisSql.addShareAndTopic( oShare, oTopic );
        } else {
            oTopic.addClue( clue );
            //&p[i], &q[i]
            if( shareInfo[0] == artemisLib.URI_B ) {
                oTopic.incCount(); // only B Types contribute towards the count
            } else {
                oTopic.setMIndicator(); // A types set the indicator
            }
            //
            String recordArr = share;
            Cursor cursor = artemisSql.getShareTopicCursor( topic );
            if (cursor.moveToFirst()) {
                do {
                    String othershare = cursor.getString( ArtemisSQL.SHARE_COL );
                    if( recordArr.length() > 0 ) { recordArr += "\n"; }
                    recordArr += othershare;
                } while( cursor.moveToNext() );
            }
            cursor.close();
            //
            oTopic.message = artemisLib.nativeDecode( recordArr ); // szLocation baked into library
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
