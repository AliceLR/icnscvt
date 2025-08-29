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
 * Generic test for all ICNS write handlers.
 *
 * Since encoding is done elsewhere, this mainly just needs to make sure the
 * correct raw data is saved and at the correct size.
 */
static void test_format_write_to_icns(struct icns_data * RESTRICT icns,
  const struct test_format *which)
{
  const struct icns_format *format = which->format;
  const struct loaded_file *loaded = NULL;
  const struct loaded_file *compare = NULL;
  struct icns_image *image;
  char tmp[512];
  enum icns_error ret;
  uint8_t *buffer;

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  check_ok(icns, ret);
  check_image_dirty(image); /* Clear flags */

  buffer = (uint8_t *)malloc(OUTPUT_BUFFER_SIZE);
  ASSERT(buffer, "failed to allocate output buffer");

  /* No image data -> always fail */
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  icns_io_end(icns);

  /* No prepared image data -> always fail */
  image->pixels = icns_allocate_pixel_array_for_image(image);
  ASSERT(image->pixels, "%s: failed to allocated pixel array", format->name);

  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  icns_clear_image(image);
  icns_io_end(icns);
  check_image_dirty(image); /* Clear flags */

  ASSERT(which->png_prefix, "%s: no file prefix provided", format->name);
  snprintf(tmp, sizeof(tmp), "%s.tga.gz", which->png_prefix);
  compare = test_load_cached(icns, tmp);
  snprintf(tmp, sizeof(tmp), "%s.png", which->png_prefix);
  loaded = test_load_cached(icns, tmp);

  /* The actual contents of these pointers doesn't matter at this stage. */
  /* PNG set only: direct copy if png_save, otherwise ICNS_INTERNAL_ERROR */
  image->png = loaded->data;
  image->png_size = loaded->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  if(which->png_save)
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, loaded->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, loaded->data_size);
    ASSERTMEM(icns->io.ptr.dest, loaded->data, loaded->data_size, "%s", format->name);
  }
  else
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

  icns_io_end(icns);
  clear_image_no_free(image);

  /* JP2 set only: direct copy if png_save, otherwise ICNS_INTERNAL_ERROR */
  image->jp2 = loaded->data;
  image->jp2_size = loaded->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  if(which->png_save)
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, loaded->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, loaded->data_size);
    ASSERTMEM(icns->io.ptr.dest, loaded->data, loaded->data_size, "%s", format->name);
  }
  else
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

  icns_io_end(icns);

  /* PNG and JP2 set: direct copy PNG if png_save, otherwise ICNS_INTERNAL_ERROR */
  image->png = compare->data;
  image->png_size = compare->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  if(which->png_save)
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, compare->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, compare->data_size);
    ASSERTMEM(icns->io.ptr.dest, compare->data, compare->data_size, "%s", format->name);
  }
  else
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

  icns_io_end(icns);
  clear_image_no_free(image);

  /* RAW set: direct copy if !png_save, otherwise ICNS_INTERNAL_ERROR */
  image->data = loaded->data;
  image->data_size = loaded->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  if(!which->png_save)
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, loaded->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, loaded->data_size);
    ASSERTMEM(icns->io.ptr.dest, loaded->data, loaded->data_size, "%s", format->name);
  }
  else
    check_error(icns, ret, ICNS_INTERNAL_ERROR);

  icns_io_end(icns);

  /* RAW and PNG set: direct copy PNG if png_save, otherwise direct copy RAW */
  image->png = compare->data;
  image->png_size = compare->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  if(which->png_save)
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, compare->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, compare->data_size);
    ASSERTMEM(icns->io.ptr.dest, compare->data, compare->data_size, "%s", format->name);
  }
  else
  {
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, loaded->data_size,
      "%s: %zu != %zu", format->name, icns->io.pos, loaded->data_size);
    ASSERTMEM(icns->io.ptr.dest, loaded->data, loaded->data_size, "%s", format->name);
  }
  icns_io_end(icns);

  /* RAW and PNG set, format supports both outputs, force raw: direct copy RAW */
  if(format->type == ICNS_24_BIT_OR_PNG)
  {
    icns->force_raw_if_available = true;

    ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
    check_ok(icns, ret);
    ret = format->write_to_icns(icns, image);
    check_ok(icns, ret);
    ASSERTEQ(icns->io.pos, loaded->data_size, "%s", format->name);
    ASSERTMEM(icns->io.ptr.dest, loaded->data, loaded->data_size, "%s", format->name);
    icns_io_end(icns);

    icns->force_raw_if_available = false;
  }

  /* Unaffected by dirty_external. */
  image->dirty_external = true;
  image->png = loaded->data;
  image->png_size = loaded->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  check_ok(icns, ret);
  icns_io_end(icns);
  image->dirty_external = false;

  /* Data is okay but dirty_icns is set -> ICNS_INTERNAL_ERROR */
  image->dirty_icns = true;
  image->png = loaded->data;
  image->png_size = loaded->data_size;
  ret = icns_io_init_write_memory(icns, buffer, OUTPUT_BUFFER_SIZE);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);
  icns_io_end(icns);
  image->dirty_icns = false;

  /* RAW and PNG set with no IO initialized: ICNS_INTERNAL_ERROR */
  ret = format->write_to_icns(icns, image);
  check_error(icns, ret, ICNS_INTERNAL_ERROR);

  /* RAW and PNG set with too small buffer: ICNS_WRITE_ERROR */
  ret = icns_io_init_write_memory(icns, tmp, 1);
  check_ok(icns, ret);
  ret = format->write_to_icns(icns, image);
  check_error(icns, ret, ICNS_WRITE_ERROR);

  icns_io_end(icns);
  clear_image_no_free(image);
  icns_delete_all_images(icns);
  free(buffer);
}
