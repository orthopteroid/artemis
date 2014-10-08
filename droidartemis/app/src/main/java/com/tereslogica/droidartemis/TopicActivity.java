package com.tereslogica.droidartemis;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.widget.TextView;

public class TopicActivity extends Activity {

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        this.setContentView(R.layout.share_item);

        TextView txtProduct = (TextView) findViewById(R.id.product_label);

        Intent i = getIntent();
        String product = i.getStringExtra("product");
        txtProduct.setText(product);
    }

}
