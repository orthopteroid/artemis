package com.tereslogica.droidartemis;

import android.app.Activity;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.pm.ResolveInfo;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.os.Bundle;
import android.os.Environment;
import android.provider.DocumentsContract;
import android.support.v4.content.FileProvider;
import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.ArrayAdapter;
import android.widget.ListView;
import android.widget.TextView;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.common.BitArray;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.io.PrintStream;
import java.io.UnsupportedEncodingException;
import java.nio.charset.Charset;
import java.nio.charset.StandardCharsets;
import java.util.ArrayList;
import java.util.List;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

public class ShareListActivity extends Activity {
    private String topic;
    private String aRecord = "";
    private String message;

    private ArtemisSQL artemisSql;
    private Notifier notifier;

    private ArtemisSQL.SortOrder sortOrder = ArtemisSQL.SortOrder.NATURAL;

    private ArrayList<ArtemisShare> shareArrayList = new ArrayList<ArtemisShare>();
    private ShareArrayAdapter saa;

    public static final int ACTIVITY_COMPLETE = 0;

    ////////////////////////////////////

    // https://developer.android.com/reference/android/support/v4/content/FileProvider.html
    // http://stackoverflow.com/questions/18249007/how-to-use-support-fileprovider-for-sharing-content-to-other-apps

    private class TopicZipper {
        private File dir;
        private File fout;
        private Uri uri;
        private FileOutputStream fouts;
        private ZipOutputStream zout;
        //
        private int width = 300, height = 300;
        private String topic, sARecord;
        private ArrayList<ArtemisShare> oSRecordList;
        private int key = 0;
        //
        public int filesToConvert;

        TopicZipper( String a, String b, ArrayList<ArtemisShare> c ) {
            topic = a;
            sARecord = b;
            oSRecordList = c;
            //
            filesToConvert = ( sARecord.length() > 0 ? 1 : 0 ) + oSRecordList.size();
        }

        private void addPNG(ZipOutputStream zout, String fname, String code, int width, int height) throws Exception {
            ZipEntry ze = new ZipEntry( fname );
            zout.putNextEntry( ze );

            QRCodeWriter qrcw = new QRCodeWriter();
            BitMatrix bm = qrcw.encode( code, BarcodeFormat.QR_CODE, width, height );

            Bitmap bitmap = Bitmap.createBitmap( width, height, Bitmap.Config.RGB_565 );
            for (int i = 0; i < width; i++) {
                for (int j = 0; j < height; j++) {
                    bitmap.setPixel(i, j, bm.get(i, j) ? Color.BLACK: Color.WHITE);
                }
            }

            ByteArrayOutputStream bos = new ByteArrayOutputStream();
            bitmap.compress(Bitmap.CompressFormat.PNG, 0, bos);

            zout.write( bos.toByteArray() );
            zout.closeEntry();
        }

        private void open() throws Exception {
            dir = new File(getApplicationContext().getCacheDir(), "");
            dir.mkdirs();

            String fname = "topic-" + topic.substring(0, 10) + ".zip";
            fout = new File(dir, fname);

            uri = FileProvider.getUriForFile(getApplicationContext(), "com.tereslogica.droidartemis.ShareListActivity", fout);

            fouts = new FileOutputStream(fout);
            zout = new ZipOutputStream(fouts);
            zout.setMethod(ZipOutputStream.DEFLATED);
            zout.setLevel(1); // for png, use almost no compression. faster.
        }

        // return true if file was added, false if not
        private boolean addMessage() throws Exception {
            if( sARecord.length() > 0 ) {
                addPNG( zout, "message.png", sARecord, width, height );
                return true;
            }
            return false;
        }

        // return true if file was added, false if not (and there are no more files to add)
        private boolean addKey() throws Exception {
            if( key == oSRecordList.size() ) { return false; }
            ArtemisShare sRecord = oSRecordList.get( key );
            String istr = String.format("%04d", key); // 12 bits serialized = 4095 = 4 decimal digits, 0 padding
            addPNG( zout, "key" + istr + ".png", sRecord.share, width, height );
            key++;
            return true;
        }

        private void close() throws Exception {
            zout.close();
            fouts.close();
            fout.deleteOnExit();
        }
    }

    ////////////////////////////////////

    private class TopicPackageAndShare extends AsyncTask<Integer,Integer,Integer> {
        private ProgressDialog dialog;
        TopicZipper tz;
        Uri uri;
        int progress = 0;

        TopicPackageAndShare( Activity a ) {
            tz = new TopicZipper( topic, aRecord, shareArrayList );
            dialog = new ProgressDialog( a );
            dialog.setProgressStyle( ProgressDialog.STYLE_HORIZONTAL );
            dialog.setMax( tz.filesToConvert );
            dialog.setTitle( "Generating Images" );
            dialog.show();
        }

