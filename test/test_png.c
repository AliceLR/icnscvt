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
#include "../src/icns.h"
#include "../src/icns_format.h"
#include "../src/icns_image.h"
#include "../src/icns_io.h"
#include "../src/icns_png.h"

#include <png.h>

struct test_png
{
  const char *path;
  const char *compare;
  struct icns_png_stat st;
};

static const struct test_png png_types[] =
{
  { PNG_DIR "/16x16_adam7.png",
    PNG_DIR "/16x16.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_RGB_INDEXED, true, true }},
  { PNG_DIR "/16x16_gs1.png",
    PNG_DIR "/16x16_gs1.tga.gz",
    {   16,   16,  1, ICNS_PNG_TYPE_GREY, false, false }},
  { PNG_DIR "/16x16_gs8.png",
    PNG_DIR "/16x16_gs8.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_GREY, false, false }},
  { PNG_DIR "/16x16_gs16.png",
    PNG_DIR "/16x16_gs8.tga.gz",
    {   16,   16, 16, ICNS_PNG_TYPE_GREY, false, false }},
  { PNG_DIR "/16x16_gsa8.png",
    PNG_DIR "/16x16_gsa8.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},
  { PNG_DIR "/16x16_gsa16.png",
    PNG_DIR "/16x16_gsa8.tga.gz",
    {   16,   16, 16, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},
  { PNG_DIR "/16x16_i1.png",
    PNG_DIR "/16x16_i1.tga.gz",
    {   16,   16,  1, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/16x16_i2.png",
    PNG_DIR "/16x16_i2.tga.gz",
    {   16,   16,  2, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/16x16_i4.png",
    PNG_DIR "/16x16_i4.tga.gz",
    {   16,   16,  4, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/16x16_i8.png",
    PNG_DIR "/16x16_i8.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/16x16_rgba.png",
    PNG_DIR "/16x16.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_RGBA, false, false }},
  { PNG_DIR "/16x16_rgba16.png",
    PNG_DIR "/16x16.tga.gz",
    {   16,   16, 16, ICNS_PNG_TYPE_RGBA, false, false }},
};
#define num_png_types (sizeof(png_types) / sizeof(png_types[0]))

static const struct test_png png_formats[] =
{
  /* Monochrome */
  { PNG_DIR "/32x32_system1.png",
    PNG_DIR "/32x32_system1.tga.gz",
    {   32,   32,  1, ICNS_PNG_TYPE_GREY, false, false }},

  /* Monochrome (masked) */
  { PNG_DIR "/16x12_1bpp.png",
    PNG_DIR "/16x12_1bpp.tga.gz",
    {   16,   12,  8, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},
  { PNG_DIR "/16x16_1bpp.png",
    PNG_DIR "/16x16_1bpp.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},
  { PNG_DIR "/32x32_1bpp.png",
    PNG_DIR "/32x32_1bpp.tga.gz",
    {   32,   32,  8, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},
  { PNG_DIR "/48x48_1bpp.png",
    PNG_DIR "/48x48_1bpp.tga.gz",
    {   48,   48,  8, ICNS_PNG_TYPE_GREY_ALPHA, false, false }},

  /* 4-bit indexed */
  { PNG_DIR "/16x12_4bpp.png",
    PNG_DIR "/16x12_4bpp.tga.gz",
    {   16,   12,  4, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/16x16_4bpp.png",
    PNG_DIR "/16x16_4bpp.tga.gz",
    {   16,   16,  4, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/32x32_4bpp.png",
    PNG_DIR "/32x32_4bpp.tga.gz",
    {   32,   32,  4, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/48x48_4bpp.png",
    PNG_DIR "/48x48_4bpp.tga.gz",
    {   48,   48,  4, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},

  /* 8-bit indexed */
  { PNG_DIR "/16x12_8bpp.png",
    PNG_DIR "/16x12_8bpp.tga.gz",
    {   16,   12,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/16x16_8bpp.png",
    PNG_DIR "/16x16_8bpp.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/32x32_8bpp.png",
    PNG_DIR "/32x32_8bpp.tga.gz",
    {   32,   32,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},
  { PNG_DIR "/48x48_8bpp.png",
    PNG_DIR "/48x48_8bpp.tga.gz",
    {   48,   48,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, false }},

  /* True color */
  { PNG_DIR "/16x16.png",
    PNG_DIR "/16x16.tga.gz",
    {   16,   16,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/18x18.png",
    PNG_DIR "/18x18.tga.gz",
    {   18,   18,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/24x24.png",
    PNG_DIR "/24x24.tga.gz",
    {   24,   24,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/32x32.png",
    PNG_DIR "/32x32.tga.gz",
    {   32,   32,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/36x36.png",
    PNG_DIR "/36x36.tga.gz",
    {   36,   36,  8, ICNS_PNG_TYPE_RGBA, false, false }},
  { PNG_DIR "/48x48.png",
    PNG_DIR "/48x48.tga.gz",
    {   48,   48,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/64x64.png",
    PNG_DIR "/64x64.tga.gz",
    {   64,   64,  8, ICNS_PNG_TYPE_RGB_INDEXED, false, true }},
  { PNG_DIR "/128x128.png",
    PNG_DIR "/128x128.tga.gz",
    {  128,  128,  8, ICNS_PNG_TYPE_RGBA, false, false }},
  { PNG_DIR "/256x256.png",
    PNG_DIR "/256x256.tga.gz",
    {  256,  256,  8, ICNS_PNG_TYPE_RGBA, false, false }},
  { PNG_DIR "/512x512.png",
    PNG_DIR "/512x512.tga.gz",
    {  512,  512,  8, ICNS_PNG_TYPE_RGBA, false, false }},
  { PNG_DIR "/1024x1024.png",
    PNG_DIR "/1024x1024.tga.gz",
    { 1024, 1024,  8, ICNS_PNG_TYPE_RGBA, false, false }},
};
#define num_png_formats (sizeof(png_formats) / sizeof(png_formats[0]))

static const char *not_pngs[] =
{
  PNG_DIR      "/Makefile",
  PNG_DIR      "/128x128.tga.gz",
  PNG_DIR      "/1024x1024.tga.gz",
  INTERNAL_DIR "/test.raw",
  INTERNAL_DIR "/test.raw.gz",
  INTERNAL_DIR "/test.tga",
  INTERNAL_DIR "/test.tga.gz",
  INTERNAL_DIR "/text.txt",
  INTERNAL_DIR "/text.txt.gz",
  DIRENT_DIR   "/a_file",
  DIRENT_DIR   "/README",
};
#define num_not_pngs (sizeof(not_pngs) / sizeof(not_pngs[0]))


UNITTEST(png_icns_is_file_png)
{
  bool ret;
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_png_types; i++)
  {
    const struct test_png *png = png_types + i;
    const struct loaded_file *loaded = test_load_cached(&icns, png->path);
    ret = icns_is_file_png(loaded->data, loaded->data_size);
    ASSERTEQ(ret, true, "'%s' should be PNG", png->path);
  }

  for(i = 0; i < num_png_formats; i++)
  {
    const struct test_png *png = png_formats + i;
    const struct loaded_file *loaded = test_load_cached(&icns, png->path);
    ret = icns_is_file_png(loaded->data, loaded->data_size);
    ASSERTEQ(ret, true, "'%s' should be PNG", png->path);
  }

  for(i = 0; i < num_not_pngs; i++)
  {
    const char *not_png = not_pngs[i];
    const struct loaded_file *loaded = test_load_cached(&icns, not_png);
    ret = icns_is_file_png(loaded->data, loaded->data_size);
    ASSERTEQ(ret, false, "'%s' should NOT be PNG", not_png);
  }

  /* Shorter than magic */
  ret = icns_is_file_png("\x89PNG\r\n\x1a", 7);
  ASSERTEQ(ret, false, "shorter than magic should fail");

  /* 0-length */
  ret = icns_is_file_png("", 0);
  ASSERTEQ(ret, false, "0 length should fail");

  test_load_cached_cleanup();
}


NOT_NULL
static void test_png_stat(struct icns_data * RESTRICT icns,
 const struct test_png *png)
{
  enum icns_error ret;
  struct icns_png_stat st;
  const struct loaded_file *loaded = test_load_cached(icns, png->path);

  ret = icns_get_png_info(icns, &st, loaded->data, loaded->data_size);
  ASSERTEQ(ret, ICNS_OK, "'%s': failed icsn_get_png_info", png->path);
  check_ok(icns, ret);

  ASSERTEQ(st.width, png->st.width, "'%s': width %u != %u",
   png->path, st.width, png->st.width);
  ASSERTEQ(st.height, png->st.height, "'%s': height %u != %u",
   png->path, st.height, png->st.height);
  ASSERTEQ(st.depth, png->st.depth, "'%s': depth %d != %d",
   png->path, st.depth, png->st.depth);
  ASSERTEQ(st.type, png->st.type, "'%s': type %d != %d",
   png->path, st.type, png->st.type);
  ASSERTEQ(st.interlace, png->st.interlace, "'%s': interlace %d != %d",
   png->path, st.interlace, png->st.interlace);
  ASSERTEQ(st.has_trns, png->st.has_trns, "'%s': has_trns %d != %d",
   png->path, st.has_trns, png->st.has_trns);
}

NOT_NULL
static void test_png_stat_not_a_png(struct icns_data * RESTRICT icns,
 const char *filename)
{
  enum icns_error ret;
  struct icns_png_stat st;
  struct icns_png_stat cmp;
  const struct loaded_file *loaded = test_load_cached(icns, filename);

  memset(&st, 0xff, sizeof(st));
  cmp = st;

  ret = icns_get_png_info(icns, &st, loaded->data, loaded->data_size);
  check_error(icns, ret, ICNS_PNG_READ_ERROR);

  ASSERTMEM(&st, &cmp, sizeof(st), "'%s': stat struct should be unmodified", filename);
}

UNITTEST(png_icns_get_png_info)
{
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_png_types; i++)
    test_png_stat(&icns, png_types + i);

  for(i = 0; i < num_png_formats; i++)
    test_png_stat(&icns, png_formats + i);

  for(i = 0; i < num_not_pngs; i++)
    test_png_stat_not_a_png(&icns, not_pngs[i]);

  test_load_cached_cleanup();
}


NOT_NULL
static void test_png_decode(struct icns_data * RESTRICT icns,
 const struct test_png *png, enum icns_error expected)
{
  enum icns_error ret;
  const struct loaded_file *loaded = test_load_cached(icns, png->path);
  const struct loaded_file *compare = test_load_tga_cached(icns,
    png->st.width, png->st.height, png->compare);

  const struct icns_format tmp_format =
  {
    0, "tmp ", "tmp",
    ICNS_PNG,
    png->st.width, png->st.height, 1,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  };
  struct icns_image *image;

  ret = icns_add_image_for_format(icns, &image, NULL, &tmp_format);
  check_ok(icns, ret);

  ret = icns_decode_png_to_pixel_array(icns, image, loaded->data, loaded->data_size);
  if(expected == ICNS_OK)
  {
    check_ok(icns, ret);

    ASSERTMEM(image->pixels, compare->pixels,
      image->real_width * image->real_height * sizeof(struct rgba_color),
      "'%s': pixel data mismatch", png->path);
  }
  else
    check_error(icns, ret, expected);

  icns_clear_state_data(icns);
}

UNITTEST(png_icns_decode_png_to_pixel_array)
{
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_png_types; i++)
    test_png_decode(&icns, png_types + i, ICNS_OK);

  for(i = 0; i < num_png_formats; i++)
    test_png_decode(&icns, png_formats + i, ICNS_OK);

  for(i = 0; i < num_not_pngs; i++)
    test_png_stat_not_a_png(&icns, not_pngs[i]);

  test_load_cached_cleanup();
}


NOT_NULL
static void test_png_encode_and_decode(struct icns_data * RESTRICT icns,
 const struct test_png *png, bool to_stream)
{
  enum icns_error ret;
  const struct loaded_file *compare = test_load_tga_cached(icns,
    png->st.width, png->st.height, png->compare);

  const struct icns_format tmp_format =
  {
    0, "tmp ", "tmp",
    ICNS_PNG,
    png->st.width, png->st.height, 1,
    0,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
  };
  struct icns_image *image;

  const size_t png_alloc = 1 << 16;
  uint8_t *png_data;
  size_t png_size;

  ret = icns_add_image_for_format(icns, &image, NULL, &tmp_format);
  check_ok(icns, ret);

  if(to_stream)
  {
    png_data = (uint8_t *)malloc(png_alloc);
    ASSERT(png_data, "failed to alloc buffer");

    ret = icns_io_init_write_memory(icns, png_data, png_alloc);
    check_ok(icns, ret);

    ret = icns_encode_png_to_stream(icns, compare->pixels, compare->w, compare->h);
    check_ok(icns, ret);

    png_size = icns->io.pos;
    icns_io_end(icns);
  }
  else
  {
    ret = icns_encode_png_to_buffer(icns, &png_data, &png_size,
      compare->pixels, compare->w, compare->h);
    check_ok(icns, ret);
  }

  ret = icns_decode_png_to_pixel_array(icns, image, png_data, png_size);
  free(png_data);
  check_ok(icns, ret);

  ASSERTMEM(image->pixels, compare->pixels,
    image->real_width * image->real_height * sizeof(struct rgba_color),
    "'%s': pixel data mismatch", png->path);

  icns_clear_state_data(icns);
}

UNITTEST(png_icns_encode_png_to_stream)
{
  enum icns_error ret;
#define pixels_dim 16
  struct rgba_color pixels[pixels_dim * pixels_dim];
  uint8_t buffer[64];
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_png_types; i++)
    test_png_encode_and_decode(&icns, png_types + i, true);

  for(i = 0; i < num_png_formats; i++)
    test_png_encode_and_decode(&icns, png_formats + i, true);

  /* Buffer too small -> should ICNS_PNG_WRITE_ERROR. */
  memset(pixels, 0, sizeof(pixels));
  ret = icns_io_init_write_memory(&icns, buffer, sizeof(buffer));
  check_ok(&icns, ret);
  ret = icns_encode_png_to_stream(&icns, pixels, pixels_dim, pixels_dim);
  check_error(&icns, ret, ICNS_PNG_WRITE_ERROR);
  icns_io_end(&icns);

  test_load_cached_cleanup();
}

UNITTEST(png_icns_encode_png_to_buffer)
{
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_png_types; i++)
    test_png_encode_and_decode(&icns, png_types + i, false);

  for(i = 0; i < num_png_formats; i++)
    test_png_encode_and_decode(&icns, png_formats + i, false);

  test_load_cached_cleanup();
}
