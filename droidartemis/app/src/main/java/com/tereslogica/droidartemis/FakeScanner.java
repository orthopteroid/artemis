package com.tereslogica.droidartemis;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
            {
"http://foo.bar?tp=szvNzIafmi8&ai=AF!AD!CI&vf=Hkb7QA&pk=AZoMKUOUDRZf_dB4okXV-YS4&as=cpWo8SblmAHxfXcziacj4Q!lx10akSXo9zf6MTCypAJ9g&mc=MA&mt=M0IaIYWH",
"http://foo.bar?tp=szvNzIafmi8&si=AF!AF&sh=cOPgvA&sc=NQ&ss=FvbcqF0UpwCLF9DmtrKFoQ!sdtd67e6beSBe3LKN0dIDA",
"http://foo.bar?tp=szvNzIafmi8&si=AF!AE&sh=5MHyPw&sc=NA&ss=Y0iA3bz-ed9XbUKuAyGfcw!vy5AyYGqaqEMgxcitU_dfA",
"http://foo.bar?tp=szvNzIafmi8&si=AF!AD&sh=y_R3Iw&sc=Mw&ss=qlPcizBzy0rNQ32eARfePw!N94a5vkb_X9h63fmMA5qzg",
"http://foo.bar?tp=szvNzIafmi8&si=AF!AC&sh=X9ZloA&sc=Mg&ss=HJs4lVQgij0f81DfQYTBrg!JmDkFL37lMhIuhxr__nKyg",
"http://foo.bar?tp=szvNzIafmi8&si=AF!AB&sh=DEt2vw&sc=MQ&ss=forhBEj5J5laWcPM0YKlzw!nhcAAet2mUeYee8pbMLAVw",
"http://foo.bar?tp=qISsySoncGY&ai=AD!AC!CI&vf=SnBrcA&pk=AAOeox92LcLIQLgCUWobPJGz&as=5zYwGCFraz0Ik84HekoTYw!NvmT2GFMCUT5adoVURi4_w&mc=MDAw&mt=OkYZMITiinA",
"http://foo.bar?tp=qISsySoncGY&si=AD!AB&sh=0BoCDw&ss=bPva6xusL82O4eflw-OICw!o3s7QyGdaUlMxDo6uG4i7A&sc=MTEx",
"http://foo.bar?tp=qISsySoncGY&si=AD!AC&sh=CI-oWg&ss=AtNEqwik0ktfC8obg5B3Qg!pYOBvb3yGYvKpNVFgAc_OA&sc=MjIy",
"http://foo.bar?tp=qISsySoncGY&si=AD!AD&sh=QPzOaQ&ss=aHA7vLVHvH484v-N1WSYCw!Nm86NFU2qzCII32Sk2of5g&sc=MzMz",
"http://foo.bar?tp=9CLZfcQU5t0&ai=AD!AC!CI&vf=xB9Vzw&pk=AAOeox92LcLIQLgCUWobPJGz&as=5zYwGCFrazz7IclWK-53Xw!NvmT2GzeTIG9XfCMgAnLvw&mc=MDAw&mt=OkASaoXpnQKejfKEnQ",
"http://foo.bar?tp=9CLZfcQU5t0&si=AD!AB&sh=0BoCDw&ss=bPva6xusL81HmMjKsULu8A!o3s7Q1727qWMtMmr8cKhfA&sc=MTEx",
"http://foo.bar?tp=9CLZfcQU5t0&si=AD!AC&sh=CI-oWg&ss=AtNEqwik0ktTvr0K7lsnDg!pYOBvcer6U3cAlsTTXny-A&sc=MjIy",
"http://foo.bar?tp=9CLZfcQU5t0&si=AD!AD&sh=QPzOaQ&ss=aHA7vLVHvH4Jn9BLMHHJMQ!Nm86NIFUt21xlQi4yw0dxg&sc=MzMz",
"http://foo.bar?tp=jGVeH7-wpSk&ai=AD!AC!CI&vf=SNMK5w&pk=AAOeox92LcLIQLgCUWobPJGz&as=5zYwGCFraz1YPfvG11oFMw!NvmT2By8cMwqKYp_2cA5_w&mc=MDAw&mt=NEZYNo_mnF6SjLOV9YgWlpvYx0hZ",
"http://foo.bar?tp=jGVeH7-wpSk&si=AD!AB&sh=0BoCDw&ss=bPva6xusL8y-1KGbXIInrw!o3s7Q9SrBLmIIKjYeCYSLA&sc=MTEx",
"http://foo.bar?tp=jGVeH7-wpSk&si=AD!AC&sh=CI-oWg&ss=AtNEqwik0ktAae2nwBvl7A!pYOBvdhO8ixuad40ljwiWA&sc=MjIy",
"http://foo.bar?tp=jGVeH7-wpSk&si=AD!AD&sh=QPzOaQ&ss=aHA7vLVHvH3SbT83_MEemQ!Nm86NLDVvEK1lKbY--dGRg&sc=MzMz",
"http://foo.bar?tp=0i0wT8LtTig&ai=AD!AC!CI&vf=lpSwYw&pk=AAOeox92LcLIQLgCUWobPJGz&as=5zYwGCFraz10r4GUF-AORQ!NvmT2ARB1O1yF_HyasvVnw&mc=MDAw&mt=KEEbIZ7vlhmUzL6O74QK3_Q",
"http://foo.bar?tp=0i0wT8LtTig&si=AD!AB&sh=0BoCDw&ss=bPva6xusL8yc-Xlcvx3rSQ!o3s7Q_HOIRWoGqPU3LPQTA&sc=MTEx",
"http://foo.bar?tp=0i0wT8LtTig&si=AD!AC&sh=CI-oWg&ss=AtNEqwik0ks8DNxsSdmXIA!pYOBvdwQUGsnpg9eLhK-mA&sc=MjIy",
"http://foo.bar?tp=0i0wT8LtTig&si=AD!AD&sh=QPzOaQ&ss=aHA7vLVHvH4L8KmyBYdovQ!Nm86NH9Wf_YO1PiEtJf9hg&sc=MzMz",
"http://foo.bar?tp=9xZVvHapqyE&ai=AF!AD!CI&vf=wLCBLw&pk=AZoMKUOUDRZf_dB4okXV-YS4&as=cpWo8SblmAJOf0QH5I8wig!lx10aiLOUEXVMq2i3NDsxQ&mc=MDAw&mt=PkITNILmjB7z",
"http://foo.bar?tp=9xZVvHapqyE&si=AF!AB&sh=DEt2vw&ss=forhBEj5J5mGnj6QXj5Zdg!nhcAAdth4JO5mTEf_1g2uA&sc=MTEx",
"http://foo.bar?tp=9xZVvHapqyE&si=AF!AC&sh=X9ZloA&ss=HJs4lVQgijxbuL6-VJeuBQ!JmDkFQVEKwVZpDIqB9bm-w&sc=MjIy",
"http://foo.bar?tp=9xZVvHapqyE&si=AF!AD&sh=y_R3Iw&ss=qlPcizBzy0oCSI6GGSUZIQ!N94a50LYb2yE_kUHKmqOXA&sc=MzMz",
"http://foo.bar?tp=9xZVvHapqyE&si=AF!AE&sh=5MHyPw&ss=Y0iA3bz-ed-fkr26X3i0fQ!vy5AyWd1DfLOT4VKUnbjog&sc=NDQ0",
"http://foo.bar?tp=9xZVvHapqyE&si=AF!AF&sh=cOPgvA&ss=FvbcqF0UpwEcSO4kVQWwdw!sdtd64L8GR8JWH-FXzcrBg&sc=NTU1",
"http://foo.bar?tp=89UiHLfwyIc&ai=AF!AD!CI&vf=ew22pg&pk=AZoMKUOUDRZf_dB4okXV-YS4&as=cpWo8SblmAIxYKY042m2Pw!lx10ai1iVV0p7uYynicfKA&mc=MDAw&mt=ME8YI4v1lx_z",
"http://foo.bar?tp=89UiHLfwyIc&si=AF!AB&sh=DEt2vw&ss=forhBEj5J5lgFi5AautTlA!nhcAAelhK8b7Etx_kbX2Kg&sc=MTEx",
"http://foo.bar?tp=89UiHLfwyIc&si=AF!AC&sh=X9ZloA&ss=HJs4lVQgij0tCMEdA3wCLA!JmDkFLk61sx2bgJoBaoY3A&sc=MjIy",
"http://foo.bar?tp=89UiHLfwyIc&si=AF!AD&sh=y_R3Iw&ss=qlPcizBzy0pYXNWSZibnZA!N94a5yOTXzDNWHQTfwUyQQ&sc=MzMz",
"http://foo.bar?tp=89UiHLfwyIc&si=AF!AE&sh=5MHyPw&ss=Y0iA3bz-ed-es8v5w_21Mg!vy5AyWfGCvzaOVeo6y0DBQ&sc=NDQ0",
"http://foo.bar?tp=89UiHLfwyIc&si=AF!AF&sh=cOPgvA&ss=FvbcqF0UpwCk7O8x3rYNTA!sdtd665YGSskglgt3rDlSQ&sc=NTU1",
"http://foo.bar?tp=iW3HlmUOeto&ai=AF!AF!CI&vf=QGAWFQ&pk=AQiVPVZFqbcqDXYFzyXcHeKo&as=forhBEj5J5knT0nEqPfO1A!MuqrgAfItFEp4Vti_dzg-A&mc=MDAw&mt=Nl0DK4-H",
"http://foo.bar?tp=iW3HlmUOeto&si=AF!AB&sh=xP5FHg&ss=HJs4lVQgijy797EmzNoyoA!3LZzzEMIfXbgVP8J9etC1g&sc=MTEx",
"http://foo.bar?tp=iW3HlmUOeto&si=AF!AC&sh=MKk39A&ss=qlPcizBzy0ntpjhs_MAbVA!PZk01xlgLLhUwQT-LebTgg&sc=MjIy",
"http://foo.bar?tp=iW3HlmUOeto&si=AF!AD&sh=uQZAfQ&ss=Y0iA3bz-ed8NeN-dtqIzIA!wFu5gl7ECBJKy8XHuou85Q&sc=MzMz",
"http://foo.bar?tp=iW3HlmUOeto&si=AF!AE&sh=5TGp0g&ss=FvbcqF0UpwDXdTs1_hmTtw!lFtgBPBkkRSE0njVXoaJQg&sc=NDQ0",
"http://foo.bar?tp=iW3HlmUOeto&si=AF!AF&sh=fTZhLQ&ss=FlHpf6e49omaNbq-QGYRNw!EKjPCD4EqxxOTcuIBuTPDw&sc=NTU1",
            };

    Random rnd = new Random();

    public FakeScanner() {
    }

    public String nextItem() {
        return itemArr[ rnd.nextInt( itemArr.length ) ];
        //return itemArr[ i++ ];
    }
}
