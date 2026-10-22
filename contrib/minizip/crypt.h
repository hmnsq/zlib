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

#تعریف_کړه دوراني_ازموینه_32(c, b) ((*(pcrc_32_tab+(((صحيح)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
ساكن صحيح decrypt_byte(بینښه اوږد* pkeys, ثابت z_دوراني_ازموینه_t* pcrc_32_tab) {
    بینښه لنډمهاله;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (فراغ)pcrc_32_tab;
    لنډمهاله = ((بینښه)(*(pkeys+2)) & 0xffff) | 2;
    أرجع (صحيح)(((لنډمهاله * (لنډمهاله ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
ساكن صحيح update_کلیګانې(بینښه اوږد* pkeys, ثابت z_دوراني_ازموینه_t* pcrc_32_tab, صحيح c) {
    (*(pkeys+0)) = دوراني_ازموینه_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      صحيح keyshift = (صحيح)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = دوراني_ازموینه_32((*(pkeys+2)), keyshift);
    }
    أرجع c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
ساكن فراغ init_کلیګانې(ثابت حرف* passwd, بینښه اوږد* pkeys, ثابت z_دوراني_ازموینه_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    طالما (*passwd != '\0') {
        update_کلیګانې(pkeys,pcrc_32_tab,(صحيح)*passwd);
        passwd++;
    }
}

#تعریف_کړه zdecode(pkeys,pcrc_32_tab,c) \
    (update_کلیګانې(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#تعریف_کړه zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_کلیګانې(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#که_تعریف_شوی INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#تعریف_کړه RAND_سر_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

ساكن بینښه crypthead(ثابت حرف* passwd,       /* password string */
                          بینښه حرف* buf,       /* where to write header */
                          صحيح buf_اندازه,
                          بینښه اوږد* pkeys,
                          ثابت z_دوراني_ازموینه_t* pcrc_32_tab,
                          بینښه اوږد دوراني_ازموینه_For_Crypting) {
    بینښه n;                  /* index in random header */
    صحيح t;                       /* temporary */
    صحيح c;                       /* random byte */
    بینښه حرف سرلیک[RAND_سر_LEN-2]; /* random header */
    ساكن بینښه calls = 0;   /* ensure different random header each time */

    إذا (buf_اندازه<RAND_سر_LEN)
      أرجع 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    إذا (++calls == 1)
    {
        srand((بینښه)time(عدم) ^ ZCR_SEED2);
    }
    init_کلیګانې(passwd, pkeys, pcrc_32_tab);
    لكل (n = 0; n < RAND_سر_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        سرلیک[n] = (بینښه حرف)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_کلیګانې(passwd, pkeys, pcrc_32_tab);
    لكل (n = 0; n < RAND_سر_LEN-2; n++)
    {
        buf[n] = (بینښه حرف)zencode(pkeys, pcrc_32_tab, سرلیک[n], t);
    }
    buf[n++] = (بینښه حرف)zencode(pkeys, pcrc_32_tab, (صحيح)(دوراني_ازموینه_For_Crypting >> 16) & 0xff, t);
    buf[n++] = (بینښه حرف)zencode(pkeys, pcrc_32_tab, (صحيح)(دوراني_ازموینه_For_Crypting >> 24) & 0xff, t);
    أرجع n;
}

#د_که_پای
