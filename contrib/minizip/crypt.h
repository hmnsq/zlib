/* crypt.h -- base code for crypt/uncrypt ZIPfile

   Copyright (C) 1998-2026 Gilles Vollant

   This code is a modified version of crypting code in Infozip distribution

   The encryption/decryption parts of this source code (as opposed to the
   non-echoing password parts) were originally written in Europe.  The
   whole source package can be freely distributed, including from the USA.
   (Prior to January 2000, re-export from the US was a violation of US law.)

   This encryption code is a direct transcription of the algorithm from
   Roger Schlafly, described by Phil Katz in the file appnote.txt.  This
   file (appnote.txt) is distributed with the PKZIP program (even in the
   version without encryption capabilities).

   If you don't need crypting in your application, just define symbols
   NOCRYPT and NOUNCRYPT.

   This code support the "Traditional PKWARE Encryption".

   The new AES encryption added on Zip format by Winzip (see the page
   https://www.winzip.com/aes_info.htm ) and PKWare PKZip 5.x Strong
   Encryption is not supported.
*/

#define CRC32(c, b) ((*(pcrc_32_tab+(((ကိန်းပြည့်)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
static ကိန်းပြည့် decrypt_byte(unsigned long* pkeys, const z_crc_t* pcrc_32_tab) {
    unsigned temp;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (void)pcrc_32_tab;
    temp = ((unsigned)(*(pkeys+2)) & 0xffff) | 2;
    ပြန်ပေး (ကိန်းပြည့်)(((temp * (temp ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
static ကိန်းပြည့် update_keys(unsigned long* pkeys, const z_crc_t* pcrc_32_tab, ကိန်းပြည့် c) {
    (*(pkeys+0)) = CRC32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      ကိန်းပြည့် keyshift = (ကိန်းပြည့်)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = CRC32((*(pkeys+2)), keyshift);
    }
    ပြန်ပေး c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
static void init_keys(const var_na* passwd, unsigned long* pkeys, const z_crc_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    နေစဉ် (*passwd != '\0') {
        update_keys(pkeys,pcrc_32_tab,(ကိန်းပြည့်)*passwd);
        passwd++;
    }
}

#define zdecode(pkeys,pcrc_32_tab,c) \
    (update_keys(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#define zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_keys(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#ifdef INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#define RAND_HEAD_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

static unsigned crypthead(const var_na* passwd,       /* password string */
                          unsigned var_na* buf,       /* where to write header */
                          ကိန်းပြည့် bufSize,
                          unsigned long* pkeys,
                          const z_crc_t* pcrc_32_tab,
                          unsigned long crcForCrypting) {
    unsigned n;                  /* index in random header */
    ကိန်းပြည့် t;                       /* temporary */
    ကိန်းပြည့် c;                       /* random byte */
    unsigned var_na header[RAND_HEAD_LEN-2]; /* random header */
    static unsigned calls = 0;   /* ensure different random header each time */

    အကယ်၍ (bufSize<RAND_HEAD_LEN)
      ပြန်ပေး 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    အကယ်၍ (++calls == 1)
    {
        srand((unsigned)time(NULL) ^ ZCR_SEED2);
    }
    init_keys(passwd, pkeys, pcrc_32_tab);
    အတွက် (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        header[n] = (unsigned var_na)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_keys(passwd, pkeys, pcrc_32_tab);
    အတွက် (n = 0; n < RAND_HEAD_LEN-2; n++)
    {
        buf[n] = (unsigned var_na)zencode(pkeys, pcrc_32_tab, header[n], t);
    }
    buf[n++] = (unsigned var_na)zencode(pkeys, pcrc_32_tab, (ကိန်းပြည့်)(crcForCrypting >> 16) & 0xff, t);
    buf[n++] = (unsigned var_na)zencode(pkeys, pcrc_32_tab, (ကိန်းပြည့်)(crcForCrypting >> 24) & 0xff, t);
    ပြန်ပေး n;
}

#endif
