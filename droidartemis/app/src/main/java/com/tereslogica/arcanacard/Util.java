package com.tereslogica.arcanacard;

import android.graphics.Bitmap;
import android.graphics.Color;

import com.google.zxing.BarcodeFormat;
import com.google.zxing.common.BitMatrix;
import com.google.zxing.qrcode.QRCodeWriter;

import java.io.ByteArrayOutputStream;

/**
 * Created by john on 20/11/14.
 */
public class Util {
    public static Bitmap StringToQRBitmap(String str, int width, int height) throws Exception {
        QRCodeWriter qrcw = new QRCodeWriter();
        BitMatrix bm = qrcw.encode( str, BarcodeFormat.QR_CODE, width, height );

        Bitmap bitmap = Bitmap.createBitmap( width, height, Bitmap.Config.RGB_565 );
        for (int i = 0; i < width; i++) {
            for (int j = 0; j < height; j++) {
                bitmap.setPixel(i, j, bm.get(i, j) ? Color.BLACK: Color.WHITE);
            }
        }
        return bitmap;
    }
    public static ByteArrayOutputStream StringToQRByteStream(String str, int width, int height) throws Exception {
        Bitmap bitmap = StringToQRBitmap(str, width, height);
        ByteArrayOutputStream bos = new ByteArrayOutputStream();
        bitmap.compress(Bitmap.CompressFormat.PNG, 0, bos);
        return bos;
    }
}
