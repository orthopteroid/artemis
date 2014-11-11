package com.tereslogica.droidartemis;

import android.app.ProgressDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.database.Cursor;
import android.graphics.Bitmap;
import android.graphics.Color;
import android.net.Uri;
import android.os.AsyncTask;
import android.support.v4.content.FileProvider;
import android.support.v4.content.LocalBroadcastManager;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileOutputStream;
import java.util.ArrayList;
import java.util.zip.ZipEntry;
import java.util.zip.ZipOutputStream;

/**
 * Created by john on 28/10/14.
 */
public class Packager {
    private Context cxt;
    private String topic;
    private String sARecord = "";
    private ArrayList<ArtemisShare> oSRecords = new ArrayList<ArtemisShare>();
    private ProgressDialog dialog;

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
        private int key = 0;
        //
        public int filesToConvert;

        TopicZipper() {
            filesToConvert = ( sARecord.length() > 0 ? 1 : 0 ) + oSRecords.size();
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
            dir = new File( cxt.getCacheDir(), "");
            dir.mkdirs();

            String fname = "topic-" + topic + ".zip";
            fout = new File(dir, fname);

            uri = FileProvider.getUriForFile( cxt, cxt.getClass().getName(), fout); // use classname of cxt child

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
            if( key == oSRecords.size() ) { return false; }
            ArtemisShare sRecord = oSRecords.get( key );
            String istr = String.format("%05d", key); // 16 bits serialized = 4095 = 5 decimal digits, 0 padding
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

    private class QueryAndPackage extends AsyncTask<Integer,Integer,Integer> {
        QueryAndPackage thisTask;
        TopicZipper tz;
        Uri uri;
        int progress = 0;

        private void incProgress() {
            progress++;
            dialog.setProgress( progress );
            publishProgress(0);
        }

        /////////////////////////

        @Override
        protected void onPreExecute() {
            thisTask = this;

            Cursor cursor = ArtemisSQL.Get().getShareTopicCursor( topic, ArtemisSQL.SortOrder.NATURAL );
            if( cursor == null ) { return; }

            dialog = new ProgressDialog( cxt );
            dialog.setProgressStyle( ProgressDialog.STYLE_HORIZONTAL );
            dialog.setTitle( "Generating Images" );
            dialog.setCancelable(true);
            dialog.setOnCancelListener(
                new DialogInterface.OnCancelListener() {
                    public void onCancel(DialogInterface dialog) {
                        thisTask.cancel(true);
                    }
                }
            );

            if (cursor.moveToFirst()) {
                do {
                    ArtemisShare oShare = new ArtemisShare(cursor);
                    if (oShare.stype == ArtemisLib.URI_A) {
                        sARecord = oShare.share;
                    } else {
                        oSRecords.add(oShare);
                    }
                } while (cursor.moveToNext());
            }
            cursor.close();

            tz = new TopicZipper();
            dialog.setMax(tz.filesToConvert);
            dialog.show();
        }

        @Override
        protected Integer doInBackground(Integer ... i) {
            uri = null;
            if( oSRecords.size() > 0 || sARecord.length() > 0 ) {
                try {
                    tz.open();
                    if (tz.addMessage()) {
                        incProgress();
                    }
                    while (tz.addKey()) {
                        incProgress();
                    }
                    tz.close();
                    uri = tz.uri;
                } catch (Exception e) {
                    e.printStackTrace();
                }
            }
            return 0;
        }

        @Override
        protected void onPostExecute(Integer i) {
            dialog.dismiss();

            if( this.isCancelled() ) { return; }
            if( uri == null ) { return; }

            LocalBroadcastManager.getInstance(cxt).sendBroadcast( (new Intent( ActivityTopics.INTENT_PACKAGE )).putExtra( ActivityTopics.EXTRA_URISTRING, uri.toString() ) );
        }
    }

    ///////////////////////////

    public Packager( Context _c, String _t ) {
        cxt = _c;
        topic = _t;
        sARecord = "";
        oSRecords.clear();
        (new QueryAndPackage()).execute(0);
    }

}
