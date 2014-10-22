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

    public static final int ACTIVITY_COMPLETE = 0;

    ////////////////

    private static final boolean forceFakeScanner = true;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;

    private FakeScanner fs;

    private ArrayList<ArtemisTopic> topicArrayList = new ArrayList<ArtemisTopic>();
    private TopicArrayAdapter taa;

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
            taa.notifyDataSetChanged();
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class TopicArrayAdapter extends ArrayAdapter<ArtemisTopic> {
        LayoutInflater inflater;

        public TopicArrayAdapter(Context cxt) {
            super(cxt, R.layout.topic_item, R.id.topic, topicArrayList);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if (convertView == null) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.topic_item, parent, false);
                ArtemisTopic.configureTags(rowView);
            } else {
                rowView = convertView;
            }
            topicArrayList.get(position).configureView(rowView);
            return rowView;
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
        AppLogic.Get().addItem( share );
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

        Notifier.Init( this );
        ArtemisSQL.Init( this );
        ArtemisLib.Init();
        AppLogic.Init();

        ////////////////

        OnItemClickListener oicl = new OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                Intent i = new Intent(getApplicationContext(), ShareListActivity.class);
                i.putExtra("topic", ((TextView) view.findViewById(R.id.topic)).getText().toString());
                startActivity(i);
            }
        };

        taa = new TopicArrayAdapter(getApplicationContext());
        ListView lv = (ListView) findViewById(R.id.topic_list);
        lv.setAdapter(taa);
        lv.setOnItemClickListener(oicl);

        ////////////////

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
        Notifier.Get().showOptions(R.array.dialog_sortshares, ocl);
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
        Intent i = new Intent(getApplicationContext(), TopicCreatorActivity.class);
        startActivity(i);
    }

    public void onClickScan(View v) {
        // http://stackoverflow.com/questions/3103196/android-os-build-data-examples-please
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        try {
            Intent intent = new Intent("com.google.zxing.client.android.SCAN");
            intent.putExtra("SCAN_MODE", "QR_CODE_MODE");
            startActivityForResult( intent, ACTIVITY_COMPLETE);
        } catch (Exception e1) {
            Notifier.Get().showOk(R.string.dialog_noscanner);
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
