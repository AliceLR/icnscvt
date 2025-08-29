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
 * Generic test for all format ICNS decoding handlers.
 */
static void test_format_read_from_icns(struct icns_data * RESTRICT icns,
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

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_tga_cached(icns,
    image->real_width, image->real_height, tmp);

  if(which->raw_filename)
  {
    loaded = test_load_cached(icns, which->raw_filename);
    ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
    check_ok(icns, ret);
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_ok(icns, ret);
    icns_io_end(icns);

    ASSERT(IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

    if(icns_format_is_mask(format))
    {
      /* Do not decode masks to pixels by default */
      ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    }
    else
    {
      ASSERT(IMAGE_IS_PIXELS(image), "%s", format->name);
      check_pixels(image, compare);
    }
    check_image_dirty(image);

    icns_clear_image(image);

    if(icns_format_is_mask(format))
    {
      struct icns_image *rgb = NULL;

      /* Special: loading a mask should set the external dirty flag of its
      * corresponding RGB image, but not the icns dirty flag. */
      ret = icns_add_image_for_format(icns, &rgb, image, icns_get_format_from_mask(format));
      check_ok(icns, ret);
      check_image_dirty(rgb); /* Clear flags */

      ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
      check_ok(icns, ret);
      ret = format->read_from_icns(icns, image, loaded->data_size);
      check_ok(icns, ret);
      check_image_dirty(image);
      icns_io_end(icns);

      ASSERT(rgb->dirty_external, "%s", format->name);
      ASSERT(!rgb->dirty_icns, "%s", format->name);
    }
  }
  else
  {
    /* Load junk -> ICNS_DATA_ERROR */
    /* Raw formats may interpret this as actual data. */
    ret = icns_io_init_read_memory(icns, test_random_data, sizeof(test_random_data));
    check_ok(icns, ret);
    ret = format->read_from_icns(icns, image, sizeof(test_random_data));
    check_error(icns, ret, ICNS_DATA_ERROR);
    icns_io_end(icns);
  }

  snprintf(tmp, sizeof(tmp), "%s.png", which->png_prefix);
  loaded = test_load_cached(icns, tmp);
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  if(which->png_load)
  {
    /* Direct pass-through -> may or may not decode; always keep raw */
    icns->force_recoding = false;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_ok(icns, ret);
    ASSERT(IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERTEQ(image->png_size, loaded->data_size, "%s", format->name);
    ASSERTMEM(image->png, loaded->data, loaded->data_size, "%s", format->name);
    check_image_dirty(image);
    icns_clear_image(image);

    /* force_recoding -> decode to pixels and discard raw */
    icns->io.pos = 0;
    icns->force_recoding = true;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_ok(icns, ret);
    icns_io_end(icns);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    check_pixels(image, compare);
    check_image_dirty(image);

    icns_clear_image(image);
  }
  else
  {
    /* PNG not supported in ICNS -> ICNS_DATA_ERROR regardless of recoding */
    icns->force_recoding = false;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_DATA_ERROR);

    icns->io.pos = 0;
    icns->force_recoding = true;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_DATA_ERROR);
    icns_io_end(icns);
  }

  snprintf(tmp, sizeof(tmp), "%s.j2k", which->png_prefix);
  loaded = test_load_cached(icns, tmp);
  ret = icns_io_init_read_memory(icns, loaded->data, loaded->data_size);
  check_ok(icns, ret);
  if(which->png_load)
  {
    /* Direct pass-through -> do not decode, keep raw */
    icns->force_recoding = false;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_ok(icns, ret);
    ASSERT(IMAGE_IS_JPEG_2000(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERTEQ(image->jp2_size, loaded->data_size, "%s", format->name);
    ASSERTMEM(image->jp2, loaded->data, loaded->data_size, "%s", format->name);
    check_image_dirty(image);
    icns_clear_image(image);

    /* force_recoding -> not supported */
    icns->io.pos = 0;
    icns->force_recoding = true;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_UNIMPLEMENTED_FORMAT);
    icns_io_end(icns);
    icns_clear_image(image);
  }
  else
  {
    /* JP2 not supported in ICNS -> ICNS_DATA_ERROR regardless of recoding */
    icns->force_recoding = false;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_DATA_ERROR);

    icns->io.pos = 0;
    icns->force_recoding = true;
    ret = format->read_from_icns(icns, image, loaded->data_size);
    check_error(icns, ret, ICNS_DATA_ERROR);
    icns_io_end(icns);
  }

  /* Other formats should load if their dimensions match and they can load
   * PNG from ICNS; otherwise, this fails with one of a few errors. */
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

      ret = format->read_from_icns(icns, image, loaded->data_size);
      icns_io_end(icns);

      if(icns_format_supports_png(format))
      {
        if(st.width == image->real_width && st.height == image->real_height)
        {
          /* Should load; don't bother to compare */
          check_ok(icns, ret);
          check_image_dirty(image);
          icns_clear_image(image);
        }
        else
          check_error(icns, ret, ICNS_INVALID_DIMENSIONS);
      }
      else
      {
        if(ret == ICNS_DATA_ERROR || ret == ICNS_READ_ERROR)
          check_error(icns, ret, ret);
        else
          FAIL("%d != ICNS_DATA_ERROR (%d) or ICNS_READ_ERROR (%d)",
            ret, ICNS_DATA_ERROR, ICNS_READ_ERROR);
      }
    }
  }

  icns_delete_all_images(icns);
}
