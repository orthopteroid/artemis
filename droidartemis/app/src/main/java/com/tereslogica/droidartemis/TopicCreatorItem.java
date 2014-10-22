package com.tereslogica.droidartemis;

import android.database.Cursor;
import android.text.InputType;
import android.view.View;
import android.widget.EditText;
import android.widget.TextView;

/**
 * Created by john on 21/10/14.
 */
public class TopicCreatorItem {

    public String title;
    public String value;
    public int index;

    public TopicCreatorItem() {
        index = -1;
    }

    public TopicCreatorItem( String _t, String _v, int _i ) {
        title = _t;
        value = _v;
        index = _i;
    }

    ///////////

    public static void configureTags( View rowView ) {
        rowView.setTag( R.id.setting_title, ((TextView) rowView.findViewById( R.id.setting_title )) );
        rowView.setTag( R.id.setting_value, ((EditText) rowView.findViewById( R.id.setting_value )) );
    }

    public void configureView( View listItem ) {
        ((TextView) listItem.getTag( R.id.setting_title )).setText( title );
        ((EditText) listItem.getTag( R.id.setting_value )).setText( value );
        if( index <= 1 ) {
            ((EditText) listItem.getTag(R.id.setting_value)).setInputType(InputType.TYPE_CLASS_NUMBER);
        } else {
            ((EditText) listItem.getTag(R.id.setting_value)).setInputType( InputType.TYPE_CLASS_TEXT );
        }
    }

}
