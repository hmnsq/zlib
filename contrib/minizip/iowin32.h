/* iowin32.h -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ಅನ್ತರ್ಭಾವ <windows.h>


#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
ಬಾಹ್ಯ "C" {
#ಆದರೆ_ಅಂತ್ಯ

ಶೂನ್ಯ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def);
ಶೂನ್ಯ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def);
ಶೂನ್ಯ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def);
ಶೂನ್ಯ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def);

#ವ್ಯಾಖ್ಯಾನಿತವಾದರೆ __cplusplus
}
#ಆದರೆ_ಅಂತ್ಯ
