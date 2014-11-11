package com.tereslogica.droidartemis;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://tereslogica.com?pk=AJsmfGxu3ebNXekzS3QSa6Wk&tp=00GPR08ZKJ4&mt=9pVYon7PqsCenvB4Fg&vf=ry28Qw&as=Y50_qAwWzk9D5yTbDUQEPw!oPCIEyk1_KBuff9QTSunaw&ai=BAAUF&mc=bWFpbg",
"http://tereslogica.com?sh=AAE!feD54THS0W9rRWtk&si=BAAUF&ss=GCHS9qJkaQsloWpuJBG-nA!I4htJTt-XMFcWuNjmwO_Dg&tp=00GPR08ZKJ4&sc=b25l",
"http://tereslogica.com?si=BAAUF&tp=00GPR08ZKJ4&ss=d6lzisFFGCSHkVlICsCoVg!ndjwcdp65qcqymPBRZp4ZQ&sh=AAI!cuG_6cL5sm3TBFQI&sc=dHdv",
"http://tereslogica.com?ss=hL6c7L7KwW0PaqlT1BZYyQ!Eh5y-qkGzN6HsufyiMGERA&si=BAAUF&sh=AAM!FfrFJGNB-E1vsnHt&tp=00GPR08ZKJ4&sc=dGhyZWU",
"http://tereslogica.com?si=BAAUF&sh=AAQ!eUztuNklql69iV7O&ss=HL1ObSpXQDvFbh57IfMybA!I2mdi_g2ToYLMVHbW1aF0A&tp=00GPR08ZKJ4&sc=Zm91cg",
"http://tereslogica.com?sh=AAU!KBfILgcstU5lscxY&ss=vm6f_VqU17pwZs1a0uFDpg!yw6t-Fbzfy4tAfNooUVLKg&si=BAAUF&tp=00GPR08ZKJ4&sc=Zml2ZQ",
"http://tereslogica.com?pk=AS1Yzs7Gt2KDMH_Qrf2aU7Sl&vf=KKkPqg&mt=nSRHe2pGrJhs1I8VUQ&as=U1dkoZtyFFdAsxEV_gKEGw!RSG-k80FD5hQmMkhvc0kzg&ai=BAAUE&tp=KBrQuIxSclM&mc=bWFpbg",
"http://tereslogica.com?sh=AAE!9PGIHQeP7-MH0K6M&si=BAAUE&ss=PDOT5KX6BYnfYIYMxYjlnA!OvKuD7KO7sP89iM_jozTlg&tp=KBrQuIxSclM&sc=b25l",
"http://tereslogica.com?sh=AAI!GOXJ1o8KBb0GCWi-&ss=vDerNVWi7G63k2ch3RkuNA!rIUwr2Lox3U_Ive4E0zKSw&si=BAAUE&tp=KBrQuIxSclM&sc=dHdv",
"http://tereslogica.com?tp=KBrQuIxSclM&si=BAAUE&ss=5NBXoYg75C-qV0vkOke3EA!l7AWKXqb86lMH1HqQU0_Rw&sh=AAM!cFf729P_VfzsRcP_&sc=dGhyZWU",
"http://tereslogica.com?sh=AAQ!oPGSOjURzOXiZeLm&si=BAAUE&tp=KBrQuIxSclM&ss=4mqaaaaGW9P0lwUi-Tz7IA!nE9DQG8iszqbxcsYLDj9IA&sc=Zm91cg",
"http://tereslogica.com?tp=KBrQuIxSclM&si=BAAUE&ss=rxTk1Vwqb3zUdnOhe_IT9w!Vom9iWVYF0LUaY8MOPsjQA&sh=AAU!hZAoUtWUhg1NG2ha&sc=Zml2ZQ",
"http://tereslogica.com?mt=zCfpXjiKdIoVZHg7S88&pk=AE75Hl163Q1wbLcQ2Qw0AzGk&as=NXjeINwwIowpCqr_iniL2w!KjFKuLHuEOq6sQqlmzEWFw&vf=3uzC8g&ai=BAAUD&tp=B8g1v0hOwb8&mc=bWFpbg",
"http://tereslogica.com?tp=B8g1v0hOwb8&ss=5KQDqNKGA3AlU4ziF3XC6w!2iXc3PtVLRvWL83oA1Svrw&sh=AAE!tIBxe7AtdZxH9o66&si=BAAUD&sc=b25l",
"http://tereslogica.com?si=BAAUD&ss=vIfyy6WLTeBpufCtrQLrlw!coS9ThtmbxsqiowJ_hw4qg&sh=AAI!mD7iTsZn0bCAVJUJ&tp=B8g1v0hOwb8&sc=dHdv",
"http://tereslogica.com?ss=5nJy1AX5wB1efh5F9ja3LA!xPqF1tBp06NuwnxrWwFtOA&si=BAAUD&tp=B8g1v0hOwb8&sh=AAM!7-9wdQE_ljfmYkCm&sc=dGhyZWU",
"http://tereslogica.com?tp=B8g1v0hOwb8&sh=AAQ!m-gBoYDjQIrXMiNM&ss=j3FYdUI1XKqt2NeBXd-iHA!uYWc4k8BkHqJRUU3Z4JsXg&si=BAAUD&sc=Zm91cg",
"http://tereslogica.com?tp=B8g1v0hOwb8&ss=HfQuEsIT_mtYyI2ctqUsyQ!B4Uo_IQ-Y5c8CRFRVQoPxw&sh=AAU!7DmTmke7Bw2xBPbj&si=BAAUD&sc=Zml2ZQ",
"http://tereslogica.com?pk=AFsp7J_fnjvZWumcyFALPCsP&tp=9l4Q22K9jQA&ai=BAAUC&mt=lCoEGPfhT9PNZYkd&as=BLba4dwORqdFkEgDJ0GaEg!KYwBEMcDXdoY7jtk8PXpBQ&vf=b7G-mA&mc=bWFpbg",
"http://tereslogica.com?si=BAAUC&ss=6rvmL8R3hUngCAhWdxrQtQ!1Lzc2xuc2yXfjZdNsfNi_Q&tp=9l4Q22K9jQA&sh=AAE!HsJat0wKeaOpU1lY&sc=b25l",
"http://tereslogica.com?tp=9l4Q22K9jQA&sh=AAI!kqK-LraKy3bJy56U&ss=dsTaFvcf2fZ0FC_I509n2A!ae2XR9ie__6T_1i3R2XqOw&si=BAAUC&sc=dHdv",
"http://tereslogica.com?si=BAAUC&tp=9l4Q22K9jQA&sh=AAM!HYLiWeABpc7pSCjQ&ss=W2z4oIkbZY17aWdE0eNfNg!EnZze2ZqYp5mpIuPx0lbTA&sc=dGhyZWU",
"http://tereslogica.com?tp=9l4Q22K9jQA&si=BAAUC&ss=AyJmXpvFljLlOnHcVrlP7w!R4iSxTTwy2HuSmaGJlbBXw&sh=AAQ!l2JrAV-XssEJ5g0R&sc=Zm91cg",
"http://tereslogica.com?ss=IHpUPGb93ycqAxoo2HAttw!4EWqqiB1srohRtfEU3RYIA&si=BAAUC&sh=AAU!GEI3dgkc3HkpZbtV&tp=9l4Q22K9jQA&sc=Zml2ZQ"
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
