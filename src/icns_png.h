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

#ifndef ICNSCVT_PNG_H
#define ICNSCVT_PNG_H

#include "common.h"
#include "icns_image.h"

ICNS_BEGIN_DECLS

#define ICNS_PNG_TYPE_IS_INDEXED      1
#define ICNS_PNG_TYPE_IS_COLOR        2
#define ICNS_PNG_TYPE_IS_ALPHA        4
enum icns_png_type
{
  ICNS_PNG_TYPE_GREY                = 0,
  ICNS_PNG_TYPE_GREY_INDEXED        = ICNS_PNG_TYPE_GREY | ICNS_PNG_TYPE_IS_INDEXED,
  ICNS_PNG_TYPE_RGB                 = ICNS_PNG_TYPE_IS_COLOR,
  ICNS_PNG_TYPE_RGB_INDEXED         = ICNS_PNG_TYPE_RGB | ICNS_PNG_TYPE_IS_INDEXED,
  ICNS_PNG_TYPE_GREY_ALPHA          = ICNS_PNG_TYPE_IS_ALPHA,
  ICNS_PNG_TYPE_GREY_ALPHA_INDEXED  = ICNS_PNG_TYPE_GREY_ALPHA | ICNS_PNG_TYPE_IS_INDEXED,
  ICNS_PNG_TYPE_RGBA                = ICNS_PNG_TYPE_IS_COLOR | ICNS_PNG_TYPE_IS_ALPHA,
  ICNS_PNG_TYPE_RGBA_INDEXED        = ICNS_PNG_TYPE_RGBA | ICNS_PNG_TYPE_IS_INDEXED
};
#define ICNS_PNG_IS_INDEXED(t)  (!!((t) & ICNS_PNG_TYPE_IS_INDEXED))
#define ICNS_PNG_IS_COLOR(t)    (!!((t) & ICNS_PNG_TYPE_IS_COLOR))
#define ICNS_PNG_IS_ALPHA(t)    (!!((t) & ICNS_PNG_TYPE_IS_ALPHA))

struct icns_png_stat
{
  unsigned width;           /* width, pixels */
  unsigned height;          /* height, pixels */
  unsigned depth;           /* pixel component depth (1, 2, 4, 8, 16) */
  enum icns_png_type type;  /* type (grey, color, indexed, alpha) */
  bool interlace;           /* adam7 interlaced if true */
  bool has_trns;            /* tRNS chunk instead of alpha if true */
};

bool icns_is_file_png(const void *data, size_t data_size) NOT_NULL;

enum icns_error icns_get_png_info(
 struct icns_data * RESTRICT icns, struct icns_png_stat * RESTRICT dest,
 const uint8_t *png_data, size_t png_size) NOT_NULL;
enum icns_error icns_decode_png_to_pixel_array(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image,
 const uint8_t *png_data, size_t png_size) NOT_NULL;

enum icns_error icns_encode_png_to_stream(struct icns_data * RESTRICT icns,
 const struct rgba_color *pixels, size_t width, size_t height) NOT_NULL;
enum icns_error icns_encode_png_to_buffer(
 struct icns_data * RESTRICT icns, uint8_t **dest, size_t *dest_size,
 const struct rgba_color *pixels, size_t width, size_t height) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_PNG_H */
