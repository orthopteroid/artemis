package com.tereslogica.droidartemis;

import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;
import android.support.v4.app.FragmentActivity;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.AdapterView.OnItemClickListener;
import android.widget.Button;
import android.widget.ListView;
import android.widget.TextView;

import java.util.ArrayList;

// http://www.androidhive.info/2011/10/android-listview-tutorial/
public class MyActivity extends FragmentActivity implements DialogNoScanner.DialogNoScannerListener {

    static {
        // only works with arm?
        System.loadLibrary("myNDKModule");
    }

    public native String nativeGetString();
    public native String nativeDecode( String data );
    public native boolean nativeGetStatusOK();

    ArrayList<String> sal = new ArrayList<String>();

    FakeScanner fs;
    ArrayList<HomeListItem> al = new ArrayList<HomeListItem>();;
    HomeListAdapter hla;
    HomeListLoader hll;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        String sss = nativeGetString();

        setContentView(R.layout.activity_my);

        //////////

        {
            ListView lv = (ListView) findViewById(R.id.list_container);

            //al = new ArrayList<HomeListItem>();
            hla = new HomeListAdapter(getApplicationContext(), R.layout.list_item, R.id.title, al);
            lv.setAdapter(hla);

            hll = new HomeListLoader(getResources(), hla, al);
            hll.execute("");

            // listening to single list item on click
            lv.setOnItemClickListener(new OnItemClickListener() {
                public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                    String product = ((TextView) view.findViewById(R.id.title)).getText().toString();
                    Intent i = new Intent(getApplicationContext(), SingleListItem.class);
                    i.putExtra("product", product);
                    startActivity(i);
                }
            });
        }

        //////////

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

    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == 0) {

            if (resultCode == RESULT_OK) {
                addScannedItem( data.getStringExtra("SCAN_RESULT") );
            }
            if(resultCode == RESULT_CANCELED){
                //handle cancel
            }
        }
    }

    ///////////////////////////

    public void showDialogNoScanner() {
        // Create an instance of the dialog fragment and show it
        DialogFragment dialog = new DialogNoScanner();
        dialog.show(getSupportFragmentManager(), "DialogNoScanner");
    }

    @Override
    public void onDialogNoScanner_Click(DialogFragment dialog) {
        // User touched the dialog's button
    }

    ///////////////////////////

    String zoo = new String();

    public void addScannedItem( String item ) {
        Log.d("libartemis", "add " + item);
        al.add( new HomeListItem( item, "descr" ) );
        hla.notifyDataSetChanged();
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
        String bar = nativeDecode( zoo );
        boolean ok = nativeGetStatusOK();
        if( ok == false ) bar = "*NODECODE*";
        Log.d("libartemis", "returned " + bar);
    }
    public void removeScannedItem( String item ) {
        Log.d("libartemis", "remove " + item);
    }
}
