package com.tereslogica.droidartemis;

import java.util.Random;

/**
 * Created by john on 09/09/14.
 */
public class FakeScanner {

    int i  = 0;

    String[] itemArr =
            {
                    "http://foo.bar?tp=szvNzIafmi8&ai=AF!AD!CI&vf=Hkb7QA&pk=AZoMKUOUDRZf_dB4okXV-YS4&as=cpWo8SblmAHxfXcziacj4Q!lx10akSXo9zf6MTCypAJ9g&mc=MA&mt=M0IaIYWH",
                    "http://foo.bar?tp=szvNzIafmi8&si=AF!AF&sh=cOPgvA&ss=FvbcqF0UpwCLF9DmtrKFoQ!sdtd67e6beSBe3LKN0dIDA&sc=NQ",
                    "http://foo.bar?tp=szvNzIafmi8&si=AF!AE&sh=5MHyPw&ss=Y0iA3bz-ed9XbUKuAyGfcw!vy5AyYGqaqEMgxcitU_dfA&sc=NA",
                    "http://foo.bar?tp=szvNzIafmi8&si=AF!AD&sh=y_R3Iw&ss=qlPcizBzy0rNQ32eARfePw!N94a5vkb_X9h63fmMA5qzg&sc=Mw",
                    "http://foo.bar?tp=szvNzIafmi8&si=AF!AC&sh=X9ZloA&ss=HJs4lVQgij0f81DfQYTBrg!JmDkFL37lMhIuhxr__nKyg&sc=Mg",
                    "http://foo.bar?tp=szvNzIafmi8&si=AF!AB&sh=DEt2vw&ss=forhBEj5J5laWcPM0YKlzw!nhcAAet2mUeYee8pbMLAVw&sc=MQ",
            };

    Random rnd = new Random();

    public FakeScanner() {
    }

    public String nextItem() {
        //return itemArr[ rnd.nextInt( itemArr.length ) ];
        return itemArr[ i++ ];
    }
}
