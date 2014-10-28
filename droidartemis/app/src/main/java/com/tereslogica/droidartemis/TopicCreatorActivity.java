package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.text.InputType;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.EditText;
import android.widget.ListView;

import java.util.ArrayList;

/**
 * Created by john on 19/10/14.
 */
public class TopicCreatorActivity extends Activity {

    private ArrayList<TopicCreatorItem> settings = new ArrayList<TopicCreatorItem>();
    private SettingsArrayAdapter settingsAdapter;

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class SettingsArrayAdapter extends ArrayAdapter<TopicCreatorItem> {
        LayoutInflater inflater;

        public SettingsArrayAdapter(Context cxt) {
            super(cxt, R.layout.creator_item, /* R.id.setting_title,*/ settings);
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if (convertView == null) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.creator_item, parent, false);
                TopicCreatorItem.configureTags(rowView);
            } else {
                rowView = convertView;
            }
            settings.get(position).configureView(rowView);
            return rowView;
        }
    }

    //////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.creator_page);

        settingsAdapter = new SettingsArrayAdapter(getApplicationContext());
        ListView lv = (ListView) findViewById(R.id.listview_settings);
        lv.setAdapter(settingsAdapter);

        settings.clear();
        settings.add(new TopicCreatorItem("Keys", "2", InputType.TYPE_CLASS_NUMBER));
        settings.add(new TopicCreatorItem("Locks", "2", InputType.TYPE_CLASS_NUMBER));
        settings.add(new TopicCreatorItem("Message to be Encrypted", "A Secret", InputType.TYPE_CLASS_TEXT));
        settings.add(new TopicCreatorItem("Message Clue (Optional, Cleartext)", "mc", InputType.TYPE_CLASS_TEXT));
        settings.add( new TopicCreatorItem("Key Clue (Optional)","c1", InputType.TYPE_CLASS_TEXT) );
        settings.add( new TopicCreatorItem("Key Clue (Optional)","c2", InputType.TYPE_CLASS_TEXT) );
        settingsAdapter.notifyDataSetChanged();
    }

    public void onClickAccept(View view) {
        if( Integer.parseInt( settings.get(0).value ) < 2 || Integer.parseInt( settings.get(0).value ) > 10 ) {
            Notifier.ShowOk( this, R.string.dialog_keycount);
            return;
        }
        if( Integer.parseInt( settings.get(1).value ) < 2 || Integer.parseInt( settings.get(1).value ) > 4 ) {
            Notifier.ShowOk( this, R.string.dialog_lockcount);
            return;
        }
        if( settings.get(2).value.length() > 40 ) {
            Notifier.Get().showOk(R.string.dialog_messagesize);
            return;
        }
        if( settings.get(3).value.length() > 20 ) {
            Notifier.Get().showOk(R.string.dialog_cluesize);
            return;
        }
        {
            for( int i = 4; i < settings.size(); i++) { // 4 >= are the key clues
                if( settings.get(i).value.length() > 20) {
                    Notifier.Get().showOk(R.string.dialog_cluesize);
                    return;
                }
            }
        }

        ////////////////////////

        settings.get(2).value.replace( '\n', ' ' ); // can't hae embedded \n in artemislib api

        StringBuilder clues = new StringBuilder( 1024 );
        clues.append( settings.get(3).value );
        {
            for( int i = 4; i < settings.size(); i++) { // 4 >= are the key clues
                clues.append( '\n' );
                clues.append( settings.get(i).value );
            }
        }
        int keys = Integer.parseInt( settings.get(0).value );
        int locks = Integer.parseInt( settings.get(1).value );
        String shares = ArtemisLib.Get().nativeEncode( keys, locks, "foo.bar", clues.toString(), settings.get(2).value );

        if( ArtemisLib.Get().nativeDidFail() ) {
            Notifier.ShowOk( this, R.string.dialog_err_encode);
            return;
        }

        ////////////////////////

        AppLogic.Get().addTokenArray(shares.split("\n"));

        if( AppLogic.Get().detectedError ) { Notifier.ShowOk( this, R.string.dialog_err_parse ); }
        if( AppLogic.Get().detectedDecode ) { Notifier.ShowOk( this, R.string.dialog_info_decode ); }
        if( AppLogic.Get().detectedDecode ) { this.finish(); }
    }

}
