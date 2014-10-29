package com.tereslogica.droidartemis;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.support.v4.content.LocalBroadcastManager;

// http://developer.android.com/guide/topics/ui/dialogs.html
// http://code.tutsplus.com/tutorials/android-sdk-using-alerts-toasts-and-notifications--mobile-1949

public class Notifier {

    public final static String INTENT_SHARE = "intent-share";
    public final static String INTENT_PACKAGE = "intent-package";
    public final static String EXTRA_TOPICSTRING = "intent-topic";
    public final static String EXTRA_URISTRING = "intent-uristring";

    public static void ShowOk(Context cxt, int stringid) {
        String message = cxt.getResources().getString( stringid );
        String title = cxt.getResources().getString(R.string.app_name);
        String response = cxt.getResources().getString(R.string.button_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, null)
                .show();
    }

    public static void ShowMessage(Context cxt, String message) {
        String title = cxt.getResources().getString(R.string.app_name);
        String response = cxt.getResources().getString(R.string.button_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, null)
                .show();
    }

    public static void ShowMessageAndPosiblyShare(final Context cxt, final String topic, String message) {
        String title = cxt.getResources().getString(R.string.app_name);
        String sCancel = cxt.getResources().getString(R.string.button_cancel);
        String sShare = cxt.getResources().getString(R.string.button_share);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setCancelable( true )
                .setPositiveButton( sCancel, null )
                .setNegativeButton( sShare,
                        new DialogInterface.OnClickListener() {
                            public void onClick(DialogInterface dialog, int id) {
                                LocalBroadcastManager.getInstance(cxt).sendBroadcast(new Intent(Notifier.INTENT_SHARE).putExtra( Notifier.EXTRA_TOPICSTRING, topic ));
                            }
                        }
                )
                .show();
    }

    public static void ShowOptions(Context cxt,int listid, DialogInterface.OnClickListener ocl ) {
        String title = cxt.getResources().getString(R.string.app_name);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setItems(listid, ocl)
                .show();
    }
}
