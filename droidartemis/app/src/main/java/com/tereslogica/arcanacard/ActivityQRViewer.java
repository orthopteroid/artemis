package com.tereslogica.acanacard;

import android.content.Context;
import android.database.Cursor;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Handler;
import android.support.v4.app.FragmentActivity;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;

import java.util.ArrayList;
import java.util.concurrent.CopyOnWriteArrayList;

/**
 * Created by john on 20/11/14.
 */
public class ActivityQRViewer extends FragmentActivity {
    private String sTopic;
    private FragmentActivity thisActivity;
    private ListView listView;
    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.MOSTRECENT;
    private ArrayList<ArtemisShare> shareArrayList = new ArrayList<ArtemisShare>();

    ////////////////////////////////////

    private class ShareArrayLoader extends AsyncTask<Cursor, Void, Long> {
        Handler uiThreadPoller;

        // http://developer.android.com/reference/java/util/concurrent/package-summary.html
        private CopyOnWriteArrayList<ArtemisShare> workingArray = new CopyOnWriteArrayList<ArtemisShare>(); // multi-reader safe

        private void pump() {
            if( workingArray.size() > 0 ) {
                while (workingArray.size() > 0) {
                    shareArrayList.add(workingArray.remove(0));
                }
                ((ShareArrayAdapter) listView.getAdapter()).notifyDataSetChanged();
                listView.smoothScrollToPosition(listView.getCount());
            }
        }

        @Override
        protected void onPreExecute() {
            ((ShareArrayAdapter) listView.getAdapter()).clear();
            shareArrayList.clear();
            workingArray.clear();

            // http://stackoverflow.com/questions/1921514/how-to-run-a-runnable-thread-in-android
            uiThreadPoller = new Handler();
            final Runnable r = new Runnable() // synchronous to ui thread
            {
                public void run()
                {
                    if( uiThreadPoller != null ) {
                        pump();
                        uiThreadPoller.postDelayed(this, 500);
                    }
                }
            };
            uiThreadPoller.postDelayed(r, 500);
        }

        @Override
        protected Long doInBackground(Cursor... cursors) {
            if (cursors == null) return null;
            if (cursors.length != 1) return null;
            if (cursors[0] == null) return null;

            int width = Prefs.GetInt( Prefs.QR_WIDTH, Prefs.DEFAULT_WIDTH );
            int height = Prefs.GetInt( Prefs.QR_HEIGHT, Prefs.DEFAULT_HEIGHT );

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                try {
                    do {
                        workingArray.add( (new ArtemisShare( cursor )).buildQRCode() );
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

    private class ShareArrayAdapter extends ArrayAdapter<ArtemisShare> {
        LayoutInflater inflater;

        public ShareArrayAdapter(Context cxt) {
            super(cxt, R.layout.qrviewer_item, shareArrayList);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            if (convertView == null) { // view recycling and view holder pattern
                convertView = inflater.inflate(R.layout.qrviewer_item, parent, false);
                ArtemisShare.ConfigureTags(convertView);
            }
            shareArrayList.get(position).configureView(convertView);
            return convertView;
        }
    }

    /////////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.qrviewer_page);
        thisActivity = this;

        sTopic = getIntent().getStringExtra( Const.EXTRA_TOPICSTRING );

        listView = (ListView) findViewById(R.id.qrview_list);
        listView.setAdapter( new ShareArrayAdapter(getApplicationContext()) );

        Cursor cursor = ArtemisSQL.Get().getShareTopicCursor( sTopic, ArtemisSQL.SortOrder.NATURAL );
        if (cursor != null) {
            (new ShareArrayLoader()).execute(cursor);
        }
    }
}
