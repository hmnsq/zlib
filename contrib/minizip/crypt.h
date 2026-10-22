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

#घोषणा CRC32(c, b) ((*(pcrc_32_tab+(((पूर्णांक)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
अटल पूर्णांक decrypt_byte(अचिन्हित दीर्घ* pkeys, स्थिर z_crc_t* pcrc_32_tab) {
    अचिन्हित अस्थायी;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (व्योम)pcrc_32_tab;
    अस्थायी = ((अचिन्हित)(*(pkeys+2)) & 0xffff) | 2;
    वापस (पूर्णांक)(((अस्थायी * (अस्थायी ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
अटल पूर्णांक update_कुंजियाँ(अचिन्हित दीर्घ* pkeys, स्थिर z_crc_t* pcrc_32_tab, पूर्णांक c) {
    (*(pkeys+0)) = CRC32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      पूर्णांक keyshift = (पूर्णांक)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = CRC32((*(pkeys+2)), keyshift);
    }
    वापस c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
अटल व्योम init_कुंजियाँ(स्थिर अक्षर* passwd, अचिन्हित दीर्घ* pkeys, स्थिर z_crc_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    जबतक (*passwd != '\0') {
        update_कुंजियाँ(pkeys,pcrc_32_tab,(पूर्णांक)*passwd);
        passwd++;
    }
}

#घोषणा zdecode(pkeys,pcrc_32_tab,c) \
    (update_कुंजियाँ(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#घोषणा zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_कुंजियाँ(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#अगर_घोषित INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#घोषणा RAND_शीर्ष_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

अटल अचिन्हित crypthead(स्थिर अक्षर* passwd,       /* password string */
                          अचिन्हित अक्षर* buf,       /* where to write header */
                          पूर्णांक buf_आकार,
                          अचिन्हित दीर्घ* pkeys,
                          स्थिर z_crc_t* pcrc_32_tab,
                          अचिन्हित दीर्घ crcForCrypting) {
    अचिन्हित n;                  /* index in random header */
    पूर्णांक t;                       /* temporary */
    पूर्णांक c;                       /* random byte */
    अचिन्हित अक्षर header[RAND_शीर्ष_LEN-2]; /* random header */
    अटल अचिन्हित calls = 0;   /* ensure different random header each time */

    अगर (buf_आकार<RAND_शीर्ष_LEN)
      वापस 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    अगर (++calls == 1)
    {
        बेक्रम((अचिन्हित)समय(शून्य) ^ ZCR_SEED2);
    }
    init_कुंजियाँ(passwd, pkeys, pcrc_32_tab);
    क्रम (n = 0; n < RAND_शीर्ष_LEN-2; n++)
    {
        c = (अक्रम() >> 7) & 0xff;
        header[n] = (अचिन्हित अक्षर)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_कुंजियाँ(passwd, pkeys, pcrc_32_tab);
    क्रम (n = 0; n < RAND_शीर्ष_LEN-2; n++)
    {
        buf[n] = (अचिन्हित अक्षर)zencode(pkeys, pcrc_32_tab, header[n], t);
    }
    buf[n++] = (अचिन्हित अक्षर)zencode(pkeys, pcrc_32_tab, (पूर्णांक)(crcForCrypting >> 16) & 0xff, t);
    buf[n++] = (अचिन्हित अक्षर)zencode(pkeys, pcrc_32_tab, (पूर्णांक)(crcForCrypting >> 24) & 0xff, t);
    वापस n;
}

#पूर्ण_अगर
