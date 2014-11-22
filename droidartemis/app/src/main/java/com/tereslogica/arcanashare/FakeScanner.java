package com.tereslogica.arcanashare;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&ai=BAAUF&as=IzdLeMaafe5mfA!u70eteJQTjzqSSuL&mt=WgKTAIa0i-GFIsfWHw&mc=bWFpbg",
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&si=BAAUF&ss=EHQc3FHf62aKpA!7BYAzaYp3tV2zEak&sh=AAE!ATftQsmdAT9_Ne_d&sc=b25l",
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&si=BAAUF&ss=L1A9OxV9uSSYPA!N1Bw_LDITsq3m5Tp&sh=AAI!BMGVwYdSaQNNw-p-&sc=dHdv",
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&si=BAAUF&ss=EcTCWlfjcy4hog!RjdqxCFWc6BfAD5J&sh=AAM!Ac696MCUNzB09HIZ&sc=dGhyZWU",
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&si=BAAUF&ss=PH-GsBmIuRviqA!Qo5275eBpG6Qzsf4&sh=AAQ!AAT6yh6UhXgBDI7k&sc=Zm91cg",
"http://arcanashare.com?tp=AlIJ79iIbNqcZU8H&si=BAAUF&ss=LMpNbciECER-Sw!UI5eRrkddHkRTsBW&sh=AAU!Asn-NHY7bMW89odX&sc=Zml2ZQ",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&ai=BAAUE&as=ETfGS_aaqF2wGQ!lHTHMBClpCftG-uQ&mt=QkEmiJ2O6Y5BoJFaHQ&mc=bWFpbg",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&si=BAAUE&ss=Jerhru32pxS1aQ!kv8xKk_RqnFYBM5E&sh=AAE!BzoCYmJCcdnJF-tZ&sc=b25l",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&si=BAAUE&ss=BHO2q5icN5iAXQ!rXj9dKBg_M2MLF7y&sh=AAI!BY8ThTtX-WQCt1vy&sc=dHdv",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&si=BAAUE&ss=LuW7j0M_QRb0Gw!BUCayMsLRvuVg_5N&sh=AAM!BleqdlTGl_-PAWjQ&sc=dGhyZWU",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&si=BAAUE&ss=F618Sh_HWDzp6g!V3VIoU9R7w7-zal_&sh=AAQ!ASpevJFT2UNGTzEB&sc=Zm91cg",
"http://arcanashare.com?tp=DJ3wof-xT7bRB7aK&si=BAAUE&ss=Flh-tZ_qGaZEPA!oOEqzHCbtyDbGif_&sh=AAU!A7pf8UQNLtpRpbDd&sc=Zml2ZQ",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&ai=BAAUD&as=CZ3O7gjdUWB-XA!i2hgsrZMIzGyQslD&mt=TY0THr30ez8BP0FWOjw&mc=bWFpbg",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&si=BAAUD&ss=P_bPFyOQuplniA!CAHsjtSpFxR0xIAE&sh=AAE!A7jzTAQWBNj9HjJq&sc=b25l",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&si=BAAUD&ss=OTfl7lewVi-ZXA!iWHb2MKa6A84OHeD&sh=AAI!BaX9LfdWljkE3BFu&sc=dHdv",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&si=BAAUD&ss=Mwnb6T5Hg6qEow!8_mAzrZjPClcaPRQ&sh=AAM!ALB_JP0q-A161hNM&sc=dGhyZWU",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&si=BAAUD&ss=Am058GoHjHMctA!P8VMr5tsL5GNlk8K&sh=AAQ!BClIdUwzJ7J7_6eM&sc=Zm91cg",
"http://arcanashare.com?tp=AWGVHNdAq9O_MqrM&si=BAAUD&ss=K7oYUbg4oCyDdw!Zow3crLW0mJz4wbh&sh=AAU!ATzKfEZPSYYF9aWu&sc=Zml2ZQ",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&ai=BAAUC&as=HvJZzyE92JGPAA!9Khr2fJgDj7TS0uz&mt=gom9KTJahAzJqfhc&mc=bWFpbg",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&si=BAAUC&ss=Hx-c-JsNzZegdw!W3TpnYmREbN0EAA3&sh=AAE!BbEYjdaBGirco9GH&sc=b25l",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&si=BAAUC&ss=PAu7s4_klRAgYw!agO8Oo6MoaDkw05P&sh=AAI!A3_7DkHWGj9JFf_t&sc=dHdv",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&si=BAAUC&ss=AZCDccSmOKMeFw!FS5mZP9s2v7f1KLX&sh=AAM!BsfaYszrHrPGeSXC&sc=dGhyZWU",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&si=BAAUC&ss=OwgKqEYut32iPg!oMqbVjqhEe-DmaSS&sh=AAQ!BuS8PW7IGhRieaM5&sc=Zm91cg",
"http://arcanashare.com?tp=Dj9qr_RW862oaezi&si=BAAUC&ss=BiGjYgh490tcsA!5AWXXcoBqzBSdZLv&sh=AAU!A1ydUeP1HpjtFXkW&sc=Zml2ZQ"
    };

    Random rnd = new Random();
    int ordering[] = new int[ itemArr.length ];
    final boolean scramble = true;

    public FakeScanner() {
        for (int j = 0; j < itemArr.length; j++) {
            ordering[j] = j;
        }
        if( scramble ) {
            for (int j = 0; j < itemArr.length; j++) {
                int a = rnd.nextInt(itemArr.length);
                int b = rnd.nextInt(itemArr.length);
                int swap = ordering[a];
                ordering[a] = ordering[b];
                ordering[b] = swap;
            }
        }
    }

    public String nextItem() {
        String s = itemArr[ ordering[ i++ ] ];
        if( i == itemArr.length ) i = 0;
        return s;
    }
}
