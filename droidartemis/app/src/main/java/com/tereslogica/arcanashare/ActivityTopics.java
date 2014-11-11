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

    public final static String INTENT_SHARE = "intent-share";
    public final static String INTENT_DELETEONE = "intent-deleteone";
    public final static String INTENT_DELETEALL = "intent-deleteall";
    public final static String INTENT_PACKAGE = "intent-package";
    public final static String EXTRA_TOPICSTRING = "intent-topic";
    public final static String EXTRA_URISTRING = "intent-uristring";

    ////////////////

    private FragmentActivity thisActivity;
    private FakeScanner fs;
    private LocalBroadcastManager lbm;
    private ListView listView;
    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;
    private ArrayList<ArtemisTopic> topicArrayList = new ArrayList<ArtemisTopic>();

    ////////////////

    private BroadcastReceiver shareIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            //Notifier.ShowMessage(thisActivity, "sharing!" );
            new Packager( thisActivity, intent.getStringExtra( EXTRA_TOPICSTRING ) );
        }
    };

    private BroadcastReceiver packageIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Uri uri = Uri.parse( intent.getStringExtra( EXTRA_URISTRING ) );
            //Notifier.ShowMessage(thisActivity, "packaged uri: " + uri.toString() );
            Intent intent2 = new Intent( android.content.Intent.ACTION_SEND, uri );
            intent2.setType("application/zip");
            intent2.addFlags( Intent.FLAG_GRANT_READ_URI_PERMISSION );
            intent2.putExtra(Intent.EXTRA_STREAM, uri);
            startActivity(Intent.createChooser(intent2, "Send Images"));
        }
    };

    private BroadcastReceiver deleteoneIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            ArtemisSQL.Get().delTopic(intent.getStringExtra(EXTRA_TOPICSTRING));
            refreshListView();
        }
    };

    private BroadcastReceiver deleteallIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            ArtemisSQL.Get().delAll();
            refreshListView();
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
            super(cxt, R.layout.topic_item, topicArrayList);
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

    //////////////////////
    // http://developer.android.com/reference/android/app/Activity.html#ActivityLifecycle

    @Override
    protected void onDestroy() {
        lbm.unregisterReceiver(shareIntentReceiver);
        lbm.unregisterReceiver(packageIntentReceiver);
        lbm.unregisterReceiver(deleteoneIntentReceiver);
        lbm.unregisterReceiver(deleteallIntentReceiver);

        super.onDestroy();
    }

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.topic_page);
        thisActivity = this;

        lbm = LocalBroadcastManager.getInstance( thisActivity );

        lbm.registerReceiver(shareIntentReceiver, new IntentFilter( INTENT_SHARE ));
        lbm.registerReceiver(packageIntentReceiver, new IntentFilter( INTENT_PACKAGE ));
        lbm.registerReceiver(deleteoneIntentReceiver, new IntentFilter( INTENT_DELETEONE ));
        lbm.registerReceiver(deleteallIntentReceiver, new IntentFilter( INTENT_DELETEALL ));

        ArtemisSQL.Init( this );
        ArtemisLib.Init();
        AppLogic.Init( this );

        ////////////////

        listView = (ListView) findViewById(R.id.topic_list);
        listView.setAdapter( new TopicArrayAdapter(getApplicationContext()) );

        AdapterView.OnItemLongClickListener oilcl = new AdapterView.OnItemLongClickListener() {
            public boolean onItemLongClick(AdapterView<?> parent, View view, int position, long id) {
                final String topic = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );

                DialogInterface.OnClickListener ocl = new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int _which) {
                        Intent intent = null;
                        switch( _which ) {
                            case 0:
                                intent = new Intent( INTENT_SHARE );
                                intent.putExtra( EXTRA_TOPICSTRING, topic );
                                break;
                            case 1:
                                // blank!
                                break;
                            case 2:
                                intent = new Intent( INTENT_DELETEONE );
                                intent.putExtra( EXTRA_TOPICSTRING, topic );
                                break;
                            case 3:
                                intent = new Intent( INTENT_DELETEALL );
                                break;
                            default:
                        }
                        if( intent != null ) { lbm.sendBroadcast( intent ); }
                        refreshListView();
                    }
                };
                Notifier.ShowOptions( thisActivity, R.array.dialog_topicactions, ocl );

                return false;
            }
        };
        listView.setOnItemLongClickListener( oilcl );

        listView.setOnItemClickListener(
            new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    String topic = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                    ArtemisTopic oTopic = ArtemisSQL.Get().getTopicInfo( topic );

                    String message = oTopic.cleartext; // default, assuming it exists
                    if( oTopic.isURIAPresent() == false ) {
                        message = getResources().getString( R.string.sharelist_nomessage );
                    } else if( oTopic.cleartext.length() == 0 ) {
                        message = getResources().getString( R.string.sharelist_needmorekeys );
                    }

                    Notifier.ShowMessage(thisActivity, message);
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

    public void onClickTest(View v) {
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
