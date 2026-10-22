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

#परिभाषय चक्रीयपरीक्षा_32(c, b) ((*(pcrc_32_tab+(((पूर्णाङ्क)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
स्थिर पूर्णाङ्क decrypt_byte(अचिह्नित दीर्घ* pkeys, ध्रुव z_crc_t* pcrc_32_tab) {
    अचिह्नित अस्थायी;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (शून्य)pcrc_32_tab;
    अस्थायी = ((अचिह्नित)(*(pkeys+2)) & 0xffff) | 2;
    प्रत्यागम (पूर्णाङ्क)(((अस्थायी * (अस्थायी ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
स्थिर पूर्णाङ्क update_कुञ्चिकाः(अचिह्नित दीर्घ* pkeys, ध्रुव z_crc_t* pcrc_32_tab, पूर्णाङ्क c) {
    (*(pkeys+0)) = चक्रीयपरीक्षा_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      पूर्णाङ्क keyshift = (पूर्णाङ्क)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = चक्रीयपरीक्षा_32((*(pkeys+2)), keyshift);
    }
    प्रत्यागम c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
स्थिर शून्य init_कुञ्चिकाः(ध्रुव वर्ण* passwd, अचिह्नित दीर्घ* pkeys, ध्रुव z_crc_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    यावत् (*passwd != '\0') {
        update_कुञ्चिकाः(pkeys,pcrc_32_tab,(पूर्णाङ्क)*passwd);
        passwd++;
    }
}

#परिभाषय zdecode(pkeys,pcrc_32_tab,c) \
    (update_कुञ्चिकाः(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#परिभाषय zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_कुञ्चिकाः(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#यदि_परिभाषितम् INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#परिभाषय RAND_शीर्ष_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

स्थिर अचिह्नित crypthead(ध्रुव वर्ण* passwd,       /* password string */
                          अचिह्नित वर्ण* buf,       /* where to write header */
                          पूर्णाङ्क buf_आकार,
                          अचिह्नित दीर्घ* pkeys,
                          ध्रुव z_crc_t* pcrc_32_tab,
                          अचिह्नित दीर्घ crcForCrypting) {
    अचिह्नित n;                  /* index in random header */
    पूर्णाङ्क t;                       /* temporary */
    पूर्णाङ्क c;                       /* random byte */
    अचिह्नित वर्ण शीर्षकम्[RAND_शीर्ष_LEN-2]; /* random header */
    स्थिर अचिह्नित calls = 0;   /* ensure different random header each time */

    यदि (buf_आकार<RAND_शीर्ष_LEN)
      प्रत्यागम 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    यदि (++calls == 1)
    {
        बीजं_रोपय((अचिह्नित)time(रिक्त) ^ ZCR_SEED2);
    }
    init_कुञ्चिकाः(passwd, pkeys, pcrc_32_tab);
    प्रति (n = 0; n < RAND_शीर्ष_LEN-2; n++)
    {
        c = (यादृच्छिकम्() >> 7) & 0xff;
        शीर्षकम्[n] = (अचिह्नित वर्ण)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_कुञ्चिकाः(passwd, pkeys, pcrc_32_tab);
    प्रति (n = 0; n < RAND_शीर्ष_LEN-2; n++)
    {
        buf[n] = (अचिह्नित वर्ण)zencode(pkeys, pcrc_32_tab, शीर्षकम्[n], t);
    }
    buf[n++] = (अचिह्नित वर्ण)zencode(pkeys, pcrc_32_tab, (पूर्णाङ्क)(crcForCrypting >> 16) & 0xff, t);
    buf[n++] = (अचिह्नित वर्ण)zencode(pkeys, pcrc_32_tab, (पूर्णाङ्क)(crcForCrypting >> 24) & 0xff, t);
    प्रत्यागम n;
}

#यदि_अन्तः
