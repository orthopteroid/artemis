package com.tereslogica.arcanacard;

import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

import java.util.ArrayList;

/**
 * Created by john on 19/10/14.
 */
public class ActivityNew extends Activity {

    private Activity thisActivity;

    private ArrayList<AbstractItem> settings = new ArrayList<AbstractItem>();

    public enum NumType { key, lock };

    public interface AbstractItem {
        public void setTitle( String t );
        public void setValue( String v );
        public String getValue();
        public void removeFromLayout( LinearLayout layout );
    }

    ///////////////////////////////

    public class NumberItem implements AbstractItem {
        public NumType numType;
        public View containerView;
        public TextView titleView;
        public EditText valueView;

        public NumberItem( LinearLayout layout, LayoutInflater inflater, String _t, String _v, NumType _nt ) {
            numType = _nt;

            containerView = inflater.inflate( R.layout.creator_number, layout, false );
            titleView = ((TextView) containerView.findViewById(R.id.numsetting_title));
            valueView = ((EditText) containerView.findViewById( R.id.numsetting_value));

            // configure view-holder pattern

            containerView.setTag( R.id.numsetting_title, titleView );
            containerView.setTag( R.id.numsetting_value, valueView );

            Button moreButton = ((Button) containerView.findViewById( R.id.button_more ));
            Button lessButton = ((Button) containerView.findViewById( R.id.button_less ));

            containerView.setTag(R.id.button_more, moreButton);
            containerView.setTag(R.id.button_less, lessButton);

            // stash the value-cachedRowView in the button-views, so buttons can update the value-cachedRowView
            moreButton.setTag( R.id.numsetting_value, valueView );
            lessButton.setTag( R.id.numsetting_value, valueView );

            // init properties

            //valueView.setInputType( InputType.TYPE_CLASS_NUMBER );
            valueView.addTextChangedListener(
                    new TextWatcher() {
                        public void afterTextChanged(Editable s) {
                            try {
                                if( Integer.parseInt( s.toString() ) < 2 ) { setValue( "2" ); }
                            } catch( Exception e ) {}
                        }
                        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                        public void onTextChanged(CharSequence s, int start, int before, int count) {}
                    }
            );

            // weird hack
            final LinearLayout layoutClone = layout;
            final LayoutInflater inflaterClone = inflater;

            moreButton.setOnClickListener( new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    try {
                        String value = getValue();
                        setValue(String.valueOf(Integer.parseInt(value) + 1));
                        if( numType == NumType.key ) {
                            settings.add((AbstractItem) new TextItem(layoutClone, inflaterClone, "Optional Key Clue", ""));
                        }
                    } catch (Exception e) {}
                }
            });

