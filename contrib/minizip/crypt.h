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

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ ਚੱਕਰੀ_ਜਾਂਚ_32(c, b) ((*(pcrc_32_tab+(((ਪੂਰਨ)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
ਸਥਿਰ ਪੂਰਨ decrypt_byte(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* pkeys, ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਆਰਜ਼ੀ;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (ਖਾਲੀ)pcrc_32_tab;
    ਆਰਜ਼ੀ = ((ਬਿਨਾਨਿਸ਼ਾਨੀ)(*(pkeys+2)) & 0xffff) | 2;
    ਵਾਪਸ (ਪੂਰਨ)(((ਆਰਜ਼ੀ * (ਆਰਜ਼ੀ ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
ਸਥਿਰ ਪੂਰਨ update_ਕੁੰਜੀਆਂ(ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* pkeys, ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab, ਪੂਰਨ c) {
    (*(pkeys+0)) = ਚੱਕਰੀ_ਜਾਂਚ_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      ਪੂਰਨ keyshift = (ਪੂਰਨ)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = ਚੱਕਰੀ_ਜਾਂਚ_32((*(pkeys+2)), keyshift);
    }
    ਵਾਪਸ c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
ਸਥਿਰ ਖਾਲੀ init_ਕੁੰਜੀਆਂ(ਅਚਲ ਅੱਖਰ* passwd, ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* pkeys, ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    ਜਦੋਂ (*passwd != '\0') {
        update_ਕੁੰਜੀਆਂ(pkeys,pcrc_32_tab,(ਪੂਰਨ)*passwd);
        passwd++;
    }
}

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ zdecode(pkeys,pcrc_32_tab,c) \
    (update_ਕੁੰਜੀਆਂ(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_ਕੁੰਜੀਆਂ(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#ਜੇ_ਪਰਿਭਾਸ਼ਿਤ INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#ਪਰਿਭਾਸ਼ਿਤ_ਕਰੋ RAND_ਸਿਰ_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ crypthead(ਅਚਲ ਅੱਖਰ* passwd,       /* password string */
                          ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ* buf,       /* where to write header */
                          ਪੂਰਨ buf_ਮਾਪ,
                          ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ* pkeys,
                          ਅਚਲ z_ਚੱਕਰੀ_ਜਾਂਚ_t* pcrc_32_tab,
                          ਬਿਨਾਨਿਸ਼ਾਨੀ ਲੰਮਾ ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting) {
    ਬਿਨਾਨਿਸ਼ਾਨੀ n;                  /* index in random header */
    ਪੂਰਨ t;                       /* temporary */
    ਪੂਰਨ c;                       /* random byte */
    ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ ਸਿਰਲੇਖ[RAND_ਸਿਰ_LEN-2]; /* random header */
    ਸਥਿਰ ਬਿਨਾਨਿਸ਼ਾਨੀ calls = 0;   /* ensure different random header each time */

    ਜੇ (buf_ਮਾਪ<RAND_ਸਿਰ_LEN)
      ਵਾਪਸ 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    ਜੇ (++calls == 1)
    {
        srand((ਬਿਨਾਨਿਸ਼ਾਨੀ)time(ਨਿਰਾ) ^ ZCR_SEED2);
    }
    init_ਕੁੰਜੀਆਂ(passwd, pkeys, pcrc_32_tab);
    ਲਈ (n = 0; n < RAND_ਸਿਰ_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        ਸਿਰਲੇਖ[n] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_ਕੁੰਜੀਆਂ(passwd, pkeys, pcrc_32_tab);
    ਲਈ (n = 0; n < RAND_ਸਿਰ_LEN-2; n++)
    {
        buf[n] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zencode(pkeys, pcrc_32_tab, ਸਿਰਲੇਖ[n], t);
    }
    buf[n++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zencode(pkeys, pcrc_32_tab, (ਪੂਰਨ)(ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting >> 16) & 0xff, t);
    buf[n++] = (ਬਿਨਾਨਿਸ਼ਾਨੀ ਅੱਖਰ)zencode(pkeys, pcrc_32_tab, (ਪੂਰਨ)(ਚੱਕਰੀ_ਜਾਂਚ_For_Crypting >> 24) & 0xff, t);
    ਵਾਪਸ n;
}

#ਜੇ_ਅੰਤ
