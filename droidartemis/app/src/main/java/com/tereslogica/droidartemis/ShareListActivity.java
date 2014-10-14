package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Context;
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
    private String message;

    private ArtemisSQL artemisSql;
    private Notifier notifier;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.NATURAL;

    private ArrayList<ArtemisShare> shareArrayList = new ArrayList<ArtemisShare>();
    private ShareArrayAdapter saa;

    ////////////////////////////////////

    private class ShareArrayLoader extends AsyncTask<Cursor,Void,Long> {

        private String secretShare = "";
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
                        secretShare = oShare.share;
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
            if( secretShare.length() == 0 ) { message = getResources().getString( R.string.sharelist_messagedecodeerror ); }
            ((TextView) findViewById( R.id.message ) ).setText( message );

            String secret = secretShare;
            if( secretShare.length() == 0 ) { secret = getResources().getString( R.string.sharelist_nosecret ); }
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

        ////////////////
/*
        AdapterView.OnItemClickListener oicl = new AdapterView.OnItemClickListener() {
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                String product = ((TextView) view.findViewById( R.id.loctopic ) ).getText().toString();
                Intent i = new Intent(getApplicationContext(), ShareListActivity.class);
                i.putExtra("product", product);
                startActivity(i);
            }
        };
*/
        saa = new ShareArrayAdapter( getApplicationContext() );
        ListView lv = (ListView) findViewById( R.id.key_list );
        lv.setAdapter(saa);
//        lv.setOnItemClickListener(oicl);

        ////////////////

        refreshListView();
    }

    ///////////////////////////

    public void refreshListView() {
        Cursor cursor = artemisSql.getShareTopicCursor( topic, sortOrder );
        if( cursor != null ) {
            (new ShareArrayLoader()).execute(cursor);
        }
    }
/*
    public void onClickSort(View v) {
        DialogInterface.OnClickListener ocl = new DialogInterface.OnClickListener() {
            public void onClick(DialogInterface dialog, int _which) {
                ArtemisSQL.SortOrder orderings[] = {
                        ArtemisSQL.SortOrder.NATURAL,
                        ArtemisSQL.SortOrder.MOSTRECENT,
                        ArtemisSQL.SortOrder.LEASTRECENT,
                };
                sortOrder = orderings[_which];
                refreshListView();
            }
        };
        notifier.showOptions( R.array.dialog_sortshares, ocl);
    }
*/

    ///////////////////////////
}
