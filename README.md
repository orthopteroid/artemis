Older code for a library, a console app and the old-api android app Arcanacard (now defunct).

The C99 library and console app is designed to natively compile on Windows and Linux.

The java android app talks to the NDK-built library via jni.

URL particulars (laid out in libartemis/src/ar_code.c) are:

```
// Artemis A uri is http://<location>?tp=<auth.pubkey>&ai=<info>&as=<authsig.r>|<authsig.r>&mt=<messagetext>[&mc=<messageclue>]
//
// Artemis S uri is http://<location>?tp=<auth.pubkey>&si=<info>&ss=<sharesig.r>|<sharesig.s>&sh=<shareid>|<share>[&sc=<shareclue>]
//
// where:
// <info> == <version>|<shares>|<threshold>

// artemis
//  0 url location L is specified as an identifying tag and as a url path
//  1 version, numshares, threshold are combined into info I
//  2 random cipher key C is used on message M to crypt message (producing M')
//  3 split C into shares {C'1...C'n} as C'i
//  4 random private key Kpri is used to create a matching public key Kpub
//  5 private key Kpri is applied to hash of Kpub | I to produce authsignature Sa
//  6 private key Kpri is applied to hash of Kpub | I | { 1, ... i, ... n } | { C'1, ... C'i, ... C'n } to produce sharesignatures {S1...Sn}
//  7 private key Kpri and cipher key C are thrown away
//  8 authentication is distributed in the concatenated form L | Kpub | I | Sa | M' | Mc
//  9 shares are distributed in the concatenated form      { L | Kpub | I | Si | i | C'i | Sc }
```

The encryption configuration is laid out in libartemis/inc/version.h but to summarize: With symmetric key length 56bits and ECDSA limited to 112bits, this software likley falls under classification 5x992 and can be exported without encryption registration. Supposedly. pegwit uses a Koblitz curve of the form: y^2 + xy = x^3 + b. Koblitz curves are weaker as their field is nonprime and can apparently be factored into smaller attackable fields.

At the time of that writing this was closed source but based upon publicly released pegwit sources and with the short (112bit) keylength meant it was considered exempt from Wassenaar Cat. 5 encryption controls.
