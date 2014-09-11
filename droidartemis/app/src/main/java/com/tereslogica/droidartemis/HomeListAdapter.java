package com.tereslogica.droidartemis;

import java.util.ArrayList;

import android.content.Context;
import android.support.annotation.LayoutRes;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.TextView;

public class HomeListAdapter extends ArrayAdapter<HomeListItem> {

    private @LayoutRes int resource;
    private int textViewResourceId;
    private final Context context;
    private final ArrayList<HomeListItem> itemsArrayList;

    public HomeListAdapter(Context _context, @LayoutRes int _resource, int _textViewResourceId, ArrayList<HomeListItem> _itemsArrayList) {
        super( _context, _resource, _textViewResourceId, _itemsArrayList );

        this.context = _context;
        this.resource = _resource;
        this.textViewResourceId = _textViewResourceId;
        this.itemsArrayList = _itemsArrayList;
    }

    @Override
    public View getView(int position, View convertView, ViewGroup parent) {

        // 1. Create inflater
        LayoutInflater inflater = (LayoutInflater) context.getSystemService(Context.LAYOUT_INFLATER_SERVICE);

        // 2. Get rowView from inflater
        View rowView = inflater.inflate( resource, parent, false);

        // 3. Get the two text view from the rowView
        TextView labelView = (TextView) rowView.findViewById(R.id.title);
        TextView valueView = (TextView) rowView.findViewById(R.id.description);

        // 4. Set the text for textView
        labelView.setText(itemsArrayList.get(position).title);
        valueView.setText(itemsArrayList.get(position).description);

        // 5. retrn rowView
        return rowView;
    }

}
