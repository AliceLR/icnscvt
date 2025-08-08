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
 * Generic test for all external write prepare functions.
 *
 * This function is only used to merge separate 8-bit masks into their
 * corresponding RGB image when exporting a PNG, and for converting raw
 * 8-bit masks into RGBA data (which is not done normally when decoding ICNS).
  * Nothing else should implement this function.
 */
static void test_format_prepare_for_external(struct icns_data * RESTRICT icns,
  const struct test_format *which)
{
  const struct icns_format *format = which->format;
  const struct icns_format *mask_format;
  const struct loaded_file *loaded;
  const struct loaded_file *compare;
  struct icns_image *image;
  struct icns_image *mask;
  char tmp[512];
  enum icns_error ret;
  size_t sz;
  size_t j;

  mask_format = icns_get_mask_for_format(format);
  if(mask_format || icns_format_is_mask(format))
  {
    ASSERT(format->prepare_for_external, "must be set for 24-bit and masks");
  }
  else
  {
    ASSERT(!format->prepare_for_external,
      "%d should not have a prepare function", format->type);
    return;
  }

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  check_ok(icns, ret);
  sz = image->real_width * image->real_height;

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_tga_cached(icns,
    image->real_width, image->real_height, tmp);

  /* No data present -> ICNS_INTERNAL_ERROR */
  ret = format->prepare_for_external(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
  ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
  ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
  ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

  if(icns_format_is_mask(format))
  {
    ASSERT(which->raw_filename, "%s: no raw filename provided", format->name);
    loaded = test_load_cached(icns, which->raw_filename);

    /* Raw not set -> ICNS_INTERNAL_ERROR */
    image->pixels = compare->pixels;
    image->png = loaded->data;
    image->jp2 = loaded->data;
    ret = format->prepare_for_external(icns, image);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERTEQ(image->pixels, compare->pixels, "%s", format->name);
    ASSERTEQ(image->png, loaded->data, "%s", format->name);
    ASSERTEQ(image->jp2, loaded->data, "%s", format->name);
    clear_image_no_free(image);

    /* Raw -> generate opaque pixels with all of R/G/B set to mask value. */
    image->data = loaded->data;
    image->data_size = loaded->data_size;
    ret = format->prepare_for_external(icns, image);
    check_ok(icns, ret);
    ASSERT(IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERT(IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

    for(j = 0; j < sz; j++)
    {
      ASSERTEQ(image->pixels[j].r, image->data[j],
        "%s: %zu: %d != %d", format->name, j, image->pixels[j].r, image->data[j]);
      ASSERTEQ(image->pixels[j].g, image->data[j],
        "%s: %zu: %d != %d", format->name, j, image->pixels[j].g, image->data[j]);
      ASSERTEQ(image->pixels[j].b, image->data[j],
        "%s: %zu: %d != %d", format->name, j, image->pixels[j].b, image->data[j]);
      ASSERTEQ(image->pixels[j].a, 255,
        "%s: %zu: %d != 255", format->name, j, image->pixels[j].a);
    }
    image->data = NULL;
  }
  else
  {
    compare = test_load_cached(icns, tmp);

    /* Pixels not set -> ICNS_INTERNAL_ERROR */
    image->data = compare->data;
    image->png = compare->data;
    image->jp2 = compare->data;
    ret = format->prepare_for_external(icns, image);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    ASSERT(!IMAGE_IS_PIXELS(image), "%s", format->name);
    ASSERTEQ(image->data, compare->data, "%s", format->name);
    ASSERTEQ(image->png, compare->data, "%s", format->name);
    ASSERTEQ(image->jp2, compare->data, "%s", format->name);
    clear_image_no_free(image);

    image->pixels = icns_allocate_pixel_array_for_image(image);
    ASSERT(image->pixels, "%s: failed to allocate pixel array", format->name);
    memcpy(image->pixels, compare->pixels, sz * sizeof(struct rgba_color));

    /* Pixels set, no mask image -> okay; just keep full opacity. */
    ret = format->prepare_for_external(icns, image);
    check_ok(icns, ret);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

    ret = icns_add_image_for_format(icns, &mask, image, mask_format);
    check_ok(icns, ret);

    /* Pixels set, mask image missing raw -> ICNS_INTERNAL_ERROR */
    ret = format->prepare_for_external(icns, image);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

    /* Pixels set, mask image -> success, should copy over alpha */
    mask->data = (uint8_t *)malloc(sz);
    mask->data_size = sz;
    for(j = 0; j < sz; j++)
      mask->data[j] = j * 7 - 5;

    ret = format->prepare_for_external(icns, image);
    check_ok(icns, ret);
    ASSERT(!IMAGE_IS_RAW(image), "%s", format->name);
    ASSERT(!IMAGE_IS_PNG(image), "%s", format->name);
    ASSERT(!IMAGE_IS_JPEG_2000(image), "%s", format->name);

    for(j = 0; j < sz; j++)
    {
      ASSERTEQ(image->pixels[j].a, mask->data[j],
        "%s: %d != %d", format->name, image->pixels[j].a, mask->data[j]);
    }

    /* Pixels missing, mask set -> ICNS_INTERNAL_ERROR */
    icns_clear_image(image);
    ret = format->prepare_for_external(icns, image);
    check_error(icns, ret, ICNS_INTERNAL_ERROR);
  }

  icns_delete_all_images(icns);
}
