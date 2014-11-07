package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Context;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.LayoutInflater;
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

    public interface AbstractItem {
        public void setTitle( String t );
        public void setValue( String v );
        public String getValue();
    }

    ///////////////////////////////

    public class NumberItem implements AbstractItem {
        public View containerView;

        public NumberItem( LayoutInflater inflater, LinearLayout layout, String _t, String _v ) {
            containerView = inflater.inflate( R.layout.creator_number, layout, false );

            // configure

            containerView.setTag(R.id.numsetting_title, ((TextView) containerView.findViewById(R.id.numsetting_title)));

            EditText valueView = ((EditText) containerView.findViewById( R.id.numsetting_value));
            containerView.setTag(R.id.numsetting_value, valueView);

            Button moreButton = ((Button) containerView.findViewById( R.id.button_more ));
            Button lessButton = ((Button) containerView.findViewById( R.id.button_less ));

            containerView.setTag(R.id.button_more, moreButton);
            containerView.setTag(R.id.button_less, lessButton);

            // stash the value-cachedRowView in the button-views, so buttons can update the value-cachedRowView
            moreButton.setTag( R.id.numsetting_value, valueView );
            lessButton.setTag( R.id.numsetting_value, valueView );

            // configureview

            TextView titleView = ((TextView) containerView.getTag(R.id.numsetting_title));
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

            moreButton.setOnClickListener( new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    try {
                        String value = getValue();
                        setValue( String.valueOf( Integer.parseInt( value ) + 1 ) );
                    } catch( Exception e ) {}
                }
            });

            lessButton.setOnClickListener( new View.OnClickListener() {
                @Override
                public void onClick(View view) {
                    try {
                        String value = getValue();
                        setValue( String.valueOf( Integer.parseInt( value ) + ( ( Integer.parseInt( value ) < 2 ) ? 0 : -1 ) ) );
                    } catch( Exception e ) {}
                }
            });

            // setvalues

            setTitle(_t);
            setValue(_v);
            layout.addView( containerView );
        }

        public void setTitle( String t ) { ((TextView) containerView.getTag( R.id.numsetting_title )).setText( t ); }
        public void setValue( String v ) { ((EditText) containerView.getTag( R.id.numsetting_value )).setText( v ); }
        public String getValue() { return ((EditText) containerView.getTag( R.id.numsetting_value )).getText().toString(); }
    }

    public class TextItem implements AbstractItem {
        public View containerView;

        public TextItem( LayoutInflater inflater, LinearLayout layout, String _t, String _v ) {
            containerView = inflater.inflate( R.layout.creator_text, layout, false );

            // configure

            TextView textView = ((TextView) containerView.findViewById(R.id.txtsetting_value));
            containerView.setTag(R.id.txtsetting_value, textView);

            textView.addTextChangedListener(
                    new TextWatcher() {
                        public void afterTextChanged(Editable s) {
                            if (s.length() > 20) {
                                Notifier.ShowOk(thisActivity, R.string.dialog_demofail);
                                setValue( s.toString().substring(0,19) );
                            }
                            for (int i = 0; i < s.length(); i++) {
                                if (s.charAt(i) == '\n') {
                                    Notifier.ShowOk(thisActivity, R.string.dialog_nomultiline);
                                    break;
                                }
                            }
                        }
                        public void beforeTextChanged(CharSequence s, int start, int count, int after) {}
                        public void onTextChanged(CharSequence s, int start, int before, int count) {}
                    }
            );

            // setvalues

            setTitle(_t);
            setValue(_v);
            layout.addView( containerView );
        }

        public void setTitle( String t ) { ((EditText) containerView.getTag( R.id.txtsetting_value )).setHint( t ); }
        public void setValue( String v ) { ((EditText) containerView.getTag( R.id.txtsetting_value )).setText( v ); }
        public String getValue() { return ((EditText) containerView.getTag( R.id.txtsetting_value )).getText().toString(); }
    }

    ///////////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.generic_scrollview);
        LinearLayout layout = (LinearLayout) findViewById(R.id.generic_linear);
        LayoutInflater inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        thisActivity = this;

        settings.clear();
        settings.add( (AbstractItem)new NumberItem( inflater, layout, "Keys", "2") );
        settings.add( (AbstractItem)new NumberItem( inflater, layout, "Locks", "2") );
        settings.add( (AbstractItem)new TextItem( inflater, layout, "Message to be Encrypted", "A Secret") );
        settings.add( (AbstractItem)new TextItem( inflater, layout, "Optional Message Clue", "mc") );
        settings.add( (AbstractItem)new TextItem( inflater, layout, "Optional Key Clue", "c1") );
        settings.add( (AbstractItem)new TextItem( inflater, layout, "Optional Key Clue", "c2") );
    }

    ////////
    // http://stackoverflow.com/questions/5123407/losing-data-when-rotate-screen

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
/*
        savedInstanceState.putString("keys", settings.get(0).value);
        savedInstanceState.putString("locks", settings.get(1).value);
        savedInstanceState.putString("message", settings.get(2).value);
        super.onSaveInstanceState(savedInstanceState);
*/
    }

    @Override
    public void onRestoreInstanceState(Bundle savedInstanceState) {
/*
        super.onRestoreInstanceState(savedInstanceState);
        settings.get(0).value = savedInstanceState.getString("keys");
        settings.get(1).value = savedInstanceState.getString("locks");
        settings.get(2).value = savedInstanceState.getString("message");
*/
    }

    ////////

    public void onClickAccept(View view) {
        for( int i = 2; i < settings.size(); i++) { // 2 >= are user text
            if( settings.get(i).getValue().indexOf('\n') >= 0) {
                Notifier.ShowOk( this, R.string.dialog_nomultiline);
                return;
            }
        }

        ////////////////////////

        StringBuilder clues = new StringBuilder( 1024 );
        clues.append( settings.get(3).getValue() );
        {
            for( int i = 4; i < settings.size(); i++) { // 4 >= are the key clues
                clues.append( '\n' );
                clues.append( settings.get(i).getValue() );
            }
        }
        int keys = Integer.parseInt( settings.get(0).getValue() );
        int locks = Integer.parseInt( settings.get(1).getValue() );
        String shares = ArtemisLib.Get().nativeEncode( keys, locks, "foo.bar", clues.toString(), settings.get(2).getValue() );

        if( ArtemisLib.Get().nativeDidFail() ) {
            if( ArtemisLib.Get().nativeWasFailDemo() ) {
                Notifier.ShowOk( this, R.string.dialog_demofail);
                return;
            }
            Notifier.ShowOk( this, R.string.dialog_err_encode);
            return;
        }

        ////////////////////////

        AppLogic.Get().addTokenArray(shares.split("\n"));

        if( AppLogic.Get().detectedError ) { Notifier.ShowOk( this, R.string.dialog_err_parse ); }
        if( AppLogic.Get().detectedDecode ) { this.finish(); }
    }

}
