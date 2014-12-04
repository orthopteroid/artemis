package com.tereslogica.arcanacard;

import java.util.ArrayList;
import java.util.Random;

/**
 * Created by john on 04/12/14.
 */
public class AppAutofill {

    private static String ClipRight( String s, int len ) {
        if( s.length() > len ) {
            return s.substring(0, len);
        } else {
            return s;
        }
    }

    private static String[] SelectClueStrings(String quoteString, int maxClueLength, int numClues) {
        Random rnd = new Random();

        String[] singleWords = quoteString.split(" ");

        int numwords = 2 + (int)((float)singleWords.length / (float)numClues / (float)2);
        if( numwords > singleWords.length ) { numwords = singleWords.length - 1; }
        if( numwords <= 0 ) { numwords = 1; } // eek!

        // fill clues
        ArrayList<String> arrl = new ArrayList<String>();
        for( int c=0; c<numClues; c++) {
            StringBuilder sb = new StringBuilder();
            int startingword = rnd.nextInt( singleWords.length - numwords +1 );
            for( int w=0; w<numwords; w++ ) {
                if( w > 0 ) { sb.append( ' ' ); }
                sb.append( singleWords[ startingword++ ] );
            }
            String clue = sb.toString();
            if( clue.length() > maxClueLength ) { clue = clue.substring( 0, maxClueLength -1 ); }
            arrl.add( clue );
        }
        return arrl.toArray(new String[arrl.size()]);
    }

    private static String[] EnumCategories( String[] db ) {
        ArrayList<String> arrl = new ArrayList<String>();
        int i = 0;
        while( i < db.length ) {
            arrl.add( db[i] );
            do { i += 3; } while( i < db.length && db[i] == "*" ); // find next category
        }
        return arrl.toArray(new String[arrl.size()]);
    }

    private static class PassageClueUple {
        public String[] passages;
        public String[] clues;
        public PassageClueUple(String[] _p, String[] _c) { passages=_p; clues=_c; }
    }

    private static PassageClueUple EnumPassages( String[] db, String cat ) {
        ArrayList<String> parr = new ArrayList<String>();
        ArrayList<String> carr = new ArrayList<String>();
        int i = 0;
        while( i < db.length && db[i] != cat ) { i+=3; }
        if( i < db.length ) {
            do {
                parr.add( db[++i] );
                carr.add( db[++i] );
                ++i;
            } while( i < db.length && db[i] == "*" ); // deal with all in same category
        }
        return new PassageClueUple( parr.toArray(new String[parr.size()]), (carr.toArray(new String[parr.size()]) ) );
    }

    //////////////////////

    public static class Scenario {
        public String m;
        public String l;
        public String[] k;
        public Scenario( String _m, String _l, String[] _k ) { m=_m; l=_l; k=_k; }

        public static Scenario MakeRndScenario( String[] db, int maxlen, int keycount ) {
            String lockClue = "";
            Random r = new Random();

            String[] catarr = EnumCategories( db );
            String cat = catarr[ r.nextInt( catarr.length ) ];
            PassageClueUple pcs = EnumPassages( db, cat );
            int passint = r.nextInt( pcs.passages.length );
            String pass = pcs.passages[ passint ];
            String clue = pcs.clues[ passint ];
            String[] keyclues = SelectClueStrings(pass, maxlen, keycount);
            return new Scenario( ClipRight(cat, maxlen), ClipRight(clue, maxlen), keyclues );
        }
    }

    public static class AuthorQuoteUple {
        ArrayList<String> authors = new ArrayList<String>();
        ArrayList<String[]> quotes = new ArrayList<String[]>();
        AuthorQuoteUple() {}
        public void add( String a, String q[] ) { authors.add(a); quotes.add(q); }
    }

    public static AuthorQuoteUple EnumQuotes() {
        AuthorQuoteUple quoteset = new AuthorQuoteUple();
        quoteset.add( ConstShakespeare.author, ConstShakespeare.quotes );
        quoteset.add( ConstJulesVerne.author, ConstJulesVerne.quotes );
        quoteset.add(ConstJulesVerneFR.author, ConstJulesVerneFR.quotes);
        quoteset.add( ConstBeatrixPotter.author, ConstBeatrixPotter.quotes );
        return quoteset;
    }

}
