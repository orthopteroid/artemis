package com.tereslogica.arcanacard;

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;

// http://developer.android.com/guide/topics/ui/dialogs.html
// http://code.tutsplus.com/tutorials/android-sdk-using-alerts-toasts-and-notifications--mobile-1949

public class Notifier {

    public static void ShowMessageOk(Context cxt, String title, String message, DialogInterface.OnClickListener ocl) {
        String response = cxt.getResources().getString(R.string.button_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, ocl)
                .show();
    }

    public static void ShowMessageOk(Context cxt, int stringid, String message, DialogInterface.OnClickListener ocl) {
        String response = cxt.getResources().getString(R.string.button_ok);
        String title = cxt.getResources().getString(stringid);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, ocl)
                .show();
    }

    public static void ShowText(Context cxt, int stringid) {
        String message = cxt.getResources().getString( stringid );
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setMessage(message)
                .show();
    }

    public static void ShowOk(Context cxt, int stringid, DialogInterface.OnClickListener ocl) {
        String message = cxt.getResources().getString( stringid );
        String title = cxt.getResources().getString(R.string.app_name);
        String response = cxt.getResources().getString(R.string.button_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, ocl)
                .show();
    }

    public static void ShowOk_AppVersionTitle(Context cxt, int stringid, DialogInterface.OnClickListener ocl) {
        String message = cxt.getResources().getString( stringid );
        String title = cxt.getResources().getString(R.string.app_name) +" v"+ BuildConfig.VERSION_NAME;
        String response = cxt.getResources().getString(R.string.button_ok);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, ocl)
                .show();
    }

    public static void ShowHowto(Context cxt, int stringid, DialogInterface.OnClickListener ocl) {
        String message = cxt.getResources().getString( stringid );
        String title = cxt.getResources().getString(R.string.dialog_howto);
        String response = cxt.getResources().getString(R.string.button_gotit);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setMessage(message)
                .setNeutralButton(response, ocl)
                .show();
    }

    public static void ShowMenu(Context cxt, int listid, DialogInterface.OnClickListener ocl ) {
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setItems(listid, ocl)
                .show();
    }

    public static void ShowMenu(Context cxt, int titleid, String items[], DialogInterface.OnClickListener ocl ) {
        String title = cxt.getResources().getString(titleid);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setItems(items, ocl)
                .show();
    }

    public static void ShowOptions(Context cxt, int titleid, int listid, int selectedid, DialogInterface.OnClickListener ocl) {
        String title = cxt.getResources().getString(titleid);
        new AlertDialog.Builder(cxt)
                .setIcon(R.drawable.app_icon)
                .setTitle(title)
                .setSingleChoiceItems(listid, selectedid, ocl)
                .show();
    }
}
