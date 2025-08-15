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
#include "../src/icns.h"
#include "../src/icns_io.h"
#include "../src/icns_png.h"
#include "../src/icns_format_png.h"

static const enum icns_image_read_png_options png_combos[] =
{
  0,
  ICNS_PNG_KEEP,
  ICNS_PNG_DECODE,
  ICNS_PNG_DECODE_AND_KEEP
};
static const size_t num_png_combos = sizeof(png_combos) / sizeof(png_combos[0]);

static const enum icns_image_read_png_options jp2_combos[] =
{
  0,
  ICNS_JP2_KEEP,
  ICNS_JP2_DECODE,
  ICNS_JP2_DECODE_AND_KEEP
};
static const size_t num_jp2_combos = sizeof(jp2_combos) / sizeof(jp2_combos[0]);

static const enum icns_image_read_png_options raw_combos[] =
{
  0,
  ICNS_RAW_KEEP
};
static const size_t num_raw_combos = sizeof(raw_combos) / sizeof(raw_combos[0]);


static size_t get_ic11_png(struct icns_data *icns, const uint8_t **src)
{
  static uint8_t *buffer;
  static size_t sz;

  icns_io_end(icns);
  if(!buffer)
    test_load(icns, &buffer, &sz, PNG_DIR "/32x32.png");
  if(src)
    *src = buffer;

  icns_io_init_read_memory(icns, buffer, sz);
  return sz;
}

static size_t get_ic11_jp2(struct icns_data *icns, const uint8_t **src)
{
  static uint8_t *buffer;
  static size_t sz;

  icns_io_end(icns);
  if(!buffer)
    test_load(icns, &buffer, &sz, PNG_DIR "/32x32.j2k");
  if(src)
    *src = buffer;

  icns_io_init_read_memory(icns, buffer, sz);
  return sz;
}

static size_t get_ic12_png(struct icns_data *icns, const uint8_t **src)
{
  static uint8_t *buffer;
  static size_t sz;

  icns_io_end(icns);
  if(!buffer)
    test_load(icns, &buffer, &sz, PNG_DIR "/64x64.png");
  if(src)
    *src = buffer;

  icns_io_init_read_memory(icns, buffer, sz);
  return sz;
}

static size_t get_ic12_jp2(struct icns_data *icns, const uint8_t **src)
{
  static uint8_t *buffer;
  static size_t sz;

  icns_io_end(icns);
  if(!buffer)
    test_load(icns, &buffer, &sz, PNG_DIR "/64x64.j2k");
  if(src)
    *src = buffer;

  icns_io_init_read_memory(icns, buffer, sz);
  return sz;
}

static size_t get_raw(struct icns_data *icns, const uint8_t **src)
{
  static uint8_t *buffer;
  static size_t sz;

  icns_io_end(icns);
  if(!buffer)
    test_load(icns, &buffer, &sz, PNG_DIR "/Makefile");
  if(src)
    *src = buffer;

  icns_io_init_read_memory(icns, buffer, sz);
  return sz;
}

#define check_empty_image(img) do { \
  ASSERT(!(img)->data, "%d", opts); \
  ASSERT(!(img)->png, "%d", opts); \
  ASSERT(!(img)->jp2, "%d", opts); \
  ASSERT(!(img)->pixels, "%d", opts); \
} while(0)

static void check_read_png_individual(struct icns_data *icns,
 struct icns_image *image, enum icns_image_read_png_options opts,
 bool correct_dims, size_t sz)
{
  enum icns_error ret;

  ret = icns_image_read_png(icns, image, sz, opts);
  if((opts & ICNS_PNG_MASK) == 0)
  {
    check_error(icns, ret, ICNS_DATA_ERROR);
    check_empty_image(image);
    return;
  }

  if(!correct_dims)
  {
    check_error(icns, ret, ICNS_INVALID_DIMENSIONS);
    check_empty_image(image);
    return;
  }

  check_ok(icns, ret);

  if(opts & ICNS_PNG_KEEP)
    ASSERT(image->png, "%d: should keep PNG but did not", opts);
  else
    ASSERT(!image->png, "%d: should not keep PNG but did", opts);

  if(opts & ICNS_PNG_DECODE)
    ASSERT(image->pixels, "%d: should decode PNG but did not", opts);
  else
    ASSERT(!image->pixels, "%d: should not decode PNG but did", opts);

  ASSERT(!image->jp2, "%d", opts);
  ASSERT(!image->data, "%d", opts);
  icns_clear_image(image);
}

