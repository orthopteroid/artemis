package com.tereslogica.arcanashare;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://tereslogica.com?tp=B1truJj-moWFCnHV&ai=BAAUF&as=B6zvjik8tdFIYQ!JKerbWwBsazopF-0&mt=1KHR5-uwSRQOcvh06g&mc=bWFpbg",
"http://tereslogica.com?tp=B1truJj-moWFCnHV&si=BAAUF&ss=Nzh8oqYqxb7bYA!N5zt5Lh1Z3IkvlFi&sh=AAE!AKVyCL6LT8E64J8A&sc=b25l",
"http://tereslogica.com?tp=B1truJj-moWFCnHV&si=BAAUF&ss=BDs3nm89l1DSXA!Vsut_dXb34eHkqTL&sh=AAI!BM0TGnvgpI1F21ip&sc=dHdv",
"http://tereslogica.com?tp=B1truJj-moWFCnHV&si=BAAUF&ss=D3pemMlcs1ojHA!jlrdR-3zt-bBo9yR&sh=AAM!Ay0qI8rF5-BT70wx&sc=dGhyZWU",
"http://tereslogica.com?tp=B1truJj-moWFCnHV&si=BAAUF&ss=PbdZRZsg--Ws-A!0y4kBo00WX1a59LS&sh=AAQ!AdqwSufGOWi9v7l_&sc=Zm91cg",
"http://tereslogica.com?tp=B1truJj-moWFCnHV&si=BAAUF&ss=PbAn35O_ICIDDQ!0tDo0u-n-mpFLqeh&sh=AAU!BqWwKMf2nIOzUnl4&sc=Zml2ZQ",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&ai=BAAUE&as=GyaTPeG5tJNXSg!AYYykEloEENRVcFu&mt=2SRYHQDMiwMa9NCkzQ&mc=bWFpbg",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&si=BAAUE&ss=IRBqOuHY_uTd8Q!k1dqgpkdqjoL1_b4&sh=AAE!A8Py_hkNnNTyGU4m&sc=b25l",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&si=BAAUE&ss=FMPdMMKUXVp4PQ!PTD8HNFvx07McZMS&sh=AAI!BpJgZxYrSq_RIxf3&sc=dHdv",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&si=BAAUE&ss=Fc_1fDQ0WyFqnw!4w-oaW8VXKrv58Jn&sh=AAM!A8WK_OApZxvRUPy0&sc=dGhyZWU",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&si=BAAUE&ss=OCT1PFSz06nzYg!anfRRmjYmcBtkTTP&sh=AAQ!Bfsc-COM7xBg2uyu&sc=Zm91cg",
"http://tereslogica.com?tp=DXyBE4KXtVgV7Jik&si=BAAUE&ss=NSgFiRsx6Nvdhw!wusl_zWOB-0wz4z8&sh=AAU!BZcjB2OZNj5z0X6Q&sc=Zml2ZQ",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&ai=BAAUD&as=GN9OjBFkDJUSHA!LRunG4BQGUWpPKMi&mt=RYtkMZ2ye9WgZoSj7lU&mc=bWFpbg",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&si=BAAUD&ss=GIPBNoaFn_q2aw!g-BAFjERaAcuv6hc&sh=AAE!A0oiE5NUOWVE_HuZ&sc=b25l",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&si=BAAUD&ss=BSmsnw3yjjLUvw!vIdBteknK9HW2Vz2&sh=AAI!AppRwmUx2YpeoHi9&sc=dHdv",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&si=BAAUD&ss=CnmypA_Y4wb6-g!y0JMtHP60nAcnRw0&sh=AAM!Aq6VZJtTw0SMr9Km&sc=dGhyZWU",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&si=BAAUD&ss=ATWxDixDOpSRyg!1gPKvgoUn4XrxPD9&sh=AAQ!BoPw3K4265h7apf1&sc=Zm91cg",
"http://tereslogica.com?tp=CXLA5Owg-NYmUeqw&si=BAAUD&ss=OeKxJZid0MMKTg!uaMZZhWjFt_4rYmA&sh=AAU!Brc0elBU8VapZT3u&sc=Zml2ZQ",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&ai=BAAUC&as=G134eeEaWyeKwg!PCWjPSq9Q9A48qEG&mt=k4lArFtp6kMIX9gA&mc=bWFpbg",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&si=BAAUC&ss=PiaxRtqadgMcjQ!o6FE8io2x2S3WUcD&sh=AAE!AI-WFC2sHbW87kC5&sc=b25l",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&si=BAAUC&ss=IZSuXDRKXzg_IQ!Q10WnJgat0Ua8TyX&sh=AAI!Bf_eH3-lLwQ0-ouy&sc=dHdv",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&si=BAAUC&ss=EhkYnwDseDIhTg!MMBe6KxBURCgZ8_Y&sh=AAM!BtAmC7GiPuto9s1L&sc=dGhyZWU",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&si=BAAUC&ss=HGAHslG4lZnJRQ!kr6folVBYNIUsVJX&sh=AAQ!BxmOC9oXR-ck0x2k&sc=Zm91cg",
"http://tereslogica.com?tp=AePBw9n51FxmLPQG&si=BAAUC&ss=DaKsnLx_Vlhrjg!UDaCi7A4vTWKl-n3&sh=AAU!BDZ2HxQQVgh431td&sc=Zml2ZQ"
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
