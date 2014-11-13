package com.tereslogica.acanashare;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

// http://developer.android.com/guide/topics/ui/dialogs.html
// http://code.tutsplus.com/tutorials/android-sdk-using-alerts-toasts-and-notifications--mobile-1949

public class Notifier {

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

    public static void ShowOptions(Context cxt,int listid, DialogInterface.OnClickListener ocl ) {
        String title = cxt.getResources().getString(R.string.app_name);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setItems(listid, ocl)
                .show();
    }
}
