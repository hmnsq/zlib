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

#ವ್ಯಾಖ್ಯಾನಿಸು ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32(c, b) ((*(pcrc_32_tab+(((ಪೂರ್ಣಾಂಕ)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ decrypt_byte(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* pkeys, ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab) {
    ಚಿಹ್ನೆರಹಿತ ತಾತ್ಕಾಲಿಕ;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (ಶೂನ್ಯ)pcrc_32_tab;
    ತಾತ್ಕಾಲಿಕ = ((ಚಿಹ್ನೆರಹಿತ)(*(pkeys+2)) & 0xffff) | 2;
    ಹಿಂತಿರುಗಿ (ಪೂರ್ಣಾಂಕ)(((ತಾತ್ಕಾಲಿಕ * (ತಾತ್ಕಾಲಿಕ ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
ಸ್ಥಿರ ಪೂರ್ಣಾಂಕ update_ಕೀಲಿಗಳು(ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* pkeys, ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab, ಪೂರ್ಣಾಂಕ c) {
    (*(pkeys+0)) = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      ಪೂರ್ಣಾಂಕ keyshift = (ಪೂರ್ಣಾಂಕ)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_32((*(pkeys+2)), keyshift);
    }
    ಹಿಂತಿರುಗಿ c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
ಸ್ಥಿರ ಶೂನ್ಯ init_ಕೀಲಿಗಳು(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* passwd, ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* pkeys, ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    ಆಗ (*passwd != '\0') {
        update_ಕೀಲಿಗಳು(pkeys,pcrc_32_tab,(ಪೂರ್ಣಾಂಕ)*passwd);
        passwd++;
    }
}

#ವ್ಯಾಖ್ಯಾನಿಸು zdecode(pkeys,pcrc_32_tab,c) \
    (update_ಕೀಲಿಗಳು(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#ವ್ಯಾಖ್ಯಾನಿಸು zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_ಕೀಲಿಗಳು(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#ವ್ಯಾಖ್ಯಾನಿಸು RAND_ತಲೆ_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ crypthead(ಸ್ಥಿರಾಂಕ ಅಕ್ಷರ* passwd,       /* password string */
                          ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ* buf,       /* where to write header */
                          ಪೂರ್ಣಾಂಕ buf_ಗಾತ್ರ,
                          ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ* pkeys,
                          ಸ್ಥಿರಾಂಕ z_ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_t* pcrc_32_tab,
                          ಚಿಹ್ನೆರಹಿತ ದೀರ್ಘ ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting) {
    ಚಿಹ್ನೆರಹಿತ n;                  /* index in random header */
    ಪೂರ್ಣಾಂಕ t;                       /* temporary */
    ಪೂರ್ಣಾಂಕ c;                       /* random byte */
    ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ ಶೀರ್ಷಿಕೆ[RAND_ತಲೆ_LEN-2]; /* random header */
    ಸ್ಥಿರ ಚಿಹ್ನೆರಹಿತ calls = 0;   /* ensure different random header each time */

    ಆದರೆ (buf_ಗಾತ್ರ<RAND_ತಲೆ_LEN)
      ಹಿಂತಿರುಗಿ 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    ಆದರೆ (++calls == 1)
    {
        srand((ಚಿಹ್ನೆರಹಿತ)time(ಖಾಲಿ) ^ ZCR_SEED2);
    }
    init_ಕೀಲಿಗಳು(passwd, pkeys, pcrc_32_tab);
    ಗಾಗಿ (n = 0; n < RAND_ತಲೆ_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        ಶೀರ್ಷಿಕೆ[n] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_ಕೀಲಿಗಳು(passwd, pkeys, pcrc_32_tab);
    ಗಾಗಿ (n = 0; n < RAND_ತಲೆ_LEN-2; n++)
    {
        buf[n] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zencode(pkeys, pcrc_32_tab, ಶೀರ್ಷಿಕೆ[n], t);
    }
    buf[n++] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zencode(pkeys, pcrc_32_tab, (ಪೂರ್ಣಾಂಕ)(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting >> 16) & 0xff, t);
    buf[n++] = (ಚಿಹ್ನೆರಹಿತ ಅಕ್ಷರ)zencode(pkeys, pcrc_32_tab, (ಪೂರ್ಣಾಂಕ)(ಚಕ್ರೀಯ_ಪರೀಕ್ಷೆ_For_Crypting >> 24) & 0xff, t);
    ಹಿಂತಿರುಗಿ n;
}

#ಆದರೆ_ಅಂತ್ಯ
