#ifndef CRC32_VX_HOOKS_H
#define CRC32_VX_HOOKS_H

/**
 * CRC HOOKS
 */
ZLIB_INTERNAL خارجي unsigned long (*crc32_z_hook)(unsigned long crc, ثابت unsigned حرف FAR *buf, z_size_t len);

#endif /* CRC32_VX_HOOKS_H */
