package com.tereslogica.droidartemis;

import android.content.Context;
import android.content.Intent;
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

    static ArtemisLib artemisLib;
    static ArtemisSQL artemisSql;

    Notifier notifier;
    FakeScanner fs;

    ArrayList<TLAItem> al = new ArrayList<TLAItem>();
    //TopicListAdapter tla;
    TLAAdapter tla;

    ////////////////////////////////////

    public class TLAItem {

        public String topic;
        public String details;

        public TLAItem(String _topic, String _details) {
            super();
            this.topic = _topic;
            this.details = _details;
        }
    }

    ///////////

    public class TLALoader extends AsyncTask<String,Void,Long> {
        private ArrayAdapter<TLAItem> adapter;
        private ArrayList<TLAItem> arraylist;
        private android.content.res.Resources resources;

        TLALoader(android.content.res.Resources res, ArrayAdapter<TLAItem> aa, ArrayList<TLAItem> arrl) {
            resources = res;
            adapter = aa;
            arraylist = arrl;
        }

        @Override
        protected void onPreExecute() { }

        @Override
        protected Long doInBackground(String ... parms) {
            int count = parms.length;
            if( count == 1 ) {
                if( parms[0].length() == 0 ) {
                    String[] adobe_products = resources.getStringArray(R.array.adobe_products);
                    arraylist.clear();
                    for (String s : adobe_products) {
                        arraylist.add(new TLAItem(s, "descr"));
                        if( isCancelled() ) { break; }
                        publishProgress();
                    }
                } else {
                    arraylist.add(new TLAItem(parms[0], "descr"));
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(Void ... values) {
            // adapter update in bkgrnd thread illegal when using
            // adapter.notifyDataSetChanged();
        }

        @Override
        protected void onPostExecute(Long result) {
            adapter.notifyDataSetChanged();
        }
    }

    /////////

    public class TLAAdapter extends ArrayAdapter<TLAItem> {
        public TLAAdapter() {
            super( getApplicationContext(), R.layout.list_item, R.id.topic, al );
        }
        @Override
        public View getView(int position, View convertView, ViewGroup parent) {

            LayoutInflater inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
            View rowView = inflater.inflate( R.layout.list_item, parent, false); // optimize: use view recycling
            TextView labelView = (TextView) rowView.findViewById(R.id.topic);
            TextView valueView = (TextView) rowView.findViewById(R.id.details);
            labelView.setText(al.get(position).topic);
            valueView.setText(al.get(position).details);
            return rowView;
        }
    }

    //////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_my);

        notifier = new Notifier(getApplicationContext(),getResources());

        // init library
        try {
            artemisLib = new ArtemisLib();
        } catch (Exception e1) {
            notifier.show(R.string.notify_nonativelib);
        }

        {
            //tla = new TopicListAdapter( getApplicationContext(), R.layout.list_item, R.id.topic, al );
            tla = new TLAAdapter();
            ListView lv = (ListView) findViewById( R.id.list_container );
            lv.setAdapter(tla);
            lv.setOnItemClickListener( new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    String product = ((TextView) view.findViewById(R.id.topic)).getText().toString();
                    Intent i = new Intent(getApplicationContext(), TopicActivity.class);
                    i.putExtra("product", product);
                    startActivity(i);
                }
            });

            (new TLALoader( getResources(), tla, al)).execute("");
        }
/*
        {
            Button b = (Button) findViewById(R.id.scan_button);
            b.setOnClickListener( new View.OnClickListener() {
                public void onClick(View v) {

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
                            intent.putExtra("SCAN_MODE", "QR_CODE_MODE"); // "PRODUCT_MODE for bar codes
                            startActivityForResult(intent, 0);
                        } catch (Exception e1) {
                            showDialogNoScanner();
                        }

                    }
                }

            });
        }
*/
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        if( requestCode == 0 ) {

            if( resultCode == RESULT_OK ) {
                addScannedItem( data.getStringExtra("SCAN_RESULT") );
            }
            if( resultCode == RESULT_CANCELED ) {
                //handle cancel
            }
        }
    }

    ///////////////////////////

    public void onClickSort(View v) {
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
                startActivityForResult(intent, 0);
            } catch (Exception e1) {
                notifier.show(R.string.notify_noscanner);
            }

        }
    }

    ///////////////////////////

    String zoo = new String();

    public void addScannedItem( String item ) {
        Log.d("libartemis", "add " + item);
        //
        String topic = artemisLib.nativeShareField(item, "tp", 0);
        Log.d("libartemis", "topic " + topic);
        //
        TLAItem itemobj = new TLAItem( topic, item );
        if( !al.contains( itemobj ) ) {
            al.add(itemobj);
            tla.notifyDataSetChanged();
        }
        //
        if( zoo.length() == 0 ) zoo = item; else zoo += "\n" + item;
/*
        String foo = new String();
        sal.add( item );
        int i = 0;
        for( String s : sal ) {
            if( i == 0 ) foo = s; else foo += "\n" + s;
            i++;
        }
*/
        // http://mhandroid.wordpress.com/2011/01/25/how-cc-debugging-works-on-android/
        // ./adb root ; ./adb shell stop ; ./adb shell setprop log.redirect-stdio true ; ./adb shell start
        String bar = artemisLib.nativeDecode(zoo);
        boolean ok = artemisLib.nativeGetStatusOK();
        if( ok == false ) bar = "*NODECODE*";
        Log.d("libartemis", "returned " + bar);
    }
    public void removeScannedItem( String item ) {
        Log.d("libartemis", "remove " + item);
    }
}
