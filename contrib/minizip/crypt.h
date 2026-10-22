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

#تعریف_کرو چکری_جانچ_32(c, b) ((*(pcrc_32_tab+(((صحیح)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
ساکن صحیح decrypt_byte(غیرعلامتی طویل* pkeys, ثابت z_چکری_جانچ_t* pcrc_32_tab) {
    غیرعلامتی عارضی;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (خالی)pcrc_32_tab;
    عارضی = ((غیرعلامتی)(*(pkeys+2)) & 0xffff) | 2;
    واپس (صحیح)(((عارضی * (عارضی ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
ساکن صحیح update_کلیدیں(غیرعلامتی طویل* pkeys, ثابت z_چکری_جانچ_t* pcrc_32_tab, صحیح c) {
    (*(pkeys+0)) = چکری_جانچ_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      صحیح keyshift = (صحیح)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = چکری_جانچ_32((*(pkeys+2)), keyshift);
    }
    واپس c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
ساکن خالی init_کلیدیں(ثابت حرف* passwd, غیرعلامتی طویل* pkeys, ثابت z_چکری_جانچ_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    جب (*passwd != '\0') {
        update_کلیدیں(pkeys,pcrc_32_tab,(صحیح)*passwd);
        passwd++;
    }
}

#تعریف_کرو zdecode(pkeys,pcrc_32_tab,c) \
    (update_کلیدیں(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#تعریف_کرو zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_کلیدیں(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#اگر_تعریف_شدہ INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#تعریف_کرو RAND_سر_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

ساکن غیرعلامتی crypthead(ثابت حرف* passwd,       /* password string */
                          غیرعلامتی حرف* buf,       /* where to write header */
                          صحیح buf_سائز,
                          غیرعلامتی طویل* pkeys,
                          ثابت z_چکری_جانچ_t* pcrc_32_tab,
                          غیرعلامتی طویل چکری_جانچ_For_Crypting) {
    غیرعلامتی n;                  /* index in random header */
    صحیح t;                       /* temporary */
    صحیح c;                       /* random byte */
    غیرعلامتی حرف سرخی[RAND_سر_LEN-2]; /* random header */
    ساکن غیرعلامتی calls = 0;   /* ensure different random header each time */

    اگر (buf_سائز<RAND_سر_LEN)
      واپس 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    اگر (++calls == 1)
    {
        srand((غیرعلامتی)time(تہی) ^ ZCR_SEED2);
    }
    init_کلیدیں(passwd, pkeys, pcrc_32_tab);
    برائے (n = 0; n < RAND_سر_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        سرخی[n] = (غیرعلامتی حرف)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_کلیدیں(passwd, pkeys, pcrc_32_tab);
    برائے (n = 0; n < RAND_سر_LEN-2; n++)
    {
        buf[n] = (غیرعلامتی حرف)zencode(pkeys, pcrc_32_tab, سرخی[n], t);
    }
    buf[n++] = (غیرعلامتی حرف)zencode(pkeys, pcrc_32_tab, (صحیح)(چکری_جانچ_For_Crypting >> 16) & 0xff, t);
    buf[n++] = (غیرعلامتی حرف)zencode(pkeys, pcrc_32_tab, (صحیح)(چکری_جانچ_For_Crypting >> 24) & 0xff, t);
    واپس n;
}

#اگر_ختم
