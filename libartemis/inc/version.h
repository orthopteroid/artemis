// Copyright 2014 John Howard (orthopteroid@gmail.com)

#include "ec_param.h"

/////////////////////////

/* http://www.wileyrein.com/publications.cfm?sp=articles&id=6211
 * http://www.law.cornell.edu/cfr/text/15/740.17
 * All helpful.
 *
 * http://www.bis.doc.gov/index.php/policy-guidance/encryption/registration
 * http://www.bis.doc.gov/index.php/forms-documents/doc_view/328-flowchart-2
 * With symmetric key length 56bits and ECDSA limited to 112bits, this software
 * likley falls under classification 5x992 and can be exported without
 * encryption registration. Supposedly.
 *
 * pegwit uses a Kolbitz curve of the form: y^2 + xy = x^3 + b. Kolbitz curves
 * are weaker as their field is nonprime and can apparently be factored into
 * smaller attackable fields.
 */

#define AR_MACBYTES			  8 // 64 bis for mac
#define AR_CRYPTKEYBYTES	  7 // 56 bits for rc4 key and shamir polynomial coefs
#define AR_AUTHKEYBYTES		 12 // 96 bits for authentication on GF(2^n), n=112

/////////////////////////

#define AR_LOCSTR	"arcanacard.com"
#define AR_LOCHASH	0x1E

#define AR_VERSION 1
#define AR_DEMO
