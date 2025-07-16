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

#include "icns_image.h"
#include "icns_io.h"
#include "icns_jp2.h"
#include "icns_png.h"

#include <setjmp.h>
#include <png.h>

static const uint8_t magic_png[8] =
{
  0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a
};

/**
 * Determine whether or not a buffer in memory is a PNG.
 *
 * @param data      pointer to data in memory.
 * @param data_size size of data in memory.
 * @return          true if the buffer contains a PNG, otherwise false.
 */
bool icns_is_file_png(const void *data, size_t data_size)
{
  if(data_size >= sizeof(magic_png))
    if(!memcmp(data, magic_png, sizeof(magic_png)))
      return true;

  return false;
}


/**
 * PNG reader/checker.
 */

#if PNG_LIBPNG_VER < 10504
#define png_set_scale_16(p) png_set_strip_16(p)
#endif

struct icns_png_reader_data
{
  const uint8_t *data;
  size_t pos;
  size_t size;
};

static void icns_png_error_fn(png_struct *png, const char *message)
{
  struct icns_data *icns = (struct icns_data *)png_get_error_ptr(png);
  E_("%s", message);
  png_longjmp(png, -1);
}

static void icns_png_warn_fn(png_struct *png, const char *message)
{
  struct icns_data *icns = (struct icns_data *)png_get_error_ptr(png);
  W_("%s\n", message);
}

static void icns_png_read_fn(png_struct *png, png_byte *dest, size_t count)
{
  struct icns_png_reader_data *reader =
   (struct icns_png_reader_data *)png_get_io_ptr(png);

  if(reader->pos > reader->size || reader->size - reader->pos < count)
    png_error(png, "eof");

  memcpy(dest, reader->data + reader->pos, count);
  reader->pos += count;
}

/**
 * Get the dimensions and bit depth of a PNG in a buffer without decoding
 * the entire image. This is intended for autodetecting the format for an
 * arbitrary input PNG.
 *
 * @param icns      current state data.
 * @param dest      the PNG header data will be stored to this pointer on
 *                  success. On failure, this struct will be unchanged.
 * @param png_data  pointer to PNG data in memory.
 * @param png_size  size of PNG data in memory.
 * @return          `ICNS_OK` on success;
 *                  `ICNS_PNG_INIT_ERROR` if libpng failed to init;
 *                  `ICNS_PNG_READ_ERROR` if libpng failed to read.
 */
enum icns_error icns_get_png_info(
 struct icns_data * RESTRICT icns, struct icns_png_stat * RESTRICT dest,
 const uint8_t *png_data, size_t png_size)
{
  struct icns_png_reader_data data = { png_data, 0, png_size };
  enum icns_error ret;

  png_struct *png = NULL;
  png_info *info = NULL;
  png_uint_32 w;
  png_uint_32 h;
  int bit_depth;
  int color_type;
  int interlace_type;

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
   icns, icns_png_error_fn, icns_png_warn_fn);
  if(!png)
  {
    E_("failed to create PNG read struct");
    return ICNS_PNG_INIT_ERROR;
  }

  info = png_create_info_struct(png);
  if(!info)
  {
    E_("failed to create PNG info struct");
    ret = ICNS_PNG_INIT_ERROR;
    goto error;
  }

  if(setjmp(png_jmpbuf(png)))
  {
    E_("failed to load png");
    ret = ICNS_PNG_READ_ERROR;
    goto error;
  }

  png_set_read_fn(png, &data, icns_png_read_fn);
  png_set_sig_bytes(png, 0);

  png_read_info(png, info);
  png_get_IHDR(png, info, &w, &h, &bit_depth,
   &color_type, &interlace_type, NULL, NULL);

  if(~color_type & PNG_COLOR_MASK_ALPHA)
  {
    /* Check for tRNS */
    png_byte *trans;
    int num_trans;
    png_color_16 *trans_values;

    png_get_tRNS(png, info, &trans, &num_trans, &trans_values);
    dest->has_trns = (trans && num_trans);
  }
  else
    dest->has_trns = false;

  dest->width = w;
  dest->height = h;
  dest->depth = bit_depth;
  dest->type = 0;
  dest->interlace = (interlace_type == PNG_INTERLACE_ADAM7);

  if(color_type & PNG_COLOR_MASK_PALETTE)
    dest->type |= ICNS_PNG_TYPE_IS_INDEXED;
  if(color_type & PNG_COLOR_MASK_COLOR)
    dest->type |= ICNS_PNG_TYPE_IS_COLOR;
  if(color_type & PNG_COLOR_MASK_ALPHA)
    dest->type |= ICNS_PNG_TYPE_IS_ALPHA;

  ret = ICNS_OK;