            lessButton.setOnClickListener( new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    try {
                        String value = getValue();
                        int delta =  ( Integer.parseInt( value ) < 2 ) ? 0 : -1;
                        setValue( String.valueOf( Integer.parseInt( value ) + delta ) );
                        if( numType == NumType.key ) {
                            if (delta == -1) {
                                int lastItem = settings.size() - 1; // -1 to remove last
                                AbstractItem lastClue = settings.get(lastItem);
                                lastClue.removeFromLayout(layoutClone);
                                settings.remove(lastItem);
                            }
                        }
                    } catch( Exception e ) {}
                }
            });

            // set values

            setTitle(_t);
            setValue(_v);
            layout.addView( containerView );
        }

        public void removeFromLayout( LinearLayout layout ) { layout.removeView( containerView ); }
        public void setTitle( String t ) { titleView.setText( t.subSequence(0, t.length()) ); }
        public void setValue( String v ) { valueView.setText( v.subSequence(0, v.length()) ); }
        public String getValue() { return valueView.getText().toString(); }
    }

    public class TextItem implements AbstractItem {
        public View containerView;
        public EditText textView;

        public TextItem( LinearLayout layout, LayoutInflater inflater, String _t, String _v ) {
            containerView = inflater.inflate( R.layout.creator_text, layout, false );
            textView = ((EditText) containerView.findViewById(R.id.txtsetting_value));

            // configure view-holder pattern

            containerView.setTag(R.id.txtsetting_value, textView);

            // init properties

            textView.addTextChangedListener(
                    new TextWatcher() {
                        public void afterTextChanged(Editable s) {
                            if( s.length() > BuildConfig.MAX_CHARS ) {
                                Notifier.ShowOk( thisActivity, R.string.text_features_limitedversion, null );
                                setValue(s.toString().substring(0, BuildConfig.MAX_CHARS -1));
                            }
                            for (int i = 0; i < s.length(); i++) {
                                if (s.charAt(i) == '\n') {
                                    Notifier.ShowOk( thisActivity, R.string.dialog_nomultiline, null );
                                    break;
                                }
                            }
                        }

                        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                        public void onTextChanged(CharSequence s, int start, int before, int count) {}
                    }
            );

            // set values

            setTitle(_t);
            setValue(_v);
            layout.addView( containerView );
        }

        public void removeFromLayout( LinearLayout layout ) { layout.removeView( containerView ); }
        public void setTitle( String t ) { textView.setHint( t.subSequence(0, t.length()) ); }
        public void setValue( String v ) { textView.setText( v.subSequence(0, v.length()) ); }
        public String getValue() { return textView.getText().toString(); }
    }

    ///////////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.creator_page);
        LinearLayout layout = (LinearLayout) findViewById(R.id.generic_linear);
        LayoutInflater inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        thisActivity = this;

        settings.clear();
        settings.add((AbstractItem) new NumberItem(layout, inflater, "Keys", "7", NumType.key));
        settings.add( (AbstractItem)new NumberItem( layout, inflater, "Locks", "5", NumType.lock ) );
        settings.add((AbstractItem) new TextItem(layout, inflater, "Hidden Message", ""));
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Message Clue", "") );
        settings.add((AbstractItem) new TextItem(layout, inflater, "Optional Key Clue", ""));
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );
        settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", "") );

        if( false == Prefs.GetAndSetBool( Prefs.HOWTO_NEW ) ) {
            Notifier.ShowHowto( thisActivity, R.string.text_howto_new, null );
        }
    }

    ////////
    // http://stackoverflow.com/questions/5123407/losing-data-when-rotate-screen

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        savedInstanceState.putString( "keys", settings.get(0).getValue() );
        savedInstanceState.putString( "locks", settings.get(1).getValue() );
        savedInstanceState.putString( "mtext", settings.get(2).getValue() );
        savedInstanceState.putString( "mclue", settings.get(3).getValue() );
        savedInstanceState.putString( "kc1", settings.get(4).getValue() );
        savedInstanceState.putString( "kc2", settings.get(5).getValue() );

        int extraclues = settings.size() -6; // -6 since array starts at 0
        savedInstanceState.putInt( "extraclues", extraclues );
        for( int i = 0; i < extraclues; i++ ) {
            savedInstanceState.putString( Integer.toString(i), settings.get(i +6).getValue() );
        }
        super.onSaveInstanceState(savedInstanceState);
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
        super.onRestoreInstanceState(savedInstanceState);
        settings.get(0).setValue(savedInstanceState.getString("keys"));
        settings.get(1).setValue(savedInstanceState.getString("locks"));
        settings.get(2).setValue(savedInstanceState.getString("mtext"));
        settings.get(3).setValue( savedInstanceState.getString("mclue") );
        settings.get(4).setValue( savedInstanceState.getString("kc1") );
        settings.get(5).setValue( savedInstanceState.getString("kc2") );

        LinearLayout layout = (LinearLayout) findViewById(R.id.generic_linear);
        LayoutInflater inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        int extraclues = savedInstanceState.getInt("extraclues");
        for( int i = 0; i < extraclues; i++ ) {
            String clueText = savedInstanceState.getString( Integer.toString(i) );
            settings.add( (AbstractItem)new TextItem( layout, inflater, "Optional Key Clue", clueText) );
        }
    }

    ////////////////
    // menu creation & handling

    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.new_menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        FakeUserInput.Scenario s = null;

        int keys = Integer.parseInt(settings.get(0).getValue());

        switch (item.getItemId()) {
            case R.id.menu_new_clearfill:
                for (int i = 2; i < settings.size(); i++) { // 2 >= are user text
                    settings.get(i).setValue( "" );
                }
                return true;
            case R.id.menu_new_autofill_shakespeare:
                s = FakeUserInput.Shakespeare(BuildConfig.MAX_CHARS, keys);
                break;
            case R.id.menu_new_autofill_julesverneEN:
                s = FakeUserInput.JulesVerneEN(BuildConfig.MAX_CHARS, keys);
                break;
            case R.id.menu_new_autofill_julesverneFR:
                s = FakeUserInput.JulesVerneFR(BuildConfig.MAX_CHARS, keys);
                break;
            default:
                break;
        }
        if( s != null ) {
            settings.get(2).setValue( s.m );
            settings.get(3).setValue( s.l );
            for (int i = 4; i < settings.size(); i++) { // 4 >= are key clues
                settings.get(i).setValue( s.k[i -4] ); // -4 to index from 0
            }
            return true;
        } else {
            return super.onOptionsItemSelected(item);
        }
    }

    ////////

    public void onClickAccept(View view) {
        int keys = Integer.parseInt(settings.get(0).getValue());
        int locks = Integer.parseInt(settings.get(1).getValue());

        if( keys > BuildConfig.MAX_KEYS || locks > BuildConfig.MAX_LOCKS ) {
            Notifier.ShowOk(this, R.string.text_features_limitedversion, null);
            return;
        }

        for (int i = 2; i < settings.size(); i++) { // 2 >= are user text
            if (settings.get(i).getValue().indexOf('\n') >= 0) {
                Notifier.ShowOk(this, R.string.dialog_nomultiline, null);
                return;
            }
        }

        ////////////////////////

        StringBuilder clues = new StringBuilder(1024);
        clues.append(settings.get(3).getValue()); // message clue
        {
            for (int i = 4; i < settings.size(); i++) { // 4 >= are the key clues
                clues.append('\n');
                clues.append(settings.get(i).getValue());
            }
        }
        String uri = thisActivity.getApplicationContext().getResources().getString(R.string.app_loc);
        String message = settings.get(2).getValue();
        String shares = ArtemisLib.Get().nativeEncode(keys, locks, uri, clues.toString(), message);

        if (ArtemisLib.Get().nativeDidFail()) {
            Notifier.ShowOk(this, R.string.dialog_err_encode, null);
            return;
        }

        ////////////////////////

        String[] tokenArr = shares.split("\n");
        int rc = AppLogic.Get().addTokenArray( tokenArr );

        if( AppLogic.RCTest( rc, AppLogic.RC_ERROR ) ) {

            Notifier.ShowOk(this, R.string.dialog_err_decode, null);

        } else if( AppLogic.RCTest( rc, AppLogic.RC_UNLOCKED | AppLogic.RC_MESSAGE ) ) {

            final Activity newActivity = this;
            final String topic = ArtemisLib.Get().nativeTopic( tokenArr[0] );

            Notifier.ShowMessageOk(thisActivity, R.string.dialog_newtopic, topic, new DialogInterface.OnClickListener() {
                public void onClick(DialogInterface dialog, int _which) {
                    newActivity.finish();
                }
            });

        } else {
            this.finish(); // an odd codepath, but required to close activity when the app has been hacked...
        }
    }

}
