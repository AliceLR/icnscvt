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

#include "test.h"
#include "targa.h"
#include "format.h"
#include "../src/icns_image.h"
#include "../src/icns_io.h"
#include "../src/icns_png.h"

/**
 * Generic test for all external read handlers.
 *
 * External files should be PNG only, unless the format handler supports
 * JPEG 2000, in which case it will pass through JPEG 2000 files with minimal
 * size verification.
 */
static void test_format_read_from_external(struct icns_data * RESTRICT icns,
  const struct test_format *list, size_t total,
  const struct test_format *which, size_t i)
{
  const struct icns_format *format = which->format;
  const struct loaded_file *loaded;
  const struct loaded_file *compare;
  struct icns_image *image;
  char tmp[512];
  enum icns_error ret;
  size_t j;

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  check_ok(icns, ret);

  /* Load junk -> ICNS_DATA_ERROR */
  ret = icns_io_init_read_memory(icns, test_random_data, sizeof(test_random_data));
  check_ok(icns, ret);
  ret = format->read_from_external(icns, image, sizeof(test_random_data));
  check_error(icns, ret, ICNS_DATA_ERROR);
  icns_io_end(icns);

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_cached(icns, tmp);
  compare = test_load_tga_cached(icns,
    image->real_width, image->real_height, tmp);

  if(which->raw_filename)
  {
    /* Load raw -> ICNS_DATA_ERROR */
    ASSERT(which->raw_filename, "%s: no raw filename provided", format->name);
    loaded = test_load_cached(icns, which->raw_filename);
    ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
    check_ok(icns, ret);
    ret = format->read_from_external(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_DATA_ERROR);
    icns_io_end(icns);

    ASSERT(!IMAGE_IS_RAW(image), "%s: never load external raw", format->name);
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
  }

  snprintf(tmp, sizeof(tmp), "%s.png", which->png_prefix);
  loaded = test_load_cached(icns, tmp);

  /* PNG: keep if supported, otherwise pixels (or a raw mask, if a mask) */
  icns->force_recoding = false;
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  ret = format->read_from_external(icns, image, loaded->data_size);
  check_ok(icns, ret);
  if(which->png_load)
  {
    /* This may or may not also generate pixels for a PNG depending on format. */
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
    ASSERTEQ(image->png_size, loaded->data_size,
      "%s: %zu != %zu", format->name, image->png_size, loaded->data_size);
    ASSERTMEM(image->png, loaded->data, loaded->data_size, "%s", format->name);
  }
  else

  if(!icns_format_is_mask(format))
  {
    /* This may keep external PNGs. */
    ASSERT(IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
    check_pixels(image, compare);
  }
  else
  {
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
  }
  icns_io_end(icns);
  icns_clear_image(image);

  /* PNG, force recoding: always pixels (unless a mask) */
  icns->force_recoding = true;
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  ret = format->read_from_external(icns, image, loaded->data_size);
  check_ok(icns, ret);
  if(!icns_format_is_mask(format))
  {
    ASSERT(IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
    check_pixels(image, compare);
  }
  else
  {
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
  }
  icns_io_end(icns);
  icns_clear_image(image);

  snprintf(tmp, sizeof(tmp), "%s.j2k", which->png_prefix);
  loaded = test_load_cached(icns, tmp);

  /* JP2: keep if supported, otherwise ICNS_DATA_ERROR */
  icns->force_recoding = false;
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  ret = format->read_from_external(icns, image, loaded->data_size);
  if(which->png_load)
  {
    check_ok(icns, ret);
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(IMAGE_IS_JPEG_2000(image), "%s", format->name);
    ASSERTEQ(image->jp2_size, loaded->data_size,
      "%s: %zu != %zu", format->name, image->jp2_size, loaded->data_size);
    ASSERTMEM(image->jp2, loaded->data, loaded->data_size, "%s", format->name);
  }
  else
    check_error(icns, ret, ICNS_DATA_ERROR);

  icns_io_end(icns);
  icns_clear_image(image);

  /* JP2, force recoding: ICNS_UNIMPLEMENTED_FORMAT if supported,
   * otherwise ICNS_DATA_ERROR */
  icns->force_recoding = true;
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  ret = format->read_from_external(icns, image, loaded->data_size);
  if(which->png_load)
    check_error(icns, ret, ICNS_UNIMPLEMENTED_FORMAT);
  else
    check_error(icns, ret, ICNS_DATA_ERROR);

  icns_io_end(icns);

  /* Other formats should load from PNG if their dimensions match,
   * otherwise ICNS_INVALID_DIMENSIONS. */
  for(j = 0; j < total; j++)
  {
    if(i != j && strcmp(which->png_prefix, list[j].png_prefix))
    {
      struct icns_png_stat st;

      snprintf(tmp, sizeof(tmp), "%s.png", list[j].png_prefix);
      loaded = test_load_cached(icns, tmp);
      ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
      check_ok(icns, ret);

      ret = icns_get_png_info(icns, &st, loaded->data, loaded->data_size);
      check_ok(icns, ret);

      ret = format->read_from_external(icns, image, loaded->data_size);
      icns_io_end(icns);

      if(st.width == image->real_width && st.height == image->real_height)
      {
        /* Should load; don't bother to compare */
        check_ok(icns, ret);
        icns_clear_image(image);
      }
      else
        check_error(icns, ret, ICNS_INVALID_DIMENSIONS);
    }
  }

  icns_delete_all_images(icns);
}
