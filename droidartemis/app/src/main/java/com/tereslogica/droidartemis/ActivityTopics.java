package com.tereslogica.droidartemis;

import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.os.AsyncTask;
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
public class ActivityTopics extends FragmentActivity {

    public static final int ACTIVITY_COMPLETE = 0;

    ////////////////

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;

    private FakeScanner fs;

    private ArrayList<ArtemisTopic> topicArrayList = new ArrayList<ArtemisTopic>();
    private ListView listView;

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
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.topic_page);

        ArtemisSQL.Init( this );
        ArtemisLib.Init();
        AppLogic.Init();

        ////////////////

        listView = (ListView) findViewById(R.id.topic_list);
        listView.setAdapter(new TopicArrayAdapter(getApplicationContext()));
        listView.setOnItemClickListener(
            new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    Intent i = new Intent(getApplicationContext(), ShareListActivity.class);
                    i.putExtra("topic", ((TextView) view.findViewById(R.id.topic)).getText().toString());
                    startActivity(i);
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
        Notifier.ShowOptions( this, R.array.dialog_sortshares, ocl);
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
