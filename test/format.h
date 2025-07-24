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

static inline void check_pixels(const struct icns_image *image,
  const struct loaded_file *compare)
{
  size_t sz = image->real_width * image->real_height * sizeof(struct rgba_color);
  ASSERT(image->pixels, "%s", image->format->name);
  ASSERTMEM(image->pixels, compare->pixels, sz, "%s", image->format->name);
}

void test_format_functions(const struct icns_format *format);

ICNS_END_DECLS

#endif /* ICNSCVT_TEST_FORMAT_H */
