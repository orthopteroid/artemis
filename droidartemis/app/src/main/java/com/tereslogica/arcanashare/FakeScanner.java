package com.tereslogica.arcanashare;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://tereslogica.com?tp=UGVfil85_Xo&ai=BAAUF&vf=vEM&pk=C1lyZKLavhPs4Lfe&as=QHLDe5b-rz00Xg!sbYOkH2YoITXYq-N&mt=Wf5pg7_X9AVmA-S-WA&mc=bWFpbg",
"http://tereslogica.com?tp=UGVfil85_Xo&si=BAAUF&sh=AAE!BnkfjB82y5psfdxD&ss=A0wzfp7YVeA_HA!owI82cvjc1zOaDf0&sc=b25l",
"http://tereslogica.com?tp=UGVfil85_Xo&si=BAAUF&sh=AAI!Awi7SCic10Js6uHf&ss=Mu_y5M_b8GanYA!CriFnUerRlmXQnXB&sc=dHdv",
"http://tereslogica.com?tp=UGVfil85_Xo&si=BAAUF&sh=AAM!AfH7qlDQZRbOUz2d&ss=Lophjx5qIeNHxA!EgD2eboWg4YHWZgZ&sc=dGhyZWU",
"http://tereslogica.com?tp=UGVfil85_Xo&si=BAAUF&sh=AAQ!ALznGIMJ_jxJ0kr5&ss=ANuqKucjbz3a6g!k7EbLuIXufLG1KqS&sc=Zm91cg",
"http://tereslogica.com?tp=UGVfil85_Xo&si=BAAUF&sh=AAU!BwBAZJpY1Ti4mUnA&ss=CtzvXMVx6jZehw!wsELKfpZxwTSwhmI&sc=Zml2ZQ",
"http://tereslogica.com?tp=e_Xev_oUqKc&ai=BAAUE&vf=D6o&pk=A3bJ0179cpsf1RdH&as=OPAvQr6OvI_9Tw!qzYCftUdR0wn4-cO&mt=AOPulY2hLtRATYnJBg&mc=bWFpbg",
"http://tereslogica.com?tp=e_Xev_oUqKc&si=BAAUE&sh=AAE!BRvDzNaxs4CO6fka&ss=BX6cglDBDSScog!0fVn5_t4ev92AhAg&sc=b25l",
"http://tereslogica.com?tp=e_Xev_oUqKc&si=BAAUE&sh=AAI!AzU82TtltD1uMEWK&ss=JPHiDO7SAOtJWA!pda-foVmNWViBq12&sc=dHdv",
"http://tereslogica.com?tp=e_Xev_oUqKc&si=BAAUE&sh=AAM!BCYdhNfz4GmL48d2&ss=O-vcTBIOaamsaA!6XTETbIuA7R2o2a_&sc=dGhyZWU",
"http://tereslogica.com?tp=e_Xev_oUqKc&si=BAAUE&sh=AAQ!Aq7onf3q3qxJ-mjh&ss=GpzsrCO-1nl0uQ!ZXgyAeNjSMfnnYix&sc=Zm91cg",
"http://tereslogica.com?tp=e_Xev_oUqKc&si=BAAUE&sh=AAU!Bsn3CZt8PloeNjWZ&ss=K3eV8XsOadmWMA!kJlWoj4SJ5lYjLqg&sc=Zml2ZQ",
"http://tereslogica.com?tp=6bFRglHRySI&ai=BAAUD&vf=wvI&pk=BXSHcMJ-Ajx85_AN&as=Fpyqmjy9Q7qZag!soonMFSMVIcOCLlQ&mt=gs-R3igkJKpsdTc3T-Y&mc=bWFpbg",
"http://tereslogica.com?tp=6bFRglHRySI&si=BAAUD&sh=AAE!B4tnsSgIQ5EDqvgi&ss=K2Z7wzw_lZSReQ!8LQ4lTY4hzh8ho0E&sc=b25l",
"http://tereslogica.com?tp=6bFRglHRySI&si=BAAUD&sh=AAI!A0LG7orSlP4D8_Vd&ss=No6wQNkdDiKRwg!jX3Zuu-OK9y_VJgC&sc=dHdv",
"http://tereslogica.com?tp=6bFRglHRySI&si=BAAUD&sh=AAM!Aic1yq_u3kTmDbNY&ss=NRkLZQqF0xsmaw!EK6rccSy5IPlT9IY&sc=dGhyZWU",
"http://tereslogica.com?tp=6bFRglHRySI&si=BAAUD&sh=AAQ!ACSgKzeRMICcc_UT&ss=Jxe9ObxeT57XeQ!FUj6GHKqsbtEgdqA&sc=Zm91cg",
"http://tereslogica.com?tp=6bFRglHRySI&si=BAAUD&sh=AAU!AUFTDxKtejp5jbMW&ss=OtlONx3Ym_2jRA!RMKXdkbe88DWfbSB&sc=Zml2ZQ",
"http://tereslogica.com?tp=6nMx7fLnMZ8&ai=BAAUC&vf=vpg&pk=DXDBBRC3bUQnTQoV&as=F_RlLWU6_jQceg!mAqGHmqVOXncXSmN&mt=yXZDbwQQmDVocrM3&mc=bWFpbg",
"http://tereslogica.com?tp=6nMx7fLnMZ8&si=BAAUC&sh=AAE!BCxlPzSihVBgq27C&ss=Lzf-_6Qc-wr4bw!nKGjv2mpEpcYGlWH&sc=b25l",
"http://tereslogica.com?tp=6nMx7fLnMZ8&si=BAAUC&sh=AAI!ApSXuBvKS118H4jE&ss=FybKqdbGChcwXA!PakPlWd3_LtvSDwR&sc=dHdv",
"http://tereslogica.com?tp=6nMx7fLnMZ8&si=BAAUC&sh=AAM!APzGOv7t8VmLjQrG&ss=QD-T7iUX3NYFvg!_Q-ysQKyUpfSrniJ&sc=dGhyZWU",
"http://tereslogica.com?tp=6nMx7fLnMZ8&si=BAAUC&sh=AAQ!B-PygkS72sdFdkTI&ss=I4wBUGohGEtoaA!Tj3UvAg0WqNgk8wD&sc=Zm91cg",
"http://tereslogica.com?tp=6nMx7fLnMZ8&si=BAAUC&sh=AAU!BYujAKGcYMOy5MbK&ss=EOp4ScawcepC5w!2Thzm5xRnIfUdCg_&sc=Zml2ZQ"
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
