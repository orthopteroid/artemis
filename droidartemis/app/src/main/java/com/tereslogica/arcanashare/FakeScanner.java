package com.tereslogica.arcanashare;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&ai=BAAUF&as=I2hkGCTe-9Hqvg!eS1F57gGqdk4aGCO&mt=PaFY1Vq6OygEuvek8w&mc=bWFpbg",
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&si=BAAUF&ss=IwUmdwd33OWzlQ!9ohlMzQory1VqIfU&sh=AAE!A_teN4JctOgMoCLj&sc=b25l",
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&si=BAAUF&ss=KBCkc1z74ao4BQ!W72SDotO2OBQHQRW&sh=AAI!APU9SS66A6QeETvj&sc=dHdv",
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&si=BAAUF&ss=KZ9VTChPU-Xc5A!JoCo2M2Kw3ZJB27t&sh=AAM!B67OEtuU5TxvjmIm&sc=dGhyZWU",
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&si=BAAUF&ss=HD_fe6yfd4X-Xw!9IKwsH3xnKRMwS79&sh=AAQ!BBNVU_Ws4HBwcPlu&sc=Zm91cg",
"http://as.tereslogica.com?tp=D_w6YKJCgZWsfrMh&si=BAAUF&ss=HH3dCQietDrjGw!h1aiLwvyUgAZgmqu&sh=AAU!BT7klhDbeEZLDp7G&sc=Zml2ZQ",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&ai=BAAUE&as=EhZmzs3PeYphUw!hdVtdSPhZLz2pYdS&mt=7H-q_WcOsq1-mBj5rQ&mc=bWFpbg",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&si=BAAUE&ss=PgXat3idYunxsw!tCaQafkbm8Z2HlXG&sh=AAE!BOmKAbBnhuUKq11c&sc=b25l",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&si=BAAUE&ss=PFXQuhCr5rplIg!C0Mn56wu03OyozNV&sh=AAI!A7QJGcao6S8vbXGv&sc=dHdv",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&si=BAAUE&ss=G9pYcGXO7-lcVg!qZ1pKYJThAgdF4j4&sh=AAM!BfQZKx-PIPgGav60&sc=dGhyZWU",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&si=BAAUE&ss=IvGmLkupou2KDQ!Rmrx4ttwABKt8acI&sh=AAQ!B2ZR3egzC94qQ728&sc=Zm91cg",
"http://as.tereslogica.com?tp=Cpv7s4MrxGpoHOug&si=BAAUE&ss=ABHb0y4YEfDVeg!xzlh5CyPOWAvk5UO&sh=AAU!A_bhy0YNA_Grq95G&sc=Zml2ZQ",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&ai=BAAUD&as=BexrsqRh0EBSmg!kSBa3002IL740QRM&mt=cTtJty1H8KLSl1cT-Ag&mc=bWFpbg",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&si=BAAUD&ss=IKQuC6V5FvLk4w!GudrqYyRSljcFTJQ&sh=AAE!AEYwgyAvIA6Tr22l&sc=b25l",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&si=BAAUD&ss=Al7Sdoa2gANI6g!FH1atT5ZG22bETwg&sh=AAI!AOtQ45OI2KmJK_LT&sc=dHdv",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&si=BAAUD&ss=GAG5ipNfe_wf3A!bEylll8deTUNpL61&sh=AAM!AkCQJ-x-dUPG5Rtm&sc=dGhyZWU",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&si=BAAUD&ss=H6qK7dbluOybCw!NRnsnB23TEHnVPWf&sh=AAQ!A6-15Am7CRiZoJ8p&sc=Zm91cg",
"http://as.tereslogica.com?tp=DaOI7smZLoCZaZCR&si=BAAUD&ss=GVyNu0mJGlSuJw!lHt5L65VUmjnPCvq&sh=AAU!AQR1IHZNpPLWbnac&sc=Zml2ZQ",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&ai=BAAUC&as=Fr15RfMPt3HYkA!WyUq1PyE3Es1Z8c4&mt=UOf-pQwnMqfgPcch&mc=bWFpbg",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&si=BAAUC&ss=PQpuJVLd9ZgLlA!wOff1MLzW-EGT3fu&sh=AAE!Ab5Mt6lgrJU7TgzL&sc=b25l",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&si=BAAUC&ss=NHtN5HOVEWmk8w!YVUD9CTPeCcd7Lq0&sh=AAI!B5NZc_VNZjvYLIcI&sc=dHdv",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&si=BAAUC&ss=E7cBVD6dj2mzYA!k_Q0t47LLcx2GF19&sh=AAM!BYhVzj45395dDR5A&sc=dGhyZWU",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&si=BAAUC&ss=OyYlpbKeufXVtw!XS61kRFBBnpnTAW7&sh=AAQ!A8-y-00G_uYa6vCO&sc=Zm91cg",
"http://as.tereslogica.com?tp=AMzDkHJTvTh5k6Rr&si=BAAUC&ss=HTlFfVfB9bmisw!zHuM2cXeJ8qbW1I-&sh=AAU!AdS-RoZyRwOfy2nG&sc=Zml2ZQ"
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
