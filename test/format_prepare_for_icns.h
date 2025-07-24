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
 * Generic test for all ICNS encoding handlers.
 *
 * This function should encode the loaded pixels to either a raw buffer or
 * a PNG and return the size of the data to be written. Make sure it returns
 * the correct size and has the correct buffer set for each context.
 */
static void test_format_prepare_for_icns(struct icns_data * RESTRICT icns,
  const struct test_format *which)
{
  const struct icns_format *format = which->format;
  const struct icns_format *format_mask = NULL;
  const struct loaded_file *loaded = NULL;
  const struct loaded_file *loaded_raw = NULL;
  const struct loaded_file *compare = NULL;
  struct icns_image *image;
  char tmp[512];
  enum icns_error ret;
  size_t sz;
  size_t sz2;
  size_t j;

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  check_ok(icns, ret);

  /* nothing set -> ICNS_INTERNAL_ERROR always */
  ret = format->prepare_for_icns(icns, image, &sz);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_cached(icns, tmp);
  compare = test_load_tga_cached(icns,
    image->real_width, image->real_height, tmp);

  snprintf(tmp, sizeof(tmp), "%s.png", which->png_prefix);
  loaded = test_load_cached(icns, tmp);

  if(which->png_save)
  {
    /* pixels, PNG, JP2 unset -> ICNS_INTERNAL_ERROR */
    /* raw pointer should be ignored and remain unset */
    image->data = compare->data;
    image->data_size = compare->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    clear_image_no_free(image);

    /* pixels set only -> convert to PNG, return PNG size */
    image->pixels = compare->pixels;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->png_size, "%s: %zu != %zu", format->name, sz, image->png_size);
    ASSERT(image->png, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
    ASSERT(!image->data, "%s", format->name);
    /* converted PNG should decode to exact match of pixels */
    image->pixels = NULL;
    ret = icns_decode_png_to_pixel_array(icns, image, image->png, image->png_size);
    check_ok(icns, ret);
    check_pixels(image, compare);
    icns_clear_image(image);

    /* PNG only -> return PNG size */
    image->png = loaded->data;
    image->png_size = loaded->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->png_size, "%s: %zu != %zu", format->name, sz, image->png_size);
    ASSERT(!image->pixels, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
    ASSERT(!image->data, "%s", format->name);

    /* pixels and PNG -> return PNG size */
    image->pixels = compare->pixels;
    ret = format->prepare_for_icns(icns, image, &sz2);
    check_ok(icns, ret);
    ASSERTEQ(sz2, sz, "%s: %zu != %zu", format->name, sz2, sz);
    ASSERT(!image->jp2, "%s", format->name);
    ASSERT(!image->data, "%s", format->name);
    clear_image_no_free(image);

    /* JP2 only -> return JP2 size */
    image->jp2 = compare->data;
    image->jp2_size = compare->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->jp2_size, "%s: %zu != %zu", format->name, sz, image->jp2_size);
    ASSERT(!image->pixels, "%s", format->name);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->data, "%s", format->name);

    /* pixels and JP2 -> return JP2 size */
    image->pixels = compare->pixels;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->jp2_size, "%s: %zu != %zu", format->name, sz, image->jp2_size);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->data, "%s", format->name);

    /* PNG and JP2 -> return PNG size */
    image->png = loaded->data;
    image->png_size = loaded->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->png_size, "%s: %zu != %zu", format->name, sz, image->png_size);
    ASSERT(!image->data, "%s", format->name);
    clear_image_no_free(image);
  }
  else

  if(!icns_format_is_mask(which->format))
  {
    ASSERT(which->raw_filename, "%s: no raw data file provided", format->name);
    loaded_raw = test_load_cached(icns, which->raw_filename);

    /* nothing set -> ICNS_INTERNAL_ERROR */
    ret = format->prepare_for_icns(icns, image, &sz);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

    /* raw set only -> ICNS_INTERNAL_ERROR (these formats must always recode) */
    image->data = compare->data;
    image->data_size = compare->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    ASSERT(!image->pixels, "%s", format->name);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
    clear_image_no_free(image);

    /* everything but pixels set -> same */
    image->data = (uint8_t *)0x12345;
    image->png = image->data;
    image->jp2 = image->data;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    ASSERT(!image->pixels, "%s", format->name);
    clear_image_no_free(image);

    /* pixels set only -> convert to raw, return raw size */
    image->pixels = compare->pixels;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERT(image->data, "%s", format->name);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
    /* raw should exactly match raw input file */
    ASSERTEQ(sz, image->data_size,
      "%s: %zu != %zu", format->name, sz, image->data_size);
    ASSERTEQ(sz, loaded_raw->data_size,
      "%s: %zu != %zu", format->name, sz, loaded_raw->data_size);
    ASSERTMEM(image->data, loaded_raw->data, sz, "%s", format->name);

    /* pixels and raw -> convert to raw (replace), return raw size */
    free(image->data);
    image->data = (uint8_t *)malloc(1234);
    ASSERT(image->data, "failed to allocate throwaway buffer");
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERT(image->data, "%s", format->name);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
    /* raw should exactly match raw input file */
    ASSERTEQ(sz, image->data_size,
      "%s: %zu != %zu", format->name, sz, image->data_size);
    ASSERTEQ(sz, loaded_raw->data_size,
      "%s: %zu != %zu", format->name, sz, loaded_raw->data_size);
    ASSERTMEM(image->data, loaded_raw->data, sz, "%s", format->name);
    free(image->data);
    image->data = NULL;

    format_mask = icns_get_mask_for_format(format);
    if(format_mask)
    {
      /* None of the prior calls should have generated a mask image at
       * any point. */
      struct icns_image *mask = icns_get_image_by_format(icns, format_mask);
      ASSERTEQ(mask, NULL, "%s", format->name);

      ret = icns_add_image_for_format(icns, &mask, image, format_mask);
      check_ok(icns, ret);

      /* When the corresponding mask exists, preparing this image should replace
       * the raw data for the corresponding mask image with an array of the
       * input alpha values from this image. */
      ret = format->prepare_for_icns(icns, image, &sz);
      check_ok(icns, ret);
      ASSERT(image->data, "%s", format->name);
      ASSERT(!image->png, "%s", format->name);
      ASSERT(!image->jp2, "%s", format->name);
      /* raw should exactly match raw input file */
      ASSERTEQ(sz, image->data_size,
        "%s: %zu != %zu", format->name, sz, image->data_size);
      ASSERTEQ(sz, loaded_raw->data_size,
        "%s: %zu != %zu", format->name, sz, loaded_raw->data_size);
      ASSERTMEM(image->data, loaded_raw->data, sz, "%s", format->name);

      ASSERT(mask->data, "%s", format->name);
      sz = image->real_width * image->real_height;
      for(j = 0; j < sz; j++)
        ASSERTEQ(mask->data[j], image->pixels[j].a, "%s @ %zu", format->name, j);

      free(image->data);
      image->data = NULL;
    }
    clear_image_no_free(image);
  }
  else
  {
    /* Masks are prepared either by direct raw loads from ICNS/iconset
     * or when the corresponding RGB is prepared for ICNS/iconset.
     * All the prepare function does is returns the raw size. */

    /* raw is unset -> ICNS_INTERNAL_ERROR */
    ret = format->prepare_for_icns(icns, image, &sz);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

    /* raw is set -> return raw size */
    image->data = compare->data;
    image->data_size = compare->data_size;
    ret = format->prepare_for_icns(icns, image, &sz);
    check_ok(icns, ret);
    ASSERTEQ(sz, image->data_size,
      "%s: %zu != %zu", format->name, sz, image->data_size);
    ASSERT(!image->pixels, "%s", format->name);
    ASSERT(!image->png, "%s", format->name);
    ASSERT(!image->jp2, "%s", format->name);
  }

  icns_delete_all_images(icns);
}
