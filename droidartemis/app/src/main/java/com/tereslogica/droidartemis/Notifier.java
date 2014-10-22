package com.tereslogica.droidartemis;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.content.res.Resources;

// http://developer.android.com/guide/topics/ui/dialogs.html
// http://code.tutsplus.com/tutorials/android-sdk-using-alerts-toasts-and-notifications--mobile-1949

public class Notifier {
    private Context cxt;
    private Resources rez;

    ////////////////////////////

    private static Notifier instance = null;

    private Notifier(Context _cxt) {
        cxt= _cxt;
        rez = _cxt.getResources();
    }

    static void Init(Context _cxt) {
        if( instance == null ) { instance = new Notifier( _cxt ); }
    }
    static void Cleanup() {
        instance = null;
    }
    static Notifier Get() {
        return instance;
    }

    //////////////////////

    public void showOk(int stringid) {
        String message = rez.getString( stringid );
        String title = rez.getString(R.string.app_name);
        String response = rez.getString(R.string.reponse_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setNeutralButton(response, null)
                .setMessage(message)
                .show();
    }

    public void showOptions(int listid, DialogInterface.OnClickListener ocl ) {
        String title = rez.getString(R.string.app_name);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setItems(listid, ocl)
                .show();
    }
}
