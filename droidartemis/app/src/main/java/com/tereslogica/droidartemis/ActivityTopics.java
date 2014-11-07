package com.tereslogica.droidartemis;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.support.v4.app.FragmentActivity;
import android.support.v4.content.LocalBroadcastManager;
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
public class ActivityTopics extends FragmentActivity {

    public static final int ACTIVITY_COMPLETE = 0;

    ////////////////

    private FragmentActivity thisActivity;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;

    private FakeScanner fs;

    private ArrayList<ArtemisTopic> topicArrayList = new ArrayList<ArtemisTopic>();
    private ListView listView;

    private BroadcastReceiver shareIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            //Notifier.ShowMessage(thisActivity, "sharing!" );
            new Packager( thisActivity, intent.getStringExtra(Notifier.EXTRA_TOPICSTRING) );
        }
    };

    private BroadcastReceiver packageIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Uri uri = Uri.parse( intent.getStringExtra(Notifier.EXTRA_URISTRING) );
            //Notifier.ShowMessage(thisActivity, "packaged uri: " + uri.toString() );
            Intent intent2 = new Intent( android.content.Intent.ACTION_SEND, uri );
            intent2.setType("application/zip");
            intent2.addFlags( Intent.FLAG_GRANT_READ_URI_PERMISSION );
            intent2.putExtra( Intent.EXTRA_STREAM, uri );
            startActivity(Intent.createChooser(intent2, "Send Images"));
        }
    };

    ////////////////////////////////////

    private class TopicArrayLoader extends AsyncTask<Cursor, Void, Long> {

        private ArrayList<ArtemisTopic> al = new ArrayList<ArtemisTopic>();

        @Override
        protected void onPreExecute() {
            al.clear();
        }

        @Override
        protected Long doInBackground(Cursor... cursors) {
            if (cursors == null) return null;
            if (cursors.length != 1) return null;
            if (cursors[0] == null) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                do {
                    al.add(new ArtemisTopic(cursor));
                } while (cursor.moveToNext());
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            topicArrayList.clear();
            for (ArtemisTopic item : al) topicArrayList.add(item); // addAll
            ((TopicArrayAdapter)listView.getAdapter()).notifyDataSetChanged();
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class TopicArrayAdapter extends ArrayAdapter<ArtemisTopic> {
        LayoutInflater inflater;

        public TopicArrayAdapter(Context cxt) {
            super(cxt, R.layout.topic_item, /* R.id.topic,*/ topicArrayList);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) { // view recycling and view holder pattern
                convertView = inflater.inflate(R.layout.topic_item, parent, false);
                ArtemisTopic.configureTags( convertView );
            }
            topicArrayList.get(position).configureView(convertView);
            return convertView;
        }
    }

    //////////////////////

    public void refreshListView() {
        Cursor cursor = ArtemisSQL.Get().getTopicsCursor(sortOrder);
        if (cursor != null) {
            (new TopicArrayLoader()).execute(cursor);
        }
    }

    ///////////////////////////

    public void addScannedItem( String share ) {
        AppLogic.Get().addToken(share);
        if( AppLogic.Get().detectedError ) { Notifier.ShowOk( this, R.string.dialog_err_parse); }
        if( AppLogic.Get().detectedDecode ) { Notifier.ShowOk( this, R.string.dialog_info_decode); }

        refreshListView();
    }

    public void removeScannedItem( String item ) {
        Log.d("libartemis", "remove " + item);
    }

    //////////////////////
    // http://developer.android.com/reference/android/app/Activity.html#ActivityLifecycle

    @Override
    protected void onDestroy() {
        LocalBroadcastManager.getInstance(this).unregisterReceiver(shareIntentReceiver);
        LocalBroadcastManager.getInstance(this).unregisterReceiver(packageIntentReceiver);
        super.onDestroy();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.topic_page);
        thisActivity = this;

        LocalBroadcastManager.getInstance(this).registerReceiver(shareIntentReceiver, new IntentFilter( Notifier.INTENT_SHARE ));
        LocalBroadcastManager.getInstance(this).registerReceiver(packageIntentReceiver, new IntentFilter( Notifier.INTENT_PACKAGE ));

        ArtemisSQL.Init( this );
        ArtemisLib.Init();
        AppLogic.Init();

        ////////////////

        listView = (ListView) findViewById(R.id.topic_list);
        listView.setAdapter(new TopicArrayAdapter(getApplicationContext()));
        /*
        listView.setOnItemLongClickListener(
                new AdapterView.OnItemLongClickListener() {
                    public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                        String topic = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                        ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );

                        boolean shareMode = false;
                        String message = oTopic.message;
                        if( oTopic.isARecordPresent() == false ) { message = getResources().getString( R.string.sharelist_nomessage ); }
                        else if( oTopic.message.length() == 0 ) { message = getResources().getString( R.string.sharelist_needmorekeys ); }
                        else { shareMode = true; }

                        if( shareMode ) {
                            Notifier.ShowMessageAndPosiblyShare(thisActivity, topic, message);
                        } else {
                            Notifier.ShowMessage(thisActivity, message);
                        }
                        return false;
                    }
                }
        );
        */
        listView.setOnItemClickListener(
            new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
/*
                    Intent i = new Intent(getApplicationContext(), ShareListActivity.class);
                    i.putExtra("topic", ((TextView) view.findViewById(R.id.topic)).getText().toString());
                    startActivity(i);
*/
                    String topic = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                    ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );

                    boolean shareMode = false;
                    String message = oTopic.message;
                    if( oTopic.isARecordPresent() == false ) { message = getResources().getString( R.string.sharelist_nomessage ); }
                    else if( oTopic.message.length() == 0 ) { message = getResources().getString( R.string.sharelist_needmorekeys ); }
                    else { shareMode = true; }

                    if( shareMode ) {
                        Notifier.ShowMessageAndPosiblyShare(thisActivity, topic, message);
                    } else {
                        Notifier.ShowMessage(thisActivity, message);
                    }
                }
            }
        );
    }

    @Override
    public void onResume() {
        super.onResume();

        refreshListView();
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
                refreshListView();
            }
        };
        Notifier.ShowOptions( this, R.array.dialog_sorttopics, ocl);
    }

    public void onClickPurge(View v) {
        ArtemisSQL.Get().delAll();
        refreshListView();
    }

    public void onClickHack(View v) {
        if( fs == null) {
            fs = new FakeScanner();
        }
        addScannedItem( fs.nextItem() );
    }

    public void onClickNew(View view) {
        startActivity( new Intent(getApplicationContext(), ActivityNew.class) );
    }

    public void onClickScan(View v) {
        // http://stackoverflow.com/questions/3103196/android-os-build-data-examples-please
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        try {
            Intent intent = new Intent("com.google.zxing.client.android.SCAN");
            intent.putExtra("SCAN_MODE", "QR_CODE_MODE");
            startActivityForResult( intent, ACTIVITY_COMPLETE);
        } catch (Exception e1) {
            Notifier.ShowOk( this, R.string.dialog_noscanner);
        }
    }

    //////////////////

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        switch( requestCode ) {
            case ACTIVITY_COMPLETE:
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

}
