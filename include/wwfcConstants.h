#ifndef WWFC_CONSTANTS
#define WWFC_CONSTANTS

#define WWFC_SHA256_DIGEST_SIZE 32

#define WWFC_TITLE_TYPE_DISC 0
#define WWFC_TITLE_TYPE_NAND 1

#ifndef WWFC_DOMAIN
#  ifdef PROD
#    define WWFC_DOMAIN "wiilink.ca"
#  else
#    define WWFC_DOMAIN "nwfc.wiinoma.com" // Points to localhost
#  endif
#endif

#endif // WWFC_CONSTANTS