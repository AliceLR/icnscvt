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

#ifndef ICNSCVT_FORMAT_PNG_H
#define ICNSCVT_FORMAT_PNG_H

#include "common.h"
#include "icns_format.h"
#include "icns_image.h"

ICNS_BEGIN_DECLS

#define icns_magic_icp6 MAGIC('i','c','p','6')
#define icns_magic_ic07 MAGIC('i','c','0','7')
#define icns_magic_ic08 MAGIC('i','c','0','8')
#define icns_magic_ic09 MAGIC('i','c','0','9')
#define icns_magic_ic10 MAGIC('i','c','1','0')
#define icns_magic_ic11 MAGIC('i','c','1','1')
#define icns_magic_ic12 MAGIC('i','c','1','2')
#define icns_magic_ic13 MAGIC('i','c','1','3')
#define icns_magic_ic14 MAGIC('i','c','1','4')
#define icns_magic_icsB MAGIC('i','c','s','B')
#define icns_magic_sb24 MAGIC('s','b','2','4')
#define icns_magic_SB24 MAGIC('S','B','2','4')

extern const struct icns_format icns_format_icp6;
extern const struct icns_format icns_format_ic07;
extern const struct icns_format icns_format_ic08;
extern const struct icns_format icns_format_ic09;
extern const struct icns_format icns_format_ic10;
extern const struct icns_format icns_format_ic11;
extern const struct icns_format icns_format_ic12;
extern const struct icns_format icns_format_ic13;
extern const struct icns_format icns_format_ic14;
extern const struct icns_format icns_format_icsB;
extern const struct icns_format icns_format_sb24;
extern const struct icns_format icns_format_SB24;

enum icns_image_read_png_options
{
  /* permit or deny PNG data; decode discards the PNG data after, unless keep
   * is also specified. */
  ICNS_PNG_KEEP             = 0x01,
  ICNS_PNG_DECODE           = 0x02,
  ICNS_PNG_DECODE_AND_KEEP  = (ICNS_PNG_KEEP | ICNS_PNG_DECODE),
  ICNS_PNG_MASK             = 0x03,
  /* permit non-PNG/JP2; store it to image->data. caller must decode */
  ICNS_RAW_KEEP             = 0x04,
  ICNS_RAW_MASK             = 0x0c,
  /* permit or deny JP2 data (see PNG options) */
  ICNS_JP2_KEEP             = 0x10,
  ICNS_JP2_DECODE           = 0x20,
  ICNS_JP2_DECODE_AND_KEEP  = (ICNS_JP2_KEEP | ICNS_JP2_DECODE),
  ICNS_JP2_MASK             = 0x30
};

enum icns_error icns_image_read_png(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image,
 size_t sz, enum icns_image_read_png_options options);

enum icns_error icns_image_write_pixel_array_to_png(
 struct icns_data * RESTRICT icns, const struct icns_image *image);

ICNS_END_DECLS

#endif /* ICNSCVT_FORMAT_PNG_H */