error:
  png_destroy_read_struct(&png, info ? &info : NULL, NULL);
  return ret;
}

NOT_NULL
static enum icns_error icns_decode_png(struct rgba_color **dest,
 struct icns_data * RESTRICT icns, const struct icns_image *image,
 const uint8_t *png_data, size_t png_size)
{
  struct rgba_color * volatile pixels = NULL;
  struct rgba_color *pos;
  size_t real_width = image->real_width;
  size_t real_height = image->real_height;
  struct icns_png_reader_data data = { png_data, 0, png_size };

  png_struct *png = NULL;
  png_info *info = NULL;
  png_uint_32 w;
  png_uint_32 h;
  png_uint_32 j;
  int bit_depth;
  int color_type;
  int interlace_type;
  int i;
  enum icns_error ret;

  *dest = NULL;

  png = png_create_read_struct(PNG_LIBPNG_VER_STRING,
   icns, icns_png_error_fn, icns_png_warn_fn);
  if(!png)
  {
    E_("failed to create PNG read struct");
    return ICNS_PNG_INIT_ERROR;
  }

  info = png_create_info_struct(png);
  if(!info)
  {
    E_("failed to create PNG info struct");
    ret = ICNS_PNG_INIT_ERROR;
    goto error;
  }

  if(setjmp(png_jmpbuf(png)))
  {
    E_("failed to load png");
    ret = ICNS_PNG_READ_ERROR;
    goto error;
  }

  png_set_read_fn(png, &data, icns_png_read_fn);
  png_set_sig_bytes(png, 0);

  png_read_info(png, info);
  png_get_IHDR(png, info, &w, &h, &bit_depth,
   &color_type, &interlace_type, NULL, NULL);

  if(w != real_width || h != real_height)
  {
    E_("PNG dimensions %" PRIu32 " x %" PRIu32 " don't match expected %zu x %zu",
     w, h, real_width, real_height);
    ret = ICNS_INVALID_DIMENSIONS;
    goto error;
  }

  pixels = icns_allocate_pixel_array_for_image(image);
  if(!pixels)
  {
    E_("failed to allocate pixel array");
    ret = ICNS_ALLOC_ERROR;
    goto error;
  }

  /* This SHOULD convert everything to RGBA32.
   * See the far too complicated table in libpng-manual.txt for more info. */
  if(bit_depth == 16)
    png_set_scale_16(png);
  if(color_type & PNG_COLOR_MASK_PALETTE)
    png_set_palette_to_rgb(png);
  if(!(color_type & PNG_COLOR_MASK_COLOR))
    png_set_gray_to_rgb(png);
#if PNG_LIBPNG_VER >= 10207
  if(!(color_type & PNG_COLOR_MASK_ALPHA))
    png_set_add_alpha(png, 0xff, PNG_FILLER_AFTER);
#endif
  if(png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  {
    /* silence -Wcobbered false positive */
    int num_passes = 1;

    if(interlace_type != PNG_INTERLACE_NONE)
      num_passes = png_set_interlace_handling(png);

    for(i = 0; i < num_passes; i++)
    {
      pos = pixels;
      for(j = 0; j < h; j++)
      {
        png_read_row(png, (png_bytep)pos, NULL);
        pos += w;
      }
    }
  }
  png_read_end(png, NULL);
  png_destroy_read_struct(&png, &info, NULL);

  *dest = pixels;
  return ICNS_OK;

error:
  png_destroy_read_struct(&png, info ? &info : NULL, NULL);

  free(pixels);
  return ret;
}

/**
 * Verify and decode a PNG in memory to an image's pixel array.
 * On success, this will clear all existing image data in the image.
 * On failure, the image will not be modified.
 *
 * @param icns      current state data.
 * @param image     image to generate a pixel array for.
 * @param png_data  pointer to PNG data in memory.
 * @param png_size  size of PNG data in memory.
 * @return          `ICNS_OK` on success;
 *                  `ICNS_PNG_INIT_ERROR` if libpng failed to init;
 *                  `ICNS_PNG_READ_ERROR` if libpng failed to read;
 *                  `ICNS_INVALID_DIMENSIONS` if the PNG doesn't match
 *                                            the format of the image;
 *                  `ICNS_ALLOC_ERROR` if the pixel array failed to allocate.
 */
enum icns_error icns_decode_png_to_pixel_array(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image,
 const uint8_t *png_data, size_t png_size)
{
  struct rgba_color *pixels = NULL;
  enum icns_error ret;

  ret = icns_decode_png(&pixels, icns, image, png_data, png_size);
  if(ret)
  {
    E_("failed to decode PNG to pixel array");
    return ret;
  }

  icns_clear_image(image);
  image->pixels = pixels;
  return ICNS_OK;
}


/**
 * PNG writer.
 */

static void icns_png_write_fn(png_struct *png, png_bytep src, size_t size)
{
  struct icns_data *icns = (struct icns_data *)png_get_io_ptr(png);

  if(icns->write_fn(src, size, icns) < size)
    png_error(png, "write error");
}

static void icns_png_flush_fn(png_struct *png)
{
  /* nop */
  (void)png;
}

/**
 * Encode a pixel array into a PNG and write it directly to the output stream.
 * On error, this may have written incomplete PNG data to the stream.
 *
 * @param icns      current state data.
 * @param pixels    pixel array to encode into a PNG.
 * @param width     width of pixel array, in real pixels.
 * @param height    height of pixel array, in real pixels.
 * @return          `ICNS_OK` on success;
 *                  `ICNS_PNG_INIT_ERROR` if libpng failed to init;
 *                  `ICNS_PNG_WRITE_ERROR` if libpng failed during write.
 */
enum icns_error icns_encode_png_to_stream(struct icns_data * RESTRICT icns,
 const struct rgba_color *pixels, size_t width, size_t height)
{
  const struct rgba_color *pos;
  png_struct *png = NULL;
  png_info *info = NULL;
  enum icns_error ret;
  size_t i;

  png = png_create_write_struct(PNG_LIBPNG_VER_STRING,
   icns, icns_png_error_fn, icns_png_warn_fn);
  if(!png)
  {
    E_("failed to create PNG write struct");
    return ICNS_PNG_INIT_ERROR;
  }

  info = png_create_info_struct(png);
  if(!info)
  {
    E_("failed to create PNG info struct");
    ret = ICNS_PNG_INIT_ERROR;
    goto error;
  }

  if(setjmp(png_jmpbuf(png)))
  {
    E_("failed to write png");
    ret = ICNS_PNG_WRITE_ERROR;
    goto error;
  }

  png_set_write_fn(png, icns, icns_png_write_fn, icns_png_flush_fn);

  png_set_IHDR(png, info, width, height, 8,
   PNG_COLOR_TYPE_RGB_ALPHA, PNG_INTERLACE_NONE,
   PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  pos = pixels;
  for(i = 0; i < height; i++)
  {
    png_write_row(png, (png_bytep)pos);
    pos += width;

  }
  png_write_end(png, info);
  png_destroy_write_struct(&png, &info);
  return ICNS_OK;

error:
  png_destroy_write_struct(&png, &info);
  return ret;
}
