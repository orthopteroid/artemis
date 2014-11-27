package com.tereslogica.arcanacard;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.IntentFilter;
import android.database.Cursor;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.FragmentActivity;
import android.support.v4.content.LocalBroadcastManager;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

// http://www.androidhive.info/2011/10/android-listview-tutorial/
public class ActivityTopics extends FragmentActivity {

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
            //Notifier.ShowMessageOk(thisActivity, "sharing!" );
            new Packager( thisActivity, intent.getStringExtra( Const.EXTRA_TOPICSTRING ) );
        }
    };

    private BroadcastReceiver packageIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            Uri uri = Uri.parse( intent.getStringExtra( Const.EXTRA_URISTRING ) );
            //Notifier.ShowMessageOk(thisActivity, "packaged uri: " + uri.toString() );
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
            ArtemisSQL.Get().delTopic(intent.getStringExtra(Const.EXTRA_TOPICSTRING));
            refreshListView();
        }
    };

    private BroadcastReceiver deleteallIntentReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            ArtemisSQL.Get().delAll();
            ((TopicArrayAdapter) listView.getAdapter()).clear();
            refreshListView();
        }
    };

    ////////////////////////////////////

    private class TopicArrayLoader extends AsyncTask<Cursor, Void, Long> {
        Handler uiThreadPoller;

        // http://developer.android.com/reference/java/util/concurrent/package-summary.html
        private CopyOnWriteArrayList<ArtemisTopic> workingArray = new CopyOnWriteArrayList<ArtemisTopic>();

        private void pump() {
            if( workingArray.size() > 0 ) {
                while (workingArray.size() > 0) {
                    topicArrayList.add(workingArray.remove(0));
                }
                ((TopicArrayAdapter) listView.getAdapter()).notifyDataSetChanged();
                listView.smoothScrollToPosition(listView.getCount());
            }
        }

        @Override
        protected void onPreExecute() {
            ((TopicArrayAdapter) listView.getAdapter()).clear();
            topicArrayList.clear();
            workingArray.clear();

            // http://stackoverflow.com/questions/1921514/how-to-run-a-runnable-thread-in-android
            uiThreadPoller = new Handler();
            final Runnable r = new Runnable() // synchronous to ui thread
            {
                public void run()
                {
                    if( uiThreadPoller != null ) {
                        pump();
                        uiThreadPoller.postDelayed(this, 200);
                    }
                }
            };
            uiThreadPoller.postDelayed(r, 200);
        }

        @Override
        protected Long doInBackground(Cursor... cursors) {
            if (cursors == null) return null;
            if (cursors.length != 1) return null;
            if (cursors[0] == null) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                try {
                    do {
                        workingArray.add( (new ArtemisTopic( cursor )).buildDetails() );
                    } while (cursor.moveToNext());
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            uiThreadPoller = null; // stop polling
            pump();
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
                ArtemisTopic.ConfigureTags(convertView);
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
        int rc = AppLogic.Get().addToken( share );

        AppLogic.Get().provideUserFeedback( rc, share );
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

        lbm.registerReceiver(shareIntentReceiver, new IntentFilter( Const.INTENT_SHARE ));
        lbm.registerReceiver(packageIntentReceiver, new IntentFilter( Const.INTENT_PACKAGE ));
        lbm.registerReceiver(deleteoneIntentReceiver, new IntentFilter( Const.INTENT_DELETEONE ));
        lbm.registerReceiver(deleteallIntentReceiver, new IntentFilter( Const.INTENT_DELETEALL ));

        Prefs.Init( this );
        ArtemisSQL.Init( this );
        ArtemisLib.Init();
        AppLogic.Init( this );

        ////////////

        sortOrder = ArtemisSQL.SortOrder.values()[ Prefs.GetInt( Prefs.TOPIC_SORTORDER, ArtemisSQL.SortOrder.NATURAL.ordinal() ) ];

        Prefs.GetInt( Prefs.QR_WIDTH, 300 );
        Prefs.GetInt( Prefs.QR_HEIGHT, 300 );

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
                                intent = new Intent(getApplicationContext(), ActivityQRViewer.class);
                                intent.putExtra( Const.EXTRA_TOPICSTRING, topic );
                                startActivity( intent );
                                intent = null;
                                break;
                            case 1:
                                intent = new Intent( Const.INTENT_SHARE );
                                intent.putExtra( Const.EXTRA_TOPICSTRING, topic );
                                break;
                            case 2:
                                // blank!
                                break;
                            case 3:
                                intent = new Intent( Const.INTENT_DELETEONE );
                                intent.putExtra( Const.EXTRA_TOPICSTRING, topic );
                                break;
                            case 4:
                                intent = new Intent( Const.INTENT_DELETEALL );
                                break;
                            default:
                        }
                        if( intent != null ) { lbm.sendBroadcast( intent ); }
                    }
                };
                Notifier.ShowMenu(thisActivity, R.array.dialog_topicactions, ocl);

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

                    Notifier.ShowMessageOk(thisActivity, topic, message, null);
                }
            }
        );

        //////////////////
        // show startup dialogs

        if( false == Prefs.GetBool( Prefs.EULA_VERSION, false ) ) {
            Notifier.ShowOk_AppVersionTitle(thisActivity, R.string.text_eula, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int _which) {
                    Prefs.SetBool(Prefs.EULA_VERSION, true);

                    // then show freeware warning
                    Notifier.ShowOk( thisActivity, R.string.text_features_limitedversion, null );
                }
            });
        }

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
                Prefs.SetInt( Prefs.TOPIC_SORTORDER, sortOrder.ordinal() );
                refreshListView();
            }
        };

        int selectedid = 0;
        switch( sortOrder ) {
            case UNNATURAL: selectedid=0; break;
            case NATURAL: selectedid=0; break;
            case MOSTRECENT: selectedid=0; break;
            case LEASTRECENT: selectedid=1; break;
            case MOSTCOMPLETE: selectedid=2; break;
            case LEASTCOMPLETE: selectedid=3; break;
            default: selectedid=0; break;
        }

        Notifier.ShowOptions(this, R.string.dialog_selectsorting, R.array.dialog_sorttopics, selectedid, ocl);
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
            startActivityForResult( intent, Const.ACTIVITY_COMPLETE);
        } catch (Exception e1) {
            Notifier.ShowOk( this, R.string.dialog_noscanner, null );
        }
    }

    ////////////////
    // menu creation & handling

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.main_menu, menu);
        if( BuildConfig.DEBUG == false ) {
            menu.removeItem( R.id.menu_testtoken );
        }
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        // Handle item selection
        switch (item.getItemId()) {
            case R.id.menu_testtoken:
                if( fs == null) {
                    fs = new FakeScanner();
                }
                AppLogic.Get().addToken( fs.nextItem() );
                refreshListView();
                return true;
            case R.id.menu_sort:
                onClickSort(null);
                return true;
            case R.id.menu_licenses:
                Notifier.ShowText(thisActivity, R.string.text_licenses);
                return true;
            case R.id.menu_website:
                startActivity( new Intent(Intent.ACTION_VIEW).setData(Uri.parse(this.getResources().getString(R.string.app_weblink))) );
                return true;
            default:
                return super.onOptionsItemSelected(item);
        }
    }

    //////////////////

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        switch( requestCode ) {
            case Const.ACTIVITY_COMPLETE:
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
