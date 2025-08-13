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

#ifndef ICNSCVT_TEST_FORMAT_H
#define ICNSCVT_TEST_FORMAT_H

#include "test.h"
#include "targa.h"
#include "../src/icns_format.h"
#include "../src/icns_image.h"

ICNS_BEGIN_DECLS

#define OUTPUT_BUFFER_SIZE (1 << 16)

/* Pregenerated PNGs should decode to the original pixels;
 * JP2s are lossy and just exist to test pass-through. */
struct test_format
{
  const struct icns_format *format;
  bool png_load;
  bool png_save;
  const char *png_prefix;
  const char *raw_filename;
};

NOT_NULL
static inline void clear_image_no_free(struct icns_image *image)
{
  image->pixels = NULL;
  image->png = NULL;
  image->jp2 = NULL;
  image->data = NULL;
  image->png_size = 0;
  image->jp2_size = 0;
  image->data_size = 0;
}

/* Macro to preserve invocation file/line. */
#define check_pixels(image, compare) do { \
  const struct rgba_color *i_pixel = (image)->pixels; \
  const struct rgba_color *c_pixel = (compare)->pixels; \
  size_t sz = (image)->real_width * (image)->real_height; \
  size_t i; \
  int ignore_alpha = (image)->format->type == ICNS_24_BIT || \
    (image)->format->type == ICNS_24_BIT_OR_PNG; \
  ASSERT((image)->pixels, "%s", (image)->format->name); \
  ASSERT((compare)->pixels, "%s", (image)->format->name); \
  for(i = 0; i < sz; i++) \
  { \
    ASSERTMEM(i_pixel, c_pixel, sizeof(struct rgba_color) - ignore_alpha, \
      "%s: pixel %zu: %d %d %d %d != %d %d %d %d", \
      (image)->format->name, i, \
      i_pixel->r, i_pixel->g, i_pixel->b, i_pixel->a, \
      c_pixel->r, c_pixel->g, c_pixel->b, c_pixel->a); \
    \
    i_pixel++; \
    c_pixel++; \
  } \
} while(0)

void test_format_functions(const struct icns_format *format) NOT_NULL;
void test_format_maybe_generate_raw(const struct icns_format *format) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_TEST_FORMAT_H */
