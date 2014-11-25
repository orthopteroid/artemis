package com.tereslogica.acanacard;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&ai=BAAUF&as=CWyQHoQPOck7bw!zU-zA1KFrpZwtsB9&mt=kbb_tDduQzQ5fxLfmg&mc=bWFpbg",
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&si=BAAUF&ss=DmBLbq2qSdGVbw!vNe0HqcFlQha0ON-&sh=AAE!Al041fS35W_v9Miu&sc=b25l",
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&si=BAAUF&ss=LqqY9bVJ7KPO_A!5__clmN-aLASC9GL&sh=AAI!AMU2W_shEjs_hX57&sc=dHdv",
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&si=BAAUF&ss=GQfKxRN5l_B49Q!k44dbB7NYQ2tXGRA&sh=AAM!BRixdSEZwvQ1HLAv&sc=dGhyZWU",
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&si=BAAUF&ss=O-xgqYEzv6rgvQ!VNPJpE_wOtx1sx2p&sh=AAQ!AMu_Ecgw6VMbqZoA&sc=Zm91cg",
"http://arcanacard.com?tp=AcNtmwvjT-m-CIlh&si=BAAUF&ss=FWDFO0l76Tqt2Q!bgJ-rVSpPZTQB66d&sh=AAU!BAY0oq8kbR3nrgFd&sc=Zml2ZQ",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&ai=BAAUE&as=GuS9_Mme9n6TbA!0Oh7iN34BajRukcw&mt=oawd19LX9U5omFqUCg&mc=bWFpbg",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&si=BAAUE&ss=NiyAMkmi--CD0g!A_RT4BhOLd5sINR1&sh=AAE!Aj0zzH1RM5iIijO2&sc=b25l",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&si=BAAUE&ss=Hyzlqe2HJ5K02w!D3AXaOzsO3fXf25K&sh=AAI!B8W-DC8OaZi3VEsB&sc=dHdv",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&si=BAAUE&ss=MkyecJbqTKbagQ!JQvDXFhk4NwHj6Vi&sh=AAM!Ber4h1p8EDghvZ6a&sc=dGhyZWU",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&si=BAAUE&ss=KMiC54v6-2YaIA!CBBTbanVaWtbkZrE&sh=AAQ!Azld5t5CRup-tylM&sc=Zm91cg",
"http://arcanacard.com?tp=AsQwYBPTZkBybZQn&si=BAAUE&ss=De6jHtICTOsWJQ!f-q93tyy1EtHR8c5&sh=AAU!BMo6zSPJa2Qiq6CH&sc=Zml2ZQ",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&ai=BAAUD&as=DHyRNWFuM4Vlyg!gXfdmgkFL9Yss8UI&mt=IEUW2vcXrKPEyGPpviA&mc=bWFpbg",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&si=BAAUD&ss=K799Ivn1SgE5VQ!wgMgEBUSi6X-KLs4&sh=AAE!BtczpR6fGj43DDW-&sc=b25l",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&si=BAAUD&ss=K00R4w1kQ5UASA!_NbmDxGPBDO82SDJ&sh=AAI!BuSfQtQtHQeodACS&sc=dHdv",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&si=BAAUD&ss=OP2gtAfCssbGxQ!Oo-2wK5hDziR0eWo&sh=AAM!A8wflBnr_IPWGsyd&sc=dGhyZWU",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&si=BAAUD&ss=I4JKaoDEP4yEeg!chWFg9e4UGoOZDPM&sh=AAQ!BPcdOa50u5WEbE8Y&sc=Zm91cg",
"http://arcanacard.com?tp=BS-wblz1FrLqFBEg&si=BAAUD&ss=B4DszQvK_gN5AQ!LyFo8Y2hNP4gQV-F&sh=AAU!Ad-d72OyWhH6AoMX&sc=Zml2ZQ",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&ai=BAAUC&as=HZYGTSigTroSBQ!jAHCpE0h11IwpzdD&mt=meV8y3qAMwspnkpr&mc=bWFpbg",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&si=BAAUC&ss=INuZJsPpY-FJbw!P8SihHt5_F4r-gZx&sh=AAE!ATsazDoxKNKNvrSQ&sc=b25l",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&si=BAAUC&ss=Fb9ERFbNdA8svQ!IUj86CXjPBGuqbls&sh=AAI!AOauw8Mx5aL4rwoV&sc=dHdv",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&si=BAAUC&ss=GkWDHNFqZwVz4w!wWGtRm_Q5J4nO_hR&sh=AAM!AFI9OJSuWnLzoV-f&sc=dGhyZWU",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&si=BAAUC&ss=EVmkVChz5lju4A!c_1OGUFw16JOT8OM&sh=AAQ!A12G6DCAf8J-jFcE&sc=Zm91cg",
"http://arcanacard.com?tp=B0w4NHy3FfZy2bea&si=BAAUC&ss=F63Yc1GqFVwZWw!HduPrPbZkg0FQLTy&sh=AAU!A-kVE2cfwBJ1ggKO&sc=Zml2ZQ"

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
