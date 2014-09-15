package com.tereslogica.droidartemis;

import android.app.AlertDialog;
import android.content.Context;
import android.content.res.Resources;

// http://developer.android.com/guide/topics/ui/dialogs.html
// http://code.tutsplus.com/tutorials/android-sdk-using-alerts-toasts-and-notifications--mobile-1949

public class Notifier {
    Context cxt;
    Resources rez;
    public Notifier(Context _cxt, Resources _rez) {
        cxt= _cxt;
        rez = _rez;
    }
    public void show(int stringid) {
        String message = rez.getString( stringid );
        String title = rez.getString(R.string.app_name);
        String response = rez.getString(R.string.reponse_ok);
        new AlertDialog.Builder(cxt).setTitle(title).setMessage(message).setIcon(R.drawable.app_icon).setNeutralButton(response, null).show();
    }
}
