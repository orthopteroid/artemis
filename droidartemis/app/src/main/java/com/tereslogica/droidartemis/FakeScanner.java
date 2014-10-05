package com.tereslogica.droidartemis;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://foo.bar?tp=EUAsYRgDPEk&mt=YFXv5y5zmHKeSqhmBA&pk=AAh4ja-DLu2ORvuNpb1Q0RsR&vf=ry28Qw&as=vxNlfhxAUPN-81AR0YuyNw!JBwzNQDYBFxrgzkF9F-3gQ&ai=AF!AF!CI&mc=bWFpbg",
"http://foo.bar?tp=EUAsYRgDPEk&sh=xDNtXw&ss=V5bq6kmk2_UEdhz3TL_ewg!CLLYzY0Ycd-LWYqqDQOTnQ&si=AF!AF!AB&sc=b25l",
"http://foo.bar?sh=xjR6DQ&tp=EUAsYRgDPEk&si=AF!AF!AC&ss=Mlz-U2Mf7qblAWbaClH_WA!pu2DXyOWLwmTg6zDgOC8uA&sc=dHdv",
"http://foo.bar?si=AF!AF!AD&sh=FxMryA&tp=EUAsYRgDPEk&ss=DkI1Qi35hjOK_hhlw9Iguw!QmWTnj8UmxZLbhO1Dz8DSw&sc=dGhyZWU",
"http://foo.bar?sh=agUrNQ&si=AF!AF!AE&ss=p6TZiZ305Nzq46cyhP6iGA!KjgPUY4u7O4asvht9xl5ZQ&tp=EUAsYRgDPEk&sc=Zm91cg",
"http://foo.bar?sh=KOYgkw&tp=EUAsYRgDPEk&ss=sADofp3IaTcJcMPZzq86Dg!hR7m1HdeG5zb0H3y6Ct_EA&si=AF!AF!AF&sc=Zml2ZQ",
"http://foo.bar?mt=LTEcCugljRs1gvg92w&pk=ADZRW9nlDGrXGIxXuYUYrRKv&tp=16UqxPQk-Vk&as=rNRalL19WezruFcF3EPalQ!W1J5FjefcqsIaj8L7josRw&ai=AF!AE!CI&vf=KKkPqg&mc=bWFpbg",
"http://foo.bar?tp=16UqxPQk-Vk&ss=X47HXUrdFALgW3p8gEXxJA!D5dpjxxEtiJBayVA9ssrgA&si=AF!AE!AB&sh=40o_-w&sc=b25l",
"http://foo.bar?ss=ktZOFPx2dXX0q9m9cNzBDw!3Hqc5ob1MJXP_eXUH8XPew&sh=-hhNXQ&si=AF!AE!AC&tp=16UqxPQk-Vk&sc=dHdv",
"http://foo.bar?sh=a12__g&tp=16UqxPQk-Vk&si=AF!AE!AD&ss=IFdeA8WtAzxULW3d1N_Arw!01bUWiwjPRWkuLGF7pGzUA&sc=dGhyZWU",
"http://foo.bar?si=AF!AE!AE&tp=16UqxPQk-Vk&ss=0PXreEI0whW9nJEWrghMLg!OoIlmNl-Ha-g6WRzpaqBAA&sh=LloxbQ&sc=Zm91cg",
"http://foo.bar?sh=b-Uw3A&ss=nHt2MaQyrFYI3uEwxSfg1A!JAOth-Tzb6aQmOGNFKXF6w&tp=16UqxPQk-Vk&si=AF!AE!AF&sc=Zml2ZQ",
"http://foo.bar?mt=_TLbR1eGlQ0FRSd-9ZY&ai=AF!AD!CI&vf=3uzC8g&pk=AM_KDho3f_sD1RenjgufC-iN&tp=niI43gmo-Ok&as=A8tPcX4EVXSE_PlIrWMi-Q!YePOWbhP_65i-W7OHi8L7Q&mc=bWFpbg",
"http://foo.bar?ss=OZVsVoktxbEm1nGoOSPx8w!QTR6fteBiP126CGh5UQgxw&tp=niI43gmo-Ok&si=AF!AD!AB&sh=5VpjDg&sc=b25l",
"http://foo.bar?si=AF!AD!AC&sh=4CYtlg&tp=niI43gmo-Ok&ss=HwdayhDdE5DIuPZGS6a3kw!pDztiCun4py7M9ushcMwYQ&sc=dHdv",
"http://foo.bar?tp=niI43gmo-Ok&si=AF!AD!AD&ss=D3LdeCFBydsy1ZSn24Rd8Q!h0-z_hPH1vWczjP63S0khQ&sh=pxZ1MA&sc=dGhyZWU",
"http://foo.bar?sh=MUK1dQ&tp=niI43gmo-Ok&ss=x6fl7dxWDkJ5AxyMILdHEQ!qgU9bC194w46Rz6BVgh5xQ&si=AF!AD!AE&sc=Zm91cg",
"http://foo.bar?sh=dnLt0w&tp=niI43gmo-Ok&ss=P8Tc_6LAj2wpiSCLf0u-4g!XE41dWwtPn7prYUfvMnoVg&si=AF!AD!AF&sc=Zml2ZQ",
"http://foo.bar?mt=TBvxikcLA-bvZ0tx&pk=ADsQ11z_X9VhVuomlDPbHEC9&tp=H22HUQy5CWU&ai=AF!AC!CI&vf=b7G-mA&as=smlF348J0AUBqojj1c9Q-Q!AK8_enF4p8GgzHmTlaM9LA&mc=bWFpbg",
"http://foo.bar?tp=H22HUQy5CWU&ss=aa35M-2S4WX8r0lrLx2mKg!QGOiGe9ObLE9kMVgozCRDw&si=AF!AC!AB&sh=BwYCiA&sc=b25l",
"http://foo.bar?sh=teV5KA&ss=ANy-MeruvVPetQOYKKiZTw!unWlwxtWz-UfwrK1yRVE5Q&si=AF!AC!AC&tp=H22HUQy5CWU&sc=dHdv",
"http://foo.bar?ss=0V8Rt2oOhsGxYbyGk2JDqg!gngoEB8cBTlsxYXMu4rFGQ&tp=H22HUQy5CWU&sh=209QSA&si=AF!AC!AD&sc=dGhyZWU",
"http://foo.bar?si=AF!AC!AE&ss=fVwcDZGueA_OPTLiTm3Kww!QXqGWPzgEC4IMFMoul-v7g&sh=zD6PdQ&tp=H22HUQy5CWU&sc=Zm91cg",
"http://foo.bar?tp=H22HUQy5CWU&ss=XHr7Ed0oASKxswlaAoIvyg!W-e8oMxHoCKzdOU0e3wNpg&sh=opSmFQ&si=AF!AC!AF&sc=Zml2ZQ"
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
