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

#include "target.h"
#include "targa.h"
#include "format.h"
#include "../src/icns_image.h"
#include "../src/icns_png.h"
#include "../src/icns_format_argb.h"

/* Decode an exported PNG and compare its pixels. */
#define check_decode(image, data, data_size) do { \
  struct icns_image chk; \
  enum icns_error r; \
  memset(&chk, 0, sizeof(chk)); \
  chk.format = (image)->format; \
  chk.real_width = (image)->real_width; \
  chk.real_height = (image)->real_height; \
  r = icns_decode_png_to_pixel_array(icns, &chk, (data), (data_size)); \
  ASSERTEQ(r, ICNS_OK, "%zu: %d != %d", i, r, ICNS_OK); \
  check_pixels(&chk, image); \
  icns_clear_image(&chk); \
  } while(0)

/* Quickly load all external success files to prepare for the save sequence. */
static void load_all_skip_errors(icnscvt context)
{
  size_t i;
  int ret;

  for(i = 0; i < num_external_load_sequence; i++)
  {
    const struct target_test *s = &external_load_sequence[i];
    if(s->api_expected == 0)
    {
      /* Prefer the filesystem function since this is already going to
       * duplicate most of these files to RAM, and will not benefit from
       * the caching features of test_load_cached. */
#ifndef ICNSCVT_NO_FILESYSTEM
      ret = icnscvt_load_image_from_external_file(context, s->format, s->file);
#else
      uint8_t *buf;
      size_t sz;
      test_load((struct icns_data *)icns, &buf, &sz, s->file);

      ret = icnscvt_load_image_from_external_memory(context, s->format,
       loaded->data, loaded->data_size);
      free(buf);
#endif
      ASSERTEQ(ret, 0, "%zu: %d != 0", i, ret);
    }
  }
}


UNITTEST(icnscvt_load_image_from_external_memory)
{
  icnscvt context = NULL;
  struct icns_data compare;
  struct icns_data *icns;
  uint8_t *buf;
  size_t sz;
  size_t i;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_load_image_from_external_memory(context,
    icns_magic_is32, "absdfbds", 8);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  /* Error on junk context. */
  ret = icnscvt_load_image_from_external_memory((icnscvt)&compare,
    icns_magic_is32, "absdfbds", 8);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  icns = (struct icns_data *)context;

  /* Error if the memory buffer is NULL. */
  ret = icnscvt_load_image_from_external_memory(context, icns_magic_is32, NULL, 32);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

#if SIZE_MAX > UINT32_MAX
  /* Error if the format ID isn't a uint32_t. */
  ret = icnscvt_load_image_from_external_memory(context, SIZE_MAX, "absdfbds", 8);
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
#endif

  for(i = 0; i < num_external_load_sequence; i++)
  {
    const struct target_test *s = &external_load_sequence[i];
    if(s->expected == ICNS_READ_OPEN_ERROR)
      continue;

    test_load(icns, &buf, &sz, s->file);

    ret = icnscvt_load_image_from_external_memory(context, s->format, buf, sz);
    free(buf);
    ASSERTEQ(ret, s->api_expected,
      "%zu: %d != %d", i, ret, (int)s->api_expected);

    if(ret == 0)
    {
      struct icns_image *image = get_image_by_magic(icns, s->format_loaded);
      ASSERT(image, "%zu", i);

      ASSERT(IMAGE_IS_PNG(image) || IMAGE_IS_JPEG_2000(image) ||
             IMAGE_IS_RAW(image) || IMAGE_IS_PIXELS(image), "%zu", i);
    }
  }
  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_save_image_to_external_memory)
{
  UNIMPLEMENTED();
}


UNITTEST(icnscvt_load_image_from_external_callback)
{
  UNIMPLEMENTED();
}

UNITTEST(icnscvt_save_image_to_external_callback)
{
  UNIMPLEMENTED();
}


#ifndef ICNSCVT_NO_FILESYSTEM

UNITTEST(icnscvt_load_image_from_external_file)
{
  icnscvt context = NULL;
  struct icns_data compare;
  struct icns_data *icns;
  size_t i;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_load_image_from_external_file(context,
    icns_magic_is32, PNG_DIR "/16x16.png");
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  /* Error on junk context. */
  ret = icnscvt_load_image_from_external_file((icnscvt)&compare,
    icns_magic_is32, PNG_DIR "/16x16.png");
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  icns = (struct icns_data *)context;

#if SIZE_MAX > UINT32_MAX
  /* Error if the format ID isn't a uint32_t. */
  ret = icnscvt_load_image_from_external_file(context, SIZE_MAX, "fdsd");
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
#endif

  for(i = 0; i < num_external_load_sequence; i++)
  {
    const struct target_test *s = &external_load_sequence[i];

    ret = icnscvt_load_image_from_external_file(context, s->format, s->file);
    ASSERTEQ(ret, s->api_expected,
      "%zu: %d != %d", i, ret, (int)s->api_expected);

    if(ret == 0)
    {
      struct icns_image *image = get_image_by_magic(icns, s->format_loaded);
      ASSERT(image, "%zu", i);

      ASSERT(IMAGE_IS_PNG(image) || IMAGE_IS_JPEG_2000(image) ||
             IMAGE_IS_RAW(image) || IMAGE_IS_PIXELS(image), "%zu", i);
    }
  }
  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_save_image_to_external_file)
{
  icnscvt context = NULL;
  struct icns_data compare;
  struct icns_data *icns;
  struct icns_image *image;
  size_t i;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_save_image_to_external_file(context,
    icns_magic_is32, PNG_DIR "/16x16.png");
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  /* Error on junk context. */
  ret = icnscvt_save_image_to_external_file((icnscvt)&compare,
    icns_magic_is32, PNG_DIR "/16x16.png");
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  icns = (struct icns_data *)context;

#if SIZE_MAX > UINT32_MAX
  /* Error if the format ID isn't a uint32_t. */
  ret = icnscvt_save_image_to_external_file(context, SIZE_MAX, "fdsd");
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
#endif

  load_all_skip_errors(context);

  /* Attempt to resave all of the successful loads. */
  for(i = 0; i < num_external_save_sequence; i++)
  {
    const struct target_test *s = &external_save_sequence[i];

    /* This may generate pixels if the image didn't already have them. */
    ret = icnscvt_save_image_to_external_file(context, s->format, TEMP_DIR "/out.png");
    ASSERTEQ(ret, s->api_expected,
      "%zu: %d != %d", i, ret, (int)s->api_expected);

    if(ret == 0)
    {
      /* Saved image should decode to an identical pixel array. */
      uint8_t *buf;
      size_t sz;

      image = get_image_by_magic(icns, s->format);
      ASSERT(image, "%zu", i);
      if(!IMAGE_IS_PIXELS(image))
        continue;

      test_load(icns, &buf, &sz, TEMP_DIR "/out.png");
      check_decode(image, buf, sz);
      free(buf);
    }
  }

  icnscvt_destroy_context(context);
}

#endif
