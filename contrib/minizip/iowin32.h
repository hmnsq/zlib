/* iowin32.h -- IO base function header for compress/uncompress .zip
     part of the MiniZip project - ( https://www.winimage.com/zLibDll/minizip.html )

         Copyright (C) 1998-2026 Gilles Vollant (minizip) ( https://www.winimage.com/zLibDll/minizip.html )

         Modifications for Zip64 support
         Copyright (C) 2009-2010 Mathias Svensson ( https://result42.com )

         For more info read MiniZip_info.txt

*/

#ضمن <windows.h>


#جيڪڏهن_وصف_ٿيل __cplusplus
خارجي "C" {
#جيڪڏهن_پورو

فراغ fill_win32_filefunc(zlib_filefunc_def* pzlib_filefunc_def);
فراغ fill_win32_filefunc64(zlib_filefunc64_def* pzlib_filefunc_def);
فراغ fill_win32_filefunc64A(zlib_filefunc64_def* pzlib_filefunc_def);
فراغ fill_win32_filefunc64W(zlib_filefunc64_def* pzlib_filefunc_def);

#جيڪڏهن_وصف_ٿيل __cplusplus
}
#جيڪڏهن_پورو
