package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.text.Editable;
import android.text.InputType;
import android.text.TextWatcher;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

/**
 * Created by john on 21/10/14.
 */
public class TopicCreatorItem {

    public String title;
    public String value;
    public int kind;

    private TopicCreatorItem() {}

    public TopicCreatorItem( String _t, String _v, int _k ) {
        title = _t;
        value = _v;
        kind = _k;
    }

    ///////////

    public static void configureTags( View rowView ) {
        rowView.setTag( R.id.setting_title, ((TextView) rowView.findViewById( R.id.setting_title )) );
        rowView.setTag( R.id.setting_value, ((EditText) rowView.findViewById( R.id.setting_value )) );
    }

    public void configureView( View listItem ) {
        ((TextView) listItem.getTag( R.id.setting_title )).setText( title );
        ((EditText) listItem.getTag( R.id.setting_value )).setText( value );
        ((EditText) listItem.getTag( R.id.setting_value )).setFocusable(true);
        ((EditText) listItem.getTag( R.id.setting_value )).setEnabled(true);
        ((EditText) listItem.getTag( R.id.setting_value )).setInputType(kind);
        ((EditText) listItem.getTag( R.id.setting_value )).addTextChangedListener(
            new TextWatcher() {
                public void afterTextChanged(Editable s) {
                    value = s.toString();
                }
                public void beforeTextChanged(CharSequence s, int start, int count, int after) { }
                public void onTextChanged(CharSequence s, int start, int before, int count) { }
            }
        );
    }

}