static void check_read_jp2_individual(struct icns_data *icns,
 struct icns_image *image, enum icns_image_read_png_options opts,
 bool correct_dims, size_t sz)
{
  enum icns_error ret;

  ret = icns_image_read_png(icns, image, sz, opts);
  if((opts & ICNS_JP2_MASK) == 0)
  {
    check_error(icns, ret, ICNS_DATA_ERROR);
    check_empty_image(image);
    return;
  }

  if(!correct_dims)
  {
    check_error(icns, ret, ICNS_INVALID_DIMENSIONS);
    check_empty_image(image);
    return;
  }

  if(opts & ICNS_JP2_DECODE)
  {
    check_error(icns, ret, ICNS_UNIMPLEMENTED_FORMAT);
    check_empty_image(image);
    return;
  }

  check_ok(icns, ret);

  if(opts & ICNS_JP2_KEEP)
    ASSERT(image->jp2, "%d: should keep JP2 but did not", opts);
  else
    ASSERT(!image->jp2, "%d: should not keep JP2 but did", opts);

  ASSERT(!image->data, "%d", opts);
  ASSERT(!image->png, "%d", opts);
  ASSERT(!image->pixels, "%d", opts);
  icns_clear_image(image);
}

static void check_read_raw_individual(struct icns_data *icns,
 struct icns_image *image, enum icns_image_read_png_options opts, size_t sz)
{
  enum icns_error ret;

  ret = icns_image_read_png(icns, image, sz, opts);
  if((opts & ICNS_RAW_MASK) == 0)
  {
    check_error(icns, ret, ICNS_DATA_ERROR);
    check_empty_image(image);
    return;
  }

  check_ok(icns, ret);

  if(opts & ICNS_RAW_KEEP)
    ASSERT(image->data, "%d: should keep raw but did not", opts);
  else
    ASSERT(!image->data, "%d: should not keep raw but did", opts);

  ASSERT(!image->png, "%d", opts);
  ASSERT(!image->jp2, "%d", opts);
  ASSERT(!image->pixels, "%d", opts);
  icns_clear_image(image);
}

static void check_read_png_all(struct icns_data *icns, struct icns_image *image,
 enum icns_image_read_png_options opts, bool is_ic11)
{
  size_t sz;

  /* PNG 32x32 */
  sz = get_ic11_png(icns, NULL);
  check_read_png_individual(icns, image, opts, is_ic11, sz);

  /* PNG 64x64 */
  sz = get_ic12_png(icns, NULL);
  check_read_png_individual(icns, image, opts, !is_ic11, sz);

  /* JP2 32x32 */
  sz = get_ic11_jp2(icns, NULL);
  check_read_jp2_individual(icns, image, opts, is_ic11, sz);

  /* JP2 64x64 */
  sz = get_ic12_jp2(icns, NULL);
  check_read_jp2_individual(icns, image, opts, !is_ic11, sz);

  /* raw */
  sz = get_raw(icns, NULL);
  check_read_raw_individual(icns, image, opts, sz);

  icns_io_end(icns);
}

UNITTEST(format_png_icns_image_read_png)
{
  struct icns_image *image11;
  struct icns_image *image12;
  enum icns_error ret;
  size_t f, i, j, k;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ret = icns_add_image_for_format(&icns, &image11, NULL, &icns_format_ic11);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &image12, NULL, &icns_format_ic12);
  check_ok(&icns, ret);

  /* Verify all deny/keep/decode/both combos with working/bad dimensions.
   * Switching formats should invert which images get filtered by size. */
  for(f = 0; f < 2; f++)
  {
    for(i = 0; i < num_raw_combos; i++)
    {
      for(j = 0; j < num_jp2_combos; j++)
      {
        for(k = 0; k < num_png_combos; k++)
        {
          enum icns_image_read_png_options opts =
           png_combos[k] | jp2_combos[j] | raw_combos[i];

          if(f)
            check_read_png_all(&icns, image12, opts, false);
          else
            check_read_png_all(&icns, image11, opts, true);
        }
      }
    }
  }
  icns_clear_state_data(&icns);
}

