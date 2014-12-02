package com.tereslogica.arcanacard;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&ai=BAAUF&as=aq9ddtKF07j-crGV!qj62VpTAfTyGk3yP&mt=ZZoc307xgXukLyJocQ&mc=bWFpbg",
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&si=BAAUF&ss=QXFk0NPuf1IOkJjh!TOfftc_3pSQeWjQJ&sh=AAE!AD46kBI-OpA&sc=b25l",
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&si=BAAUF&ss=ux0nB2699C3-1kSw!E07cAU6j8tiZug0o&sh=AAI!CkoqmElKN-c&sc=dHdv",
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&si=BAAUF&ss=k4rNvZa4f9XHYzzF!Z5byp8uirMhcHPx2&sh=AAM!CRIadgYZ1P0&sc=dGhyZWU",
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&si=BAAUF&ss=20B4gku4U3pmla-Z!7JOyJb6KN6Ax4LrF&sh=AAQ!45ziomZ4fsU&sc=Zm91cg",
"http://arcanacard.com?tp=AAFmSGb__nwouVMDdYN1Kg&si=BAAUF&ss=7TF_VRTdddMMVS7z!y11X0IYkYEgxRNSU&sh=AAU!6UZIBl0Ncwg&sc=Zml2ZQ",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&ai=BAAUE&as=QguL9ZB8okBLefJt!CU6gsvyduDWsZi7J&mt=qyFmsslkbpzVdiGs2g&mc=bWFpbg",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&si=BAAUE&ss=PHK5YjioS9w1xiOa!1R6XG7C59MhZ-SKA&sh=AAE!AGN2uTjg0EA&sc=b25l",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&si=BAAUE&ss=DLzGzlxMrRbRKJod!HLoVeqNCIZ49u15F&sh=AAI!A4TBmXNeBR8&sc=dHdv",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&si=BAAUE&ss=el7Qq6VVB8Le5076!8dKeVjEFXOlNZVwR&sh=AAM!Amlom2H76FE&sc=dGhyZWU",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&si=BAAUE&ss=U3qcUxyzl8cBEyLN!1DekAQ9JLjhzUmZa&sh=AAQ!FABn0dgoLhE&sc=Zm91cg",
"http://arcanacard.com?tp=AAHoqiB9iXsWiP_KcecuWw&si=BAAUE&ss=fHorsKGWt0I5SYWN!UUEKalpi9aj5LLWH&sh=AAU!EX9AYrreoZQ&sc=Zml2ZQ",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&ai=BAAUD&as=jMxizZLOcwg9Eizb!UABX8hAXIB0nMzcV&mt=9uaZF75EMfjB1khR9U8&mc=bWFpbg",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUD&ss=iaw08DueyjV6uJ2j!KwBa1erHTWNNwlmM&sh=AAE!AM3z9WeIhTA&sc=b25l",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUD&ss=YzxjuvXneHRVLG-r!y-4r44sP6ExVEKRv&sh=AAI!AQq6XQULtIA&sc=dHdv",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUD&ss=D967blr1X0IG7RuJ!QeLO9_MIG5e2hZDq&sh=AAM!AcEhAmO44EU&sc=dGhyZWU",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUD&ss=LF6DGQkotLYXFvB_!2DlhOd3bDKkZvV8g&sh=AAQ!AHZcjix0KJQ&sc=Zm91cg",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUD&ss=7aNEnBPn3oZKtsuH!DMo5TFJTaTXr7P7O&sh=AAU!AL3H0UrHfFE&sc=Zml2ZQ",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&ai=BAAUC&as=-MViKhQoSq_M-QlV!1X1WFn6wCAvF07Vp&mt=EOpX2F6atRwFgKHs&mc=bWFpbg",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUC&ss=0sgn9mH_bX6nKsMQ!bIfNyOoC6l84DrOv&sh=AAE!AJk12Z4hFDc&sc=b25l",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUC&ss=sigZnlizo3OoG-n1!9UWCwf3JTAw_iWBl&sh=AAI!AbngP90U9_Y&sc=dHdv",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUC&ss=o1gSjZ3FHd1EWUme!na-t5Q4-iePJkvln&sh=AAM!AVmsnRwcqUk&sc=dGhyZWU",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUC&ss=Krb9ibFBw8DmbpSF!3kxMJAMANQ4sM-vF&sh=AAQ!A_lL3lt-MFk&sc=Zm91cg",
"http://arcanacard.com?tp=AAFOZbNxm4Q3M9aAFU386Q&si=BAAUC&ss=K582gH7D9WlEQqky!xpm2Ag1KJ07OC_wZ&sh=AAU!AxkHfJp2buY&sc=Zml2ZQ"
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
