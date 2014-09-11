package com.tereslogica.droidartemis;

import android.os.AsyncTask;
import android.widget.ArrayAdapter;

import java.util.ArrayList;

public class HomeListLoader extends AsyncTask<String,Void,Long> {
    private ArrayAdapter<HomeListItem> adapter;
    private ArrayList<HomeListItem> arraylist;
    private android.content.res.Resources resources;

    HomeListLoader(android.content.res.Resources res, ArrayAdapter<HomeListItem> aa, ArrayList<HomeListItem> arrl) {
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
                    arraylist.add(new HomeListItem(s, "descr"));
                    if( isCancelled() ) { break; }
                    publishProgress();
                }
            } else {
                arraylist.add(new HomeListItem(parms[0], "descr"));
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