UNITTEST(format_png_icns_image_prepare_png_for_icns)
{
  struct icns_image *image;
  const struct loaded_file *loaded_png;
  const struct loaded_file *loaded_jp2;
  const struct loaded_file *compare;
  enum icns_error ret;
  size_t sz;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ret = icns_add_image_for_format(&icns, &image, NULL, &icns_format_ic11);
  check_ok(&icns, ret);

  loaded_png = test_load_cached(&icns, PNG_DIR "/32x32.png");
  loaded_jp2 = test_load_cached(&icns, PNG_DIR "/32x32.j2k");
  compare = test_load_tga_cached(&icns,
    icns_format_ic11.width * icns_format_ic11.factor,
    icns_format_ic11.height * icns_format_ic11.factor,
    PNG_DIR "/32x32.tga.gz");

  /* PNG present -> return size of existing PNG */
  image->png = loaded_png->data;
  image->png_size = loaded_png->data_size;
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, loaded_png->data_size, "%zu != %zu", sz, loaded_png->data_size);
  ASSERT(IMAGE_IS_PNG(image), "");
  ASSERT(!IMAGE_IS_RAW(image), "");
  ASSERT(!IMAGE_IS_JPEG_2000(image), "");
  ASSERT(!IMAGE_IS_PIXELS(image), "");
  clear_image_no_free(image);

  /* JP2 present -> return size of existing JP2 */
  image->jp2 = loaded_jp2->data;
  image->jp2_size = loaded_jp2->data_size;
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, loaded_jp2->data_size, "%zu != %zu", sz, loaded_jp2->data_size);
  ASSERT(IMAGE_IS_JPEG_2000(image), "");
  ASSERT(!IMAGE_IS_RAW(image), "");
  ASSERT(!IMAGE_IS_PNG(image), "");
  ASSERT(!IMAGE_IS_PIXELS(image), "");
  clear_image_no_free(image);

  /* pixels present -> encode to new PNG that should decode to same pixels */
  image->pixels = compare->pixels;
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_ok(&icns, ret);
  ASSERT(IMAGE_IS_PIXELS(image), "");
  ASSERT(IMAGE_IS_PNG(image), "");
  ASSERTEQ(sz, image->png_size, "%zu != %zu", sz, image->png_size);
  ASSERT(!IMAGE_IS_RAW(image), "");
  ASSERT(!IMAGE_IS_JPEG_2000(image), "");
  image->pixels = NULL;
  ret = icns_decode_png_to_pixel_array(&icns, image, image->png, image->png_size);
  check_ok(&icns, ret);
  check_pixels(image, compare);
  icns_clear_image(image);

  /* pixels + JP2 -> return JP2 size */
  image->pixels = compare->pixels;
  image->jp2 = loaded_jp2->data;
  image->jp2_size = loaded_jp2->data_size;
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, loaded_jp2->data_size, "%zu != %zu", sz, loaded_jp2->data_size);
  ASSERT(IMAGE_IS_JPEG_2000(image), "");
  ASSERT(IMAGE_IS_PIXELS(image), "");
  ASSERT(!IMAGE_IS_RAW(image), "");
  ASSERT(!IMAGE_IS_PNG(image), "");

  /* ... + PNG -> return PNG size */
  image->png = loaded_png->data;
  image->png_size = loaded_png->data_size;
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, loaded_png->data_size, "%zu != %zu", sz, loaded_png->data_size);
  ASSERT(IMAGE_IS_PNG(image), "");
  ASSERT(IMAGE_IS_JPEG_2000(image), "");
  ASSERT(IMAGE_IS_PIXELS(image), "");
  ASSERT(!IMAGE_IS_RAW(image), "");
  clear_image_no_free(image);

  /* none present -> ICNS_INTERNAL_ERROR */
  ret = icns_image_prepare_png_for_icns(&icns, image, &sz);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  icns_clear_state_data(&icns);
}

