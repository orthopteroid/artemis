package com.tereslogica.droidartemis;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://arcanashare.webhop.net?ai=BAAUF&as=A9Q-6Krr1GPldUie8XQ8Qg!E6GxuSYFGEtquXviJ5VmAg&pk=ARYXc7koEe9--RhHhtSmm3-p&mt=DvpOfN0YtyN3mLnxkg&vf=ry28Qw&tp=jgZPptXS0L0&mc=bWFpbg",
"http://arcanashare.webhop.net?tp=jgZPptXS0L0&ss=n1qvr3VJv8RepL7tsgdU9w!t9FAndSyOJV15E01-1VXrA&sh=AAE!bs9cYOWfa0lyYwyT&si=BAAUF&sc=b25l",
"http://arcanashare.webhop.net?ss=i0sEM6ohfTmCnbK2FZ7MQQ!vtfiZ11cfOenbf645UVsWg&si=BAAUF&tp=jgZPptXS0L0&sh=AAI!-I9uWTKnSQ9MnoQy&sc=dHdv",
"http://arcanashare.webhop.net?sh=AAM!QfbBjKlZHHC9eqqY&si=BAAUF&tp=jgZPptXS0L0&ss=pypAUvigdukq9DDxaQyJoA!dVyvkRAY2UOaMe7USztmQA&sc=dGhyZWU",
"http://arcanashare.webhop.net?si=BAAUF&ss=F9waIzJqGO4R2iOMcAGKeQ!vtCnE-WG2lNbiSmheB7vYA&sh=AAQ!BrysxzPxSfaCI8-Z&tp=jgZPptXS0L0&sc=Zm91cg",
"http://arcanashare.webhop.net?si=BAAUF&sh=AAU!6gV8FUt6JJvvAKkv&ss=gLlgNmYJbJU2MqBIi4Z6JQ!D8yDKU29Ts9pDyVWoOl3GQ&tp=jgZPptXS0L0&sc=Zml2ZQ",
"http://arcanashare.webhop.net?ai=BAAUE&vf=KKkPqg&tp=47fDG40J1KA&pk=AWfHZjj4B4TkpqZFMBX3156V&as=GEkV3K-PiLASzf9tRq72Eg!xjVabH413q1E64dOfCl2Cg&mt=qbe2pr3GMiMz9j8CaA&mc=bWFpbg",
"http://arcanashare.webhop.net?sh=AAE!XnXEb557MpboJo1T&tp=47fDG40J1KA&ss=dFfx93qoFkVnK_DzxuUm1g!wF3yswJxOp_7tdXV45J0Zg&si=BAAUE&sc=b25l",
"http://arcanashare.webhop.net?tp=47fDG40J1KA&ss=Mym2rrlPRXBr4tt-ozoG2w!ck4JWYOXVJh_6u4tvTR9rQ&sh=AAI!eGRCLkgIlfqsh0Qb&si=BAAUE&sc=dHdv",
"http://arcanashare.webhop.net?sh=AAM!mxKdl9uGzLUXfHEK&ss=otG-bBkPxdhMsw9UHe4fFw!WozLSrx_jhN3O4i_XrNWHQ&si=BAAUE&tp=47fDG40J1KA&sc=dGhyZWU",
"http://arcanashare.webhop.net?tp=47fDG40J1KA&ss=zerSjXO9Jzb7DHIrxJHxMw!bqA6-IgXF0R1SFvFT6PH_g&sh=AAQ!rmRsGsfk37t3Kccx&si=BAAUE&sc=Zm91cg",
"http://arcanashare.webhop.net?si=BAAUE&sh=AAU!ipOxQ3WUnmS89JPb&tp=47fDG40J1KA&ss=htap3PwTwnwkI4rnA08BVg!CabNz0vL_v41ztZUYV-sjA&sc=Zml2ZQ",
"http://arcanashare.webhop.net?pk=AeTdPWVoirVwP7vJRXLV_KU-&tp=qCcqJdwWtlA&mt=Yj_9XfDOPaXuOYp4ZFo&as=MlRkggRlOhYXLyRg4tRpzQ!A2vuARbytCrkzVuLZf-xQA&vf=3uzC8g&ai=BAAUD&mc=bWFpbg",
"http://arcanashare.webhop.net?sh=AAE!eCwTJYd9ITlnTJ7S&ss=5GrJhY9CYdR1BHmVwkb6yA!KrQCA-Fe9FV6EeI0NrtIeA&si=BAAUD&tp=qCcqJdwWtlA&sc=b25l",
"http://arcanashare.webhop.net?tp=qCcqJdwWtlA&sh=AAI!fb3hzevipla_WWY9&ss=JEue2SzBzmKYNHZCeqQQmw!kNYuw7cMwXW_1cIu_3F2Ow&si=BAAUD&sc=dHdv",
"http://arcanashare.webhop.net?tp=qCcqJdwWtlA&si=BAAUD&ss=xd5Vlb5uT3gYg33sRwmg8A!Pucih-kJU9fr7NaHyRyaLQ&sh=AAM!bmtffEj9_i7HTlOY&sc=dGhyZWU",
"http://arcanashare.webhop.net?ss=NCl9KrpiqkFjBUiuYqiT-Q!h8IA-hJcvWZUdI-B-LxPsA&tp=qCcqJdwWtlA&si=BAAUD&sh=AAQ!h4uBvs29e9Qf74qX&sc=Zm91cg",
"http://arcanashare.webhop.net?tp=qCcqJdwWtlA&ss=Dqv9LoEBTfeHdUbwI3pB7A!kU42Zubp4FwQMfWMabH6DA&sh=AAU!lF0_D26iI6xn-L8y&si=BAAUD&sc=Zml2ZQ",
"http://arcanashare.webhop.net?tp=YBiI1irafoU&pk=AFAqvSYMuLHO3LDw_06TgtGU&vf=b7G-mA&mt=IeyhVOJ8ffHFnp_r&ai=BAAUC&as=kEJxk3Q0ohgEyiXAq0r0sQ!dw0N53iw_1rpKahnZoZIug&mc=bWFpbg",
"http://arcanashare.webhop.net?sh=AAE!gyIkXrBVXVJoEW2W&ss=JH2v2-LYQKC6a6pqfaqkFw!apmt5Y-PiE6_eF7vxdZ54Q&si=BAAUC&tp=YBiI1irafoU&sc=b25l",
"http://arcanashare.webhop.net?tp=YBiI1irafoU&ss=hpFFXnLEfIpURGI7Kf9q3g!41sBOD27xgdMh_Dxqe7LNA&sh=AAI!rRAC-PDqW3BkOxtz&si=BAAUC&sc=dHdv",
"http://arcanashare.webhop.net?tp=YBiI1irafoU&sh=AAM!t_Xrmjt0WW5g1sLb&ss=wr2Z4n4x07fUNdPcTpoppw!r9ceJfwGb3HK_iSZ8LZk2g&si=BAAUC&sc=dGhyZWU",
"http://arcanashare.webhop.net?ss=w5xF2iX8pzCXQosVUa5EUg!a8y7uxyCwzboilELVloZBg&si=BAAUC&sh=AAQ!8XROqXCJVzR8b_ek&tp=YBiI1irafoU&sc=Zm91cg",
"http://arcanashare.webhop.net?si=BAAUC&sh=AAU!65Gny7sXVSp4gi4M&tp=YBiI1irafoU&ss=rdypcDtjISUWdWWDcgIJlA!gTgti4HxoBmIVF4JhTgNQg&sc=Zml2ZQ"
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
