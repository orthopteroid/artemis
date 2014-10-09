package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

public class ShareListActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.share_item);

        TextView txtProduct = (TextView) findViewById(R.id.share);

        Intent i = getIntent();
        String product = i.getStringExtra("product");
        txtProduct.setText(product);
    }

}