UNITTEST(format_png_icns_image_write_pixel_array_to_png)
{
  struct icns_image *image;
  struct rgba_color *pixels;
  enum icns_error ret;
  const uint8_t *input;
  size_t sz;
  uint8_t *output_buffer;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  output_buffer = (uint8_t *)malloc(65536);
  ASSERT(output_buffer, "failed to alloc buffer");

  ret = icns_add_image_for_format(&icns, &image, NULL, &icns_format_ic11);
  check_ok(&icns, ret);

  /* 1. decode/write/decode should produce identical pixel array */
  sz = get_ic11_png(&icns, NULL);
  ret = icns_image_read_png(&icns, image, sz, ICNS_PNG_DECODE);
  check_ok(&icns, ret);
  icns_io_end(&icns);

  ret = icns_io_init_write_memory(&icns, output_buffer, 65536);
  check_ok(&icns, ret);
  ret = icns_image_write_pixel_array_to_png(&icns, image);
  check_ok(&icns, ret);

  pixels = image->pixels;
  image->pixels = NULL;
  ret = icns_decode_png_to_pixel_array(&icns, image, icns.io.ptr.src, icns.io.pos);
  check_ok(&icns, ret);
  ASSERTMEM(pixels, image->pixels,
    image->real_width * image->real_height * sizeof(struct rgba_color), "not identical");
  icns_clear_image(image);
  free(pixels);

  /* 2. write should pass through PNG */
  sz = get_ic11_png(&icns, &input);
  ret = icns_image_read_png(&icns, image, sz, ICNS_PNG_KEEP);
  check_ok(&icns, ret);
  icns_io_end(&icns);

  ret = icns_io_init_write_memory(&icns, output_buffer, 65536);
  check_ok(&icns, ret);
  ret = icns_image_write_pixel_array_to_png(&icns, image);
  check_ok(&icns, ret);

  ASSERTEQ(sz, icns.io.pos, "write out != PNG size");
  ASSERTMEM(input, icns.io.ptr.dest, sz, "not identical");
  icns_clear_image(image);

  /* 3. write should pass through JP2 */
  sz = get_ic11_jp2(&icns, &input);
  ret = icns_image_read_png(&icns, image, sz, ICNS_JP2_KEEP);
  check_ok(&icns, ret);
  icns_io_end(&icns);

  ret = icns_io_init_write_memory(&icns, output_buffer, 65536);
  check_ok(&icns, ret);
  ret = icns_image_write_pixel_array_to_png(&icns, image);
  check_ok(&icns, ret);

  ASSERTEQ(sz, icns.io.pos, "write out != JP2 size");
  ASSERTMEM(input, icns.io.ptr.dest, sz, "not identical");
  icns_clear_image(image);

  /* 4. write should fail with internal error if all of three are missing */
  ret = icns_image_write_pixel_array_to_png(&icns, image);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  icns_clear_state_data(&icns);
  free(output_buffer);
}

UNITTEST(format_icns_format_icp6)
{
  test_format_functions(&icns_format_icp6);
}

UNITTEST(format_icns_format_ic07)
{
  test_format_functions(&icns_format_ic07);
}

UNITTEST(format_icns_format_ic08)
{
  test_format_functions(&icns_format_ic08);
}

UNITTEST(format_icns_format_ic09)
{
  test_format_functions(&icns_format_ic09);
}

UNITTEST(format_icns_format_ic10)
{
  test_format_functions(&icns_format_ic10);
}

UNITTEST(format_icns_format_ic11)
{
  test_format_functions(&icns_format_ic11);
}

UNITTEST(format_icns_format_ic12)
{
  test_format_functions(&icns_format_ic12);
}

UNITTEST(format_icns_format_ic13)
{
  test_format_functions(&icns_format_ic13);
}

UNITTEST(format_icns_format_ic14)
{
  test_format_functions(&icns_format_ic14);
}

UNITTEST(format_icns_format_icsB)
{
  test_format_functions(&icns_format_icsB);
}

UNITTEST(format_icns_format_sb24)
{
  test_format_functions(&icns_format_sb24);
}

UNITTEST(format_icns_format_SB24)
{
  test_format_functions(&icns_format_SB24);
}
