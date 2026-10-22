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

#నిర్వచించు చక్రీయ_తనిఖీ_32(c, b) ((*(pcrc_32_tab+(((పూర్ణాంకం)(c) ^ (b)) & 0xff))) ^ ((c) >> 8))

/***********************************************************************
 * Return the next byte in the pseudo-random sequence
 */
స్థిర పూర్ణాంకం decrypt_byte(గుర్తులేని దీర్ఘ* pkeys, స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab) {
    గుర్తులేని తాత్కాలిక;  /* POTENTIAL BUG:  temp*(temp^1) may overflow in an
                     * unpredictable manner on 16-bit systems; not a problem
                     * with any known compiler so far, though */

    (శూన్య)pcrc_32_tab;
    తాత్కాలిక = ((గుర్తులేని)(*(pkeys+2)) & 0xffff) | 2;
    తిరిగి (పూర్ణాంకం)(((తాత్కాలిక * (తాత్కాలిక ^ 1)) >> 8) & 0xff);
}

/***********************************************************************
 * Update the encryption keys with the next byte of plain text
 */
స్థిర పూర్ణాంకం update_కీలకాలు(గుర్తులేని దీర్ఘ* pkeys, స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab, పూర్ణాంకం c) {
    (*(pkeys+0)) = చక్రీయ_తనిఖీ_32((*(pkeys+0)), c);
    (*(pkeys+1)) += (*(pkeys+0)) & 0xff;
    (*(pkeys+1)) = (*(pkeys+1)) * 134775813L + 1;
    {
      పూర్ణాంకం keyshift = (పూర్ణాంకం)((*(pkeys+1)) >> 24);
      (*(pkeys+2)) = చక్రీయ_తనిఖీ_32((*(pkeys+2)), keyshift);
    }
    తిరిగి c;
}


/***********************************************************************
 * Initialize the encryption keys and the random header according to
 * the given password.
 */
స్థిర శూన్య init_కీలకాలు(స్థిరరాశి అక్షరం* passwd, గుర్తులేని దీర్ఘ* pkeys, స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab) {
    *(pkeys+0) = 305419896L;
    *(pkeys+1) = 591751049L;
    *(pkeys+2) = 878082192L;
    అయితే (*passwd != '\0') {
        update_కీలకాలు(pkeys,pcrc_32_tab,(పూర్ణాంకం)*passwd);
        passwd++;
    }
}

#నిర్వచించు zdecode(pkeys,pcrc_32_tab,c) \
    (update_కీలకాలు(pkeys,pcrc_32_tab,c ^= decrypt_byte(pkeys,pcrc_32_tab)))

#నిర్వచించు zencode(pkeys,pcrc_32_tab,c,t) \
    (t=decrypt_byte(pkeys,pcrc_32_tab), update_కీలకాలు(pkeys,pcrc_32_tab,c), (Byte)t^(c))

#నిర్వచితమైతే INCLUDECRYPTINGCODE_IFCRYPTALLOWED

#నిర్వచించు RAND_తల_LEN  12
   /* "last resort" source for second part of crypt seed pattern */
#  ifndef ZCR_SEED2
#    define ZCR_SEED2 3141592654UL      /* use PI as default pattern */
#  endif

స్థిర గుర్తులేని crypthead(స్థిరరాశి అక్షరం* passwd,       /* password string */
                          గుర్తులేని అక్షరం* buf,       /* where to write header */
                          పూర్ణాంకం buf_కొలత,
                          గుర్తులేని దీర్ఘ* pkeys,
                          స్థిరరాశి z_చక్రీయ_తనిఖీ_t* pcrc_32_tab,
                          గుర్తులేని దీర్ఘ చక్రీయ_తనిఖీ_For_Crypting) {
    గుర్తులేని n;                  /* index in random header */
    పూర్ణాంకం t;                       /* temporary */
    పూర్ణాంకం c;                       /* random byte */
    గుర్తులేని అక్షరం శీర్షిక[RAND_తల_LEN-2]; /* random header */
    స్థిర గుర్తులేని calls = 0;   /* ensure different random header each time */

    ఒకవేళ (buf_కొలత<RAND_తల_LEN)
      తిరిగి 0;

    /* First generate RAND_HEAD_LEN-2 random bytes. We encrypt the
     * output of rand() to get less predictability, since rand() is
     * often poorly implemented.
     */
    ఒకవేళ (++calls == 1)
    {
        srand((గుర్తులేని)time(ఖాళీ) ^ ZCR_SEED2);
    }
    init_కీలకాలు(passwd, pkeys, pcrc_32_tab);
    కొరకు (n = 0; n < RAND_తల_LEN-2; n++)
    {
        c = (rand() >> 7) & 0xff;
        శీర్షిక[n] = (గుర్తులేని అక్షరం)zencode(pkeys, pcrc_32_tab, c, t);
    }
    /* Encrypt random header (last two bytes is high word of crc) */
    init_కీలకాలు(passwd, pkeys, pcrc_32_tab);
    కొరకు (n = 0; n < RAND_తల_LEN-2; n++)
    {
        buf[n] = (గుర్తులేని అక్షరం)zencode(pkeys, pcrc_32_tab, శీర్షిక[n], t);
    }
    buf[n++] = (గుర్తులేని అక్షరం)zencode(pkeys, pcrc_32_tab, (పూర్ణాంకం)(చక్రీయ_తనిఖీ_For_Crypting >> 16) & 0xff, t);
    buf[n++] = (గుర్తులేని అక్షరం)zencode(pkeys, pcrc_32_tab, (పూర్ణాంకం)(చక్రీయ_తనిఖీ_For_Crypting >> 24) & 0xff, t);
    తిరిగి n;
}

#అయితే_ముగింపు
