package com.tereslogica.droidartemis;

import java.util.Random;

public class FakeScanner {

    int i  = 0;

    String[] itemArr =
    {
"http://foo.bar?as=zynyoIxzH7CpRDpf_GOLpw!rCAngAaoJ7781C9wT5zIWQ&tp=scGzOtjBmvk&mt=0lpUwwqK212lDcNpBg&vf=ry28Qw&ai=BAAUF&pk=Ac8ihVLV36FYI80Kn5aVcpo_&mc=bWFpbg",
"http://foo.bar?tp=scGzOtjBmvk&si=BAAUF&sh=AAE!4wheofHLkACXQvpy&ss=4sBx7fcyyeoEAci4QyqwFg!bHqRq_Y54RN8isil6peE9A&sc=b25l",
"http://foo.bar?sh=AAI!xQENLV6gAkxZJxyc&tp=scGzOtjBmvk&si=BAAUF&ss=fN0iT1ZCXz_f5P0GLspq4A!qhUfwPhE2azGebUXCAe26Q&sc=dHdv",
"http://foo.bar?ss=lPKIswuw7SMvVMdhBUjDJQ!rPeLGCa1DtPM5UpMgj0ddw&si=BAAUF&tp=scGzOtjBmvk&sh=AAM!3GvVgdwnTSi8aWwk&sc=dGhyZWU",
"http://foo.bar?si=BAAUF&tp=scGzOtjBmvk&sh=AAQ!tLEKggDAs8Q3BNXH&ss=AqIO3rVnLKU4yvbMzMJvag!WiJjbLIGYzF2bZipqyTCRA&sc=Zm91cg",
"http://foo.bar?tp=scGzOtjBmvk&sh=AAU!3ZpfUXAoI825nfC6&si=BAAUF&ss=ZBV3aywOdl16q2nvdIRhwA!FMtL0VqmETWA4vJmOEPhIg&sc=Zml2ZQ",
"http://foo.bar?tp=7WgTMrc45CM&ai=BAAUE&pk=Abq-pZNOWU1KT8zB1DqoKB2y&vf=KKkPqg&as=KF4R52fKBnsqpAETdzOg9A!eswOaYZycuf6PrxpJHXXxw&mt=NugbVkURius1ZpU31Q&mc=bWFpbg",
"http://foo.bar?si=BAAUE&ss=rKsJew4HBxH42ly1sv0nXQ!WxYb6-iaD4U6Ex2ciFsBbQ&sh=AAE!5eqLAimLgeEdlTtw&tp=7WgTMrc45CM&sc=b25l",
"http://foo.bar?si=BAAUE&sh=AAI!JbC9pbCBUZ8GRYfn&ss=rbLrN-d-dxJOUwDCwaXoMQ!C75SefFqkSiDt0gc_-O0XA&tp=7WgTMrc45CM&sc=dHdv",
"http://foo.bar?sh=AAM!nur0ymfuiUkABSps&si=BAAUE&ss=mrV80XSqPyxl2lTe2LnFzw!bCsaHymTyJFzOfCBbbkkyw&tp=7WgTMrc45CM&sc=dGhyZWU",
"http://foo.bar?si=BAAUE&ss=OurGLpudXEs7RDd1htagQA!6b2chtR1KjNM_P9maWL1jQ&tp=7WgTMrc45CM&sh=AAQ!8q18fAnZlI2pi9wA&sc=Zm91cg",
"http://foo.bar?si=BAAUE&ss=aSoFAhEMdwJj6VhXtBkXWw!gfRj4S5qy70QKq4-KQNN7w&tp=7WgTMrc45CM&sh=AAU!6ZWofg1I8XSTZnhT&sc=Zml2ZQ",
"http://foo.bar?as=tddRClEP4_PKFPSgewem8A!7TJF-sR9XJVMkLCbjLguCA&pk=AKs1gipw2JSV1XMtv9Wrs-8N&ai=BAAUD&mt=WWmahkVal9IQl6w0quc&tp=H69bNl3T6gM&vf=3uzC8g&mc=bWFpbg",
"http://foo.bar?ss=tAcReizZrp7ekJtCMIJ9BA!1MboqELNdaPac65Yf990mQ&si=BAAUD&tp=H69bNl3T6gM&sh=AAE!YIBvkq98boVFr7ZN&sc=b25l",
"http://foo.bar?si=BAAUD&sh=AAI!6JLcLI3W729E4KA6&ss=wRGfY0TqLzc5GZAVRaHySg!XTSmC-FvD2w4W94REVGNMw&tp=H69bNl3T6gM&sc=dHdv",
"http://foo.bar?tp=H69bNl3T6gM&sh=AAM!FC1nslku9iSbwMId&si=BAAUD&ss=6PHZBFoH30YPQ5MMMYXX7g!1-PY_l-DOVQz1n3VYSLekg&sc=dGhyZWU",
"http://foo.bar?tp=H69bNl3T6gM&sh=AAQ!CmdVSJvmyE3S7cay&ss=DIjLeZpD26y6Z67PKTK7Mw!tNbDIdxDU7t6xOrt6oceNA&si=BAAUD&sc=Zm91cg",
"http://foo.bar?si=BAAUD&tp=H69bNl3T6gM&ss=HxvL1-PZ5TmjX6k_5Xbg9g!1-7Bo6cbdI6thGkpBGgKIQ&sh=AAU!9tju1k8e0QYNzaSV&sc=Zml2ZQ",
"http://foo.bar?as=NXolyxiKxN7zufloExBkxg!Tc-QS72bck1xH010nnSmIQ&vf=b7G-mA&tp=UUQER-OfxXc&pk=ANXH_wtVORcBhe0lYSQEOpAE&ai=BAAUC&mt=p5Qv8V8vcoUmW_6p&mc=bWFpbg",
"http://foo.bar?ss=yx4P91HMCCkrUYcUi_XfBg!Z8T6OGBXQhlC5cg02DExFw&sh=AAE!WZDTReQhhyF8gli0&si=BAAUC&tp=UUQER-OfxXc&sc=b25l",
"http://foo.bar?ss=ikIR8ieThzLBhQEdpgC4CA!0pF0AywRIMcFwxSUZeqSNQ&sh=AAI!o69EBRecVc06B8t-&si=BAAUC&tp=UUQER-OfxXc&sc=dHdv",
"http://foo.bar?tp=UUQER-OfxXc&si=BAAUC&sh=AAM!9brCzkb3G2Lzj7o4&ss=DkcIWG7acI4CyMOmzRo6vA!VqL885icHjqVrQWPYXLXOw&sc=dGhyZWU",
"http://foo.bar?ss=bnaDndqzKdu_dTGiEkjVmA!F5cy9DQG6KUB3l-p-Le0sQ&si=BAAUC&tp=UUQER-OfxXc&sh=AAQ!StF3hez77Ai2EPD3&sc=Zm91cg",
"http://foo.bar?ss=OE8c-fX4Lz-054dQ8irBog!o3EX5JUkswOupf8pZXZl4A&si=BAAUC&sh=AAU!HMTxTr2Qoqd_mIGx&tp=UUQER-OfxXc&sc=Zml2ZQ"
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