        private void incProgress() {
            progress++;
            dialog.setProgress( progress );
            publishProgress(0);
        }

        @Override
        protected Integer doInBackground(Integer ... i) {
            try {
                tz.open();
                if( tz.addMessage() ) { incProgress(); }
                while( tz.addKey() ) { incProgress(); }
                tz.close();
                uri = tz.uri;
            } catch( Exception e ) {
                e.printStackTrace();
            }
            return 0;
        }

        @Override
        protected void onPostExecute(Integer i) {
            dialog.dismiss();
            if( uri != null ) {
                Intent intent = new Intent(android.content.Intent.ACTION_SEND, tz.uri);
                intent.setType("application/zip");
                intent.addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION);
                intent.putExtra(Intent.EXTRA_STREAM, tz.uri);
                startActivity(Intent.createChooser(intent, "Send Images"));
            }
        }
    }

    ////////////////////////////////////

    private class ShareArrayLoader extends AsyncTask<Cursor,Void,Long> {

        private ArrayList<ArtemisShare> al = new ArrayList<ArtemisShare>();

        @Override
        protected void onPreExecute() {
            al.clear();
        }

        @Override
        protected Long doInBackground(Cursor ... cursors) {
            if( cursors == null ) return null;
            if( cursors.length != 1 ) return null;
            if( cursors[0] == null ) return null;

            Cursor cursor = cursors[0];
            if (cursor.moveToFirst()) {
                do {
                    ArtemisShare oShare = new ArtemisShare( cursor );
                    if( oShare.stype == ArtemisLib.URI_A ) {
                        aRecord = oShare.share;
                    } else {
                        al.add( oShare );
                    }
                } while( cursor.moveToNext() );
            }
            cursor.close();
            return null;
        }

        @Override
        protected void onPostExecute(Long result) {
            shareArrayList.clear();
            for( ArtemisShare item : al) shareArrayList.add( item ); // addAll
            saa.notifyDataSetChanged();

            ArtemisTopic oTopic = artemisSql.getTopicInfo( topic );

            String message = oTopic.message;
            if( oTopic.message.length() == 0 ) { message = getResources().getString( R.string.sharelist_needmorekeys ); }
            if( aRecord.length() == 0 ) { message = getResources().getString( R.string.sharelist_messagedecodeerror ); }
            ((TextView) findViewById( R.id.message ) ).setText( message );

            String secret = aRecord;
            if( aRecord.length() == 0 ) { secret = getResources().getString( R.string.sharelist_nosecret ); }
            ((TextView) findViewById( R.id.secret ) ).setText( secret );
        }
    }

    ////////////////////////////////////

    // http://lucasr.org/2012/04/05/performance-tips-for-androids-listview/

    private class ShareArrayAdapter extends ArrayAdapter<ArtemisShare> {
        LayoutInflater inflater;

        public ShareArrayAdapter( Context cxt ) {
            super( cxt, R.layout.share_item, R.id.share, shareArrayList );
            inflater = (LayoutInflater) getApplicationContext().getSystemService(Context.LAYOUT_INFLATER_SERVICE);
        }

        @Override
        public View getView(int position, View convertView, ViewGroup parent) {
            View rowView;
            if( convertView == null ) { // view recycling and view holder pattern
                rowView = inflater.inflate(R.layout.share_item, parent, false);
                ArtemisShare.configureTags( rowView );
            } else {
                rowView = convertView;
            }
            shareArrayList.get(position).configureView( rowView );
            return rowView;
        }
    }

    ////////////////////////////////////

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.share_page);

        notifier = new Notifier( this );
        artemisSql = new ArtemisSQL( this );

        topic = getIntent().getStringExtra( "topic" );

        saa = new ShareArrayAdapter( getApplicationContext() );
        ListView lv = (ListView) findViewById( R.id.key_list );
        lv.setAdapter(saa);

        refreshListView();
    }

    ///////////////////////////

    public void refreshListView() {
        Cursor cursor = artemisSql.getShareTopicCursor( topic, sortOrder );
        if( cursor != null ) {
            (new ShareArrayLoader()).execute(cursor);
        }
    }

    ///////////////////////////

    public void onClickShare(View v) {
        (new TopicPackageAndShare( this )).execute(0);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        // http://stackoverflow.com/questions/8831050/android-how-to-read-qr-code-in-my-application
        super.onActivityResult(requestCode, resultCode, data);
        switch( requestCode ) {
            case ACTIVITY_COMPLETE:
                if (resultCode == RESULT_OK) {
                    break;
                } else if (resultCode == RESULT_CANCELED) {
                    break;
                    //handle cancel
                }
                break;
            default:
                break;
        }
    }

    ///////////////////////////
}
