/* icnscvt
 *
 * Copyright (C) 2025 Alice Rowan <petrifiedrowan@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */

#ifndef ICNSCVT_FORMATS_H
#define ICNSCVT_FORMATS_H

#include "common.h"

ICNS_BEGIN_DECLS

enum icns_format_type
{
  ICNS_UNKNOWN,
  ICNS_1_BIT,
  ICNS_1_BIT_WITH_MASK,
  ICNS_4_BIT,
  ICNS_8_BIT,
  ICNS_24_BIT,
  ICNS_8_BIT_MASK,
  ICNS_24_BIT_OR_PNG, /* or jpeg2k */
  ICNS_PNG, /* or jpeg2k */
  ICNS_ARGB_OR_PNG
};

struct icns_format
{
  uint32_t magic;
  const char *name; /* Used to derive icnscvt options. */
  const char *iconset;
  enum icns_format_type type;

  int width;
  int height;
  int factor; /* usually 1, retina if 2 */
  int macos_ver;

  /**
   * Function to prepare image data for import into an ICNS or .iconset.
   * This function generates raw image data which will be written by
   * `write_to_icns` and returns the size of the data, which will be used
   * to generate the ICNS chunk size and TOC.
   *
   * @param   icns    current state data.
   * @param   image   source image to prepare for import to an ICNS.
   * @param   size    the total data size will be written to this pointer.
   *                  This should NOT include the size of the chunk header.
   * @return          `ICNS_OK` on success, otherwise a non-zero `icns_error` value.
   */
  enum icns_error (*prepare_for_icns)(struct icns_data * RESTRICT,
   struct icns_image * RESTRICT, size_t * RESTRICT);

  /**
   * Function to read data from an open stream using icns->read_priv. This
   * may be either from an ICNS file (after the chunk header) or a raw file.
   * If `NULL`, this format does not support being exported.
   *
   * @param   icns    current state data.
   * @param   image   destination image to read data to.
   * @param   size    size of chunk to be read.
   * @return          `ICNS_OK` on success, otherwise a non-zero `icns_error` value.
   */
  enum icns_error (*read_from_icns)(struct icns_data * RESTRICT,
   struct icns_image * RESTRICT, size_t);

  /**
   * Function to write data to an open stream using icns->write_priv.
   * For ICNS, the stream is positioned after the chunk header; for .iconset,
   * the stream is positioned at the start of a file. This function should
   * write exactly the number of bytes declared by its corresponding
   * `prepare_for_icns` function; otherwise, the caller will emit an error.
   * If `NULL`, this format does not support being imported.
   *
   * @param   icns    current state data.
   * @param   image   source image to write data from.
   * @return          `ICNS_OK` on success, otherwise a non-zero `icns_error` value.
   */
  enum icns_error (*write_to_icns)(struct icns_data * RESTRICT,
   const struct icns_image *);

  /**
   * Function to prepare image data for export to an external file.
   * Most of the time this function isn't necessary, but for 24-bit
   * is32/il32/ih32/it32 icons, this generates an alpha channel from the
   * s8mk/l8mk/h8mk/t8mk masks.
   */
  enum icns_error (*prepare_for_external)(struct icns_data * RESTRICT,
   struct icns_image * RESTRICT);

  /**
   * Function to read data from an external file using icns->read_priv.
   * If `NULL`, this format does not support being imported.
   *
   * @param   icns    current state data.
   * @param   image   destination image to read data to.
   * @param   size    size of external data to be read.
   * @return          `ICNS_OK` on success, otherwise a non-zero `icns_error` value.
   */
  enum icns_error (*read_from_external)(struct icns_data * RESTRICT,
   struct icns_image * RESTRICT, size_t);

  /**
   * Function to write data to an external file using icns->write_priv.
   * If `NULL`, this format does not support being exported.
   *
   * @param   icns    current state data.
   * @param   image   source image to write data from.
   * @return          `ICNS_OK` on success, otherwise a non-zero `icns_error` value.
   */
  enum icns_error (*write_to_external)(struct icns_data * RESTRICT,
   const struct icns_image *);
};

struct icns_format_option
{
  const char *option;
  uint32_t magic;
};

size_t icns_get_format_string(void *dest, size_t dest_len,
 uint32_t magic, const struct icns_format *format) NOT_NULL;

const struct icns_format *icns_get_format_by_magic(uint32_t magic);
const struct icns_format *icns_get_format_by_name(const char *name) NOT_NULL;
const struct icns_format *icns_get_mask_for_format(
 const struct icns_format *format) NOT_NULL;

bool icns_format_supports_input(const struct icns_format *format,
 enum icns_target_type input_target) NOT_NULL;
bool icns_format_supports_output(const struct icns_format *format,
 enum icns_target_type output_target) NOT_NULL;
bool icns_format_is_mask(const struct icns_format *format) NOT_NULL;
bool icns_format_supports_jpeg_2000(const struct icns_format *format) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_FORMATS_H */
