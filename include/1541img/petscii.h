#ifndef I1541_PETSCII_H
#define I1541_PETSCII_H

/** Declarations for the PETSCII utils module
 * @file
 */

/** @defgroup PETSCII PETSCII
 * Utility functions for PETSCII
 *
 * `#include <1541img/petscii.h>`
 * @{
 *
 * This module contains utility functions for processing PETSCII strings.
 */

#include <stdint.h>

#include <1541img/decl.h>

/** Map uppercase graphics chars to lowercase chars.
 * This function modifies a PETSCII string in place, replacing graphics
 * characters meant for Uppercase/Gfx mode with characters of the same
 * shape that also work in lowercase mode. This only applies to centered
 * horizontal or vertical bars.
 * @param str a pointer to the PETSCII string to modify
 * @param len the length of the PETSCII string to modify
 */
DECLEXPORT void petscii_mapUpperGfxToLower(char *str, uint8_t len);

/** Map petscii string to UTF-8.
 * Convert a petscii string to a string encoded in UTF-8. The result is
 * written to a buffer provided by the caller, and a NUL terminator is
 * always added. A buffer 4 times the size of the petscii string should be
 * sufficient for any conversion. The function returns the actual buffer size
 * required for the converted string, so if the return value is larger than
 * the size of the buffer passed, the result string was truncated.
 * @param buf pointer to a buffer for the result string. If you pass NULL
 *     here, you have to pass 0 for bufsz as well and the function will just
 *     return an appropriate buffer size.
 * @param bufsz size of the result buffer
 * @param str pointer to the original petscii string
 * @param len length of the original petscii string
 * @param lowercase if set to 1, the petscii string is considered lowercase
 * @param onlyPrintable if set to 1, only printable characters are considered
 *     to have a known UTF-8 encoding.
 * @param unknown pointer to a NUL-terminated string used for petscii
 *     characters that don't map to an existing Unicode codepoint. Pass
 *     NULL here to just skip these characters.
 * @param shiftspace pointer to a NUL-terminated string used for shifted
 *     spaces. Pass NULL here to use a non-breaking space, which is visually
 *     indistinguishable from a normal space.
 * @returns the number of characters of the resulting UTF-8 string, including
 *     the NUL terminator.
 */
DECLEXPORT int petscii_toUtf8(
        char *buf, int bufsz, const char *str, uint8_t len, int lowercase,
        int onlyPrintable, const char *unknown, const char *shiftspace);

/**@}*/

#endif

