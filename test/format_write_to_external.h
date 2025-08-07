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
 * Generic test for all external write handlers.
 *
 * These should write PNGs in all circumstances EXCEPT for if the input data
 * is JPEG 2000.
 */
static void test_format_write_to_external(struct icns_data * RESTRICT icns,
  const struct test_format *which)
{
  const struct icns_format *format = which->format;
  const struct loaded_file *loaded_png = NULL;
  const struct loaded_file *loaded_jp2 = NULL;
  const struct loaded_file *compare = NULL;
  struct icns_image *image;
  char tmp[512];
  enum icns_error ret;
  uint8_t *buffer;

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  check_ok(icns, ret);

  buffer = (uint8_t *)malloc(OUTPUT_BUFFER_SIZE);
  ASSERT(buffer, "failed to allocate output buffer");

  /* No data -> always fail */
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  icns_io_end(icns);

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_cached(icns, tmp);
  snprintf(tmp, sizeof(tmp), "%s.png", which->png_prefix);
  loaded_png = test_load_cached(icns, tmp);
  snprintf(tmp, sizeof(tmp), "%s.j2k", which->png_prefix);
  loaded_jp2 = test_load_cached(icns, tmp);

  /* No pixels, PNG, or JP2 -> always fail */
  image->data = compare->data;
  image->data_size = compare->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  icns_io_end(icns);
  clear_image_no_free(image);

  /* If pixels, encode a PNG. It should match the compare file when decoded. */
  image->pixels = compare->pixels;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  image->pixels = NULL;
  ret = icns_decode_png_to_pixel_array(icns, image,
    icns->io.ptr.dest, icns->io.pos);
  check_ok(icns, ret);
  check_pixels(image, compare);
  icns_io_end(icns);
  icns_clear_image(image);

  /* If PNG, passthrough PNG. */
  image->png = loaded_png->data;
  image->png_size = loaded_png->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  ASSERTEQ(image->png_size, icns->io.pos,
    "%s: %zu != %zu", format->name, image->png_size, icns->io.pos);
  ASSERTMEM(image->png, icns->io.ptr.dest, icns->io.pos, "%s", format->name);
  icns_io_end(icns);

  /* If PNG and pixels, passthrough PNG. */
  image->pixels = compare->pixels;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  ASSERTEQ(image->png_size, icns->io.pos,
    "%s: %zu != %zu", format->name, image->png_size, icns->io.pos);
  ASSERTMEM(image->png, icns->io.ptr.dest, icns->io.pos, "%s", format->name);
  icns_io_end(icns);
  clear_image_no_free(image);

  /* If JP2, passthrough JP2. */
  image->jp2 = loaded_jp2->data;
  image->jp2_size = loaded_jp2->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  ASSERTEQ(image->jp2_size, icns->io.pos,
    "%s: %zu != %zu", format->name, image->jp2_size, icns->io.pos);
  ASSERTMEM(image->jp2, icns->io.ptr.dest, icns->io.pos, "%s", format->name);
  icns_io_end(icns);

  /* If JP2 and pixels, passthrough JP2. */
  image->pixels = compare->pixels;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  ASSERTEQ(image->jp2_size, icns->io.pos,
    "%s: %zu != %zu", format->name, image->jp2_size, icns->io.pos);
  ASSERTMEM(image->jp2, icns->io.ptr.dest, icns->io.pos, "%s", format->name);
  icns_io_end(icns);

  /* If PNG and JP2, passthrough PNG. */
  image->png = loaded_png->data;
  image->png_size = loaded_png->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_ok(icns, ret);
  ASSERTEQ(image->png_size, icns->io.pos,
    "%s: %zu != %zu", format->name, image->png_size, icns->io.pos);
  ASSERTMEM(image->png, icns->io.ptr.dest, icns->io.pos, "%s", format->name);
  icns_io_end(icns);

  /* Data is okay but IO is not initialized -> ICNS_INTERNAL_ERROR */
  ret = format->write_to_external(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);

  /* Buffer too small -> ICNS_WRITE_ERROR */
  ret = icns_io_init_write_memory(icns, buffer, 1);
  check_ok(icns, ret);
  ret = format->write_to_external(icns, image);
  check_error(icns, ret, ICNS_WRITE_ERROR);
  icns_io_end(icns);

  clear_image_no_free(image);
  icns_delete_all_images(icns);
  free(buffer);
}
