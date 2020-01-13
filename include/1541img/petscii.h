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

/**@}*/

#endif

