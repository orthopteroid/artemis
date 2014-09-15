package com.tereslogica.droidartemis;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.content.DialogInterface;
import android.os.Bundle;
import android.support.v4.app.DialogFragment;

// http://developer.android.com/guide/topics/ui/dialogs.html

public class DialogNoScanner extends DialogFragment {

    public interface DialogNoScannerListener {
        public void onDialogNoScanner_Click(DialogFragment dialog);
    }

    DialogNoScannerListener mListener;

    @Override
    public void onAttach(Activity activity) {
        super.onAttach(activity);
        // Verify that the host activity implements the callback interface
        try {
            // Instantiate the NoticeDialogListener so we can send events to the host
            mListener = (DialogNoScannerListener) activity;
        } catch (ClassCastException e) {
            // The activity doesn't implement the interface, throw exception
            throw new ClassCastException(activity.toString()
                    + " must implement DialogNoScannerListener");
        }
    }

    @Override
    public Dialog onCreateDialog(Bundle savedInstanceState) {
        // Use the Builder class for convenient dialog construction
        AlertDialog.Builder builder = new AlertDialog.Builder(getActivity());
        builder.setMessage(R.string.notify_noscanner)
                .setPositiveButton(R.string.reponse_ok, new DialogInterface.OnClickListener() {
                    public void onClick(DialogInterface dialog, int id) {
                        mListener.onDialogNoScanner_Click(DialogNoScanner.this);
                    }
                });
        // Create the AlertDialog object and return it
        return builder.create();
    }
}