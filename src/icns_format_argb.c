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

#include "icns_format_argb.h"
#include "icns_format_mask.h"
#include "icns_format_png.h"
#include "icns_image.h"
#include "icns_io.h"
#include "icns_jp2.h"
#include "icns_png.h"

#include <stddef.h>

/**
 * Pack a single (A)RGB channel.
 */
static size_t icns_rle_pack_channel(uint8_t *dest, size_t dest_size,
 size_t dest_pos, const uint8_t *src, size_t src_count, size_t src_pitch)
{
  const uint8_t *tmp;
  size_t src_pos;
  size_t i;
  size_t num;
  uint8_t current;
  bool has_run = false;

  for(src_pos = 0; dest_pos < dest_size && src_pos < src_count; )
  {
    /* Find position of next RLE of length 3 or greater. */
    tmp = src;
    i = src_pos;
    num = 0;
    while(num <= 128 && i < src_count)
    {
      current = *tmp;
      if(i + 2 < src_count &&
         tmp[src_pitch] == current &&
         tmp[src_pitch + src_pitch] == current)
      {
        has_run = true;
        break;
      }
      tmp += src_pitch;
      i++;
      num++;
    }

    /* Emit block */
    if(num > 0)
    {
      if(dest_pos + num + 1 > dest_size)
        return 0;

      dest[dest_pos++] = num - 1;
      for(i = 0; i < num; i++)
      {
        dest[dest_pos++] = *src;
        src += src_pitch;
      }

      src_pos += num;
    }

    /* Emit run(s) */
    if(has_run)
    {
      current = *src;
      num = 0;
      while(src_pos < src_count)
      {
        if(*src != current)
          break;

        src += src_pitch;
        src_pos++;
        num++;
      }

      while(num > 0)
      {
        size_t n = num > 130 ? 130 : num;
        num -= n;

        if(dest_pos + 2 > dest_size)
          return 0;

        dest[dest_pos++] = n - 3 + 0x80;
        dest[dest_pos++] = current;
      }
      has_run = false;
    }
  }
  if(src_pos < src_count)
    return 0;

  return dest_pos;
}

/**
 * Pack an (A)RGB image into its corresponding ICNS packed encoding.
 * This function automatically takes consideration of alpha vs. non-alpha
 * and it32 padding bytes.
 */
static enum icns_error icns_image_pack_pixel_array_to_24_bit(
 struct icns_data *icns, struct icns_image *image)
{
  const struct icns_format *format = image->format;
  const struct rgba_color *pixels = image->pixels;
  const uint8_t *r;
  const uint8_t *g;
  const uint8_t *b;
  const uint8_t *a;
  uint8_t *data;
  size_t num_pixels = format->width * format->height;
  size_t dest_pos;
  bool is_alpha = (format->type == ICNS_ARGB_OR_PNG);
  bool padding = (format->magic == icns_magic_it32);

  size_t bound = (num_pixels + (num_pixels + 127) / 128);
  bound *= is_alpha ? 4 : 3;
  bound += padding ? 4 : 0;
  bound++;

  data = (uint8_t *)malloc(bound);
  if(!data)
  {
    E_("failed to alloc %s data array", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_ALLOC_ERROR;
  }

  r = ((const uint8_t *)pixels) + offsetof(struct rgba_color, r);
  g = ((const uint8_t *)pixels) + offsetof(struct rgba_color, g);
  b = ((const uint8_t *)pixels) + offsetof(struct rgba_color, b);

  dest_pos = 0;
  if(padding)
  {
    memset(data, 0, 4);
    dest_pos += 4;
  }

  if(is_alpha)
  {
    a = ((const uint8_t *)pixels) + offsetof(struct rgba_color, a);

    dest_pos = icns_rle_pack_channel(data, bound, dest_pos,
     a, num_pixels, 4);
    if(!dest_pos)
    {
      free(data);
      E_("failed to pack ARGB alpha channel");
      return ICNS_DATA_ERROR;
    }
  }
  dest_pos = icns_rle_pack_channel(data, bound, dest_pos,
   r, num_pixels, 4);
  if(!dest_pos)
  {
    free(data);
    E_("failed to pack %s red channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }
  dest_pos = icns_rle_pack_channel(data, bound, dest_pos,
   g, num_pixels, 4);
  if(!dest_pos)
  {
    free(data);
    E_("failed to pack %s green channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }
  dest_pos = icns_rle_pack_channel(data, bound, dest_pos,
   b, num_pixels, 4);
  if(!dest_pos)
  {
    free(data);
    E_("failed to pack %s blue channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }
  /* Extra byte to allegedly work around blue channel unpacking bugs. */
  if(dest_pos + 1 > bound)
  {
    free(data);
    E_("failed to add padding byte");
    return ICNS_DATA_ERROR;
  }
  data[dest_pos++] = 0;

  if(image->data)
    free(image->data);

  image->data = data;
  image->data_size = dest_pos;
  return ICNS_OK;
}

/**
 * Unpack a single (A)RGB channel.
 */
static size_t icns_rle_unpack_channel(uint8_t *dest, size_t dest_count,
 size_t dest_pitch, const uint8_t *src, size_t src_size, size_t src_pos)
{
  size_t dest_pos;
  size_t i;
  size_t num;

  for(dest_pos = 0; dest_pos < dest_count && src_pos < src_size; )
  {
    uint8_t pack_byte = src[src_pos++];
    uint8_t copy;
    if(pack_byte >= 0x80)
    {
      /* RLE */
      num = pack_byte - 0x80 + 3;
      if(src_pos + 1 > src_size || dest_pos + num > dest_count)
        break;

      copy = src[src_pos++];
      for(i = 0; i < num; i++, dest_pos++)
      {
        *dest = copy;
        dest += dest_pitch;
      }
    }
    else
    {
      /* Literal */
      num = pack_byte + 1;
      if(src_pos + num > src_size || dest_pos + num > dest_count)
        break;

      for(i = 0; i < num; i++, dest_pos++)
      {
        *dest = src[src_pos++];
        dest += dest_pitch;
      }
    }
  }

  if(dest_pos < dest_count)
    return 0;

  return src_pos;
}

/**
 * Unpack an (A)RGB image from its corresponding ICNS packed encoding.
 * This function automatically takes consideration of alpha vs. non-alpha
 * and it32 padding bytes.
 */
static enum icns_error icns_image_unpack_24_bit_to_pixel_array(
 struct icns_data *icns, struct icns_image *image)
{
  const struct icns_format *format = image->format;
  struct rgba_color *pixels;
  uint8_t *r;
  uint8_t *g;
  uint8_t *b;
  uint8_t *a;
  const uint8_t *data = image->data;
  size_t num_pixels = image->real_width * image->real_height;
  size_t src_pos;
  size_t i;
  bool is_alpha = (format->type == ICNS_ARGB_OR_PNG);
  bool padding = (format->magic == icns_magic_it32);

  pixels = icns_allocate_pixel_array_for_image(image);
  if(!pixels)
  {
    E_("failed to alloc pixels array");
    return ICNS_ALLOC_ERROR;
  }

  r = ((uint8_t *)pixels) + offsetof(struct rgba_color, r);
  g = ((uint8_t *)pixels) + offsetof(struct rgba_color, g);
  b = ((uint8_t *)pixels) + offsetof(struct rgba_color, b);

  src_pos = padding ? 4 : 0;
  if(is_alpha)
  {
    a = ((uint8_t *)pixels) + offsetof(struct rgba_color, a);

    src_pos = icns_rle_unpack_channel(a, num_pixels, 4,
     data, image->data_size, src_pos);
    if(!src_pos)
    {
      free(pixels);
      E_("failed to unpack ARGB alpha channel");
      return ICNS_DATA_ERROR;
    }
  }
  else
  {
    for(i = 0; i < num_pixels; i++)
      pixels[i].a = 255;
  }

  src_pos = icns_rle_unpack_channel(r, num_pixels, 4,
   data, image->data_size, src_pos);
  if(!src_pos)
  {
    free(pixels);
    E_("failed to unpack %s red channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }
  src_pos = icns_rle_unpack_channel(g, num_pixels, 4,
   data, image->data_size, src_pos);
  if(!src_pos)
  {
    free(pixels);
    E_("failed to unpack %s green channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }
  src_pos = icns_rle_unpack_channel(b, num_pixels, 4,
   data, image->data_size, src_pos);
  if(!src_pos)
  {
    free(pixels);
    E_("failed to unpack %s red channel", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }

  /* Allow for one extra byte at the end to work around apparent blue channel
   * unpacking bugs in some implementations. */
  if(src_pos < image->data_size - 1)
  {
    free(pixels);
    E_("invalid packed %s data stream", is_alpha ? "ARGB" : "24-bit RGB");
    return ICNS_DATA_ERROR;
  }

  if(image->pixels)
    free(image->pixels);

  image->pixels = pixels;
  return ICNS_OK;
}


/**
 * Copy mask to is32/il32/ih32/it32 from s8mk/l8mk/h8mk/t8mk.
 */
static enum icns_error icns_image_prepare_rgb_for_external(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  const struct icns_format *format = image->format;
  const struct icns_format *mask_format;
  struct icns_image *mask;
  enum icns_error ret;

  if(!IMAGE_IS_PIXELS(image))
  {
    E_("missing internal pixel array");
    return ICNS_INTERNAL_ERROR;
  }

  mask_format = icns_get_mask_for_format(format);
  if(!mask_format)
  {
    E_("mask should exist for this format!");
    return ICNS_INTERNAL_ERROR;
  }

  /* Mask may or may not exist in the source ICNS/.iconset. */
  mask = icns_get_image_by_format(icns, mask_format);
  if(!mask)
  {
    image->dirty_external = false;
    return ICNS_OK;
  }

  ret = icns_add_alpha_from_8_bit_mask(icns, image, mask);
  if(ret)
  {
    E_("failed to copy 8-bit mask into 24-bit RGB image");
    return ret;
  }
  image->dirty_external = false;
  return ICNS_OK;
}

/**
 * Copy mask from is32/il32/ih32/it32 to s8mk/l8mk/h8mk/t8mk, pack
 * (A)RGB, and return the final data size. This function ignores PNG
 * if it is present (some icon types have bugged support).
 */
static enum icns_error icns_image_prepare_rgb_for_icns(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t *sz)
{
  const struct icns_format *format = image->format;
  const struct icns_format *mask_format;
  struct icns_image *mask;
  bool is_alpha = (format->type == ICNS_ARGB_OR_PNG);
  enum icns_error ret;

  if(!IMAGE_IS_PIXELS(image))
  {
    E_("missing internal pixel array");
    return ICNS_INTERNAL_ERROR;
  }

  /* If a mask image exists but has no data, it needs to be generated from
   * this image's alpha channel. */
  mask_format = icns_get_mask_for_format(format);
  if(mask_format)
  {
    mask = icns_get_image_by_format(icns, mask_format);
    if(mask && !IMAGE_IS_RAW(mask))
    {
      ret = icns_split_alpha_to_8_bit_mask(icns, mask, image);
      if(ret)
      {
        E_("failed to copy 8-bit mask out of 24-bit RGB image");
        return ret;
      }
    }
  }

  /* Prepare (A)RGB. */
  ret = icns_image_pack_pixel_array_to_24_bit(icns, image);
  if(ret)
  {
    E_("failed to pack %s image", is_alpha ? "ARGB" : "24-bit RGB");
    return ret;
  }
  image->dirty_icns = false;
  *sz = image->data_size;
  return ICNS_OK;
}

static enum icns_error icns_image_prepare_icp4_icp5_for_icns(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t *sz)
{
  const struct icns_format *format = image->format;
  bool is_alpha = (format->type == ICNS_ARGB_OR_PNG);
  enum icns_error ret = ICNS_OK;

  /* If not in force raw mode, these formats are output as PNG to preserve
   * alpha information. */
  if(!icns->force_raw_if_available)
    return icns_image_prepare_png_for_icns(icns, image, sz);

  if(!IMAGE_IS_PIXELS(image))
  {
    E_("missing internal pixel array");
    return ICNS_INTERNAL_ERROR;
  }

  /* Prepare (A)RGB. */
  ret = icns_image_pack_pixel_array_to_24_bit(icns, image);
  if(ret)
  {
    E_("failed to pack %s image", is_alpha ? "ARGB" : "24-bit RGB");
    return ret;
  }
  image->dirty_icns = false;
  *sz = image->data_size;
  return ICNS_OK;
}


/* Load internal or iconset image that is always packed (A)RGB. */
static enum icns_error icns_image_read_pixel_array_from_argb(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t sz)
{
  uint8_t *data;
  enum icns_error ret;

  ret = icns_load_direct(icns, &data, sz);
  if(ret)
  {
    E_("failed to load RGB data");
    return ret;
  }
  icns_clear_image(image);
  image->data = data;
  image->data_size = sz;

  ret = icns_image_unpack_24_bit_to_pixel_array(icns, image);
  if(ret)
  {
    E_("failed to unpack (A)RGB image");
    icns_clear_image(image);
    return ret;
  }

  if(icns->force_recoding)
  {
    free(image->data);
    image->data = NULL;
  }
  return ICNS_OK;
}

/* Load internal image that may be PNG, JPEG 2000, or packed (A)RGB. */
static enum icns_error icns_image_read_pixel_array_from_png_or_argb(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t sz)
{
  enum icns_error ret;

  /* By default, keep all images; PNGs should always be decoded. */
  enum icns_image_read_png_options options =
   ICNS_PNG_DECODE_AND_KEEP | ICNS_JP2_KEEP | ICNS_RAW_KEEP;

  if(icns->force_recoding)
    options = ICNS_PNG_DECODE | ICNS_JP2_DECODE | ICNS_RAW_KEEP;

  ret = icns_image_read_png(icns, image, sz, options);
  if(ret)
  {
    E_("failed to load image");
    return ret;
  }

  if(IMAGE_IS_JPEG_2000(image) || IMAGE_IS_PNG(image) || IMAGE_IS_PIXELS(image))
  {
    /* Leave JPEG 2000 raw to allow export; PNG needs no further unpacking;
     * pixel array means this image was already unpacked from PNG or JP2. */
    return ICNS_OK;
  }

  /* If it's not PNG or JPEG 2000, it is packed (A)RGB (dependent on format). */
  ret = icns_image_unpack_24_bit_to_pixel_array(icns, image);
  if(ret)
  {
    E_("failed to unpack (A)RGB image");
    return ret;
  }
  return ICNS_OK;
}

/* Load from external PNG image (or JP2, if supported by this format). */
static enum icns_error icns_image_read_pixel_array_from_external(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  const struct icns_format *format = image->format;
  const struct icns_format *mask_format;
  enum icns_error ret;
  enum icns_image_read_png_options options =
   ICNS_PNG_DECODE_AND_KEEP | ICNS_JP2_KEEP;

  if(icns->force_recoding)
    options = ICNS_PNG_DECODE | ICNS_JP2_DECODE;

  ret = icns_image_read_png(icns, image, 0, options | ICNS_PNG_READ_FULL_STREAM);
  if(ret)
  {
    E_("failed to load image");
    return ret;
  }

  /* Make a mask image for this image in the image set.
   * If it already exists, this will do nothing. It may be
   * overwritten later by loading a mask directly. */
  mask_format = icns_get_mask_for_format(format);
  if(mask_format)
  {
    ret = icns_add_image_for_format(icns, NULL, image, mask_format);

    if(ret != ICNS_OK && ret != ICNS_IMAGE_EXISTS_FOR_FORMAT)
    {
      E_("failed to add mask image corresponding to this image");
      return ret;
    }
  }

  return ICNS_OK;
}

/* Write to internal or .iconset (A)RGB from a loaded pixel array. */
static enum icns_error icns_image_write_pixel_array_to_argb(
 struct icns_data * RESTRICT icns, const struct icns_image *image)
{
  enum icns_error ret;

  if(image->dirty_icns)
  {
    E_("image was not prepared for import");
    return ICNS_INTERNAL_ERROR;
  }

  if(!IMAGE_IS_RAW(image))
  {
    E_("missing internal raw (A)RGB data");
    return ICNS_INTERNAL_ERROR;
  }

  ret = icns_write_direct(icns, image->data, image->data_size);
  if(ret)
  {
    E_("failed to write raw (A)RGB data");
    return ret;
  }
  return ICNS_OK;
}

static enum icns_error icns_image_write_pixel_array_to_icp4_icp5(
 struct icns_data * RESTRICT icns, const struct icns_image *image)
{
  if(image->dirty_icns)
  {
    E_("image was not prepared for import");
    return ICNS_INTERNAL_ERROR;
  }

  if(icns->force_raw_if_available && IMAGE_IS_RAW(image))
  {
    enum icns_error ret = icns_write_direct(icns, image->data, image->data_size);
    if(ret)
    {
      E_("failed to write raw RGB data");
      return ret;
    }
    return ICNS_OK;
  }

  /* Must have a prepared PNG or JP2 for import instead--converting on the fly
   * isn't allowed here, as the size must be available when writing the TOC. */
  if(!IMAGE_IS_PNG(image) && !IMAGE_IS_JPEG_2000(image))
  {
    E_("no packed image available for import");
    return ICNS_INTERNAL_ERROR;
  }
  return icns_image_write_pixel_array_to_png(icns, image);
}


const struct icns_format icns_format_is32 =
{
  icns_magic_is32, "is32", "icon_data_is32",
  ICNS_24_BIT,
  16, 16, 1,
  0x850,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_argb,
  icns_image_write_pixel_array_to_argb,
  icns_image_prepare_rgb_for_external,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};


const struct icns_format icns_format_il32 =
{
  icns_magic_il32, "il32", "icon_data_il32",
  ICNS_24_BIT,
  32, 32, 1,
  0x850,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_argb,
  icns_image_write_pixel_array_to_argb,
  icns_image_prepare_rgb_for_external,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};


const struct icns_format icns_format_ih32 =
{
  icns_magic_ih32, "ih32", "icon_data_ih32",
  ICNS_24_BIT,
  48, 48, 1,
  0x850,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_argb,
  icns_image_write_pixel_array_to_argb,
  icns_image_prepare_rgb_for_external,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};


const struct icns_format icns_format_it32 =
{
  icns_magic_it32, "it32", "icon_data_it32",
  ICNS_24_BIT,
  128, 128, 1,
  0x1000,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_argb,
  icns_image_write_pixel_array_to_argb,
  icns_image_prepare_rgb_for_external,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};

/* PNG allegedly may be bugged, but write it anyway since RGB has no alpha. */
const struct icns_format icns_format_icp4 =
{
  icns_magic_icp4, "icp4", "icon_16x16.png",
  ICNS_24_BIT_OR_PNG,
  16, 16, 1,
  0x1070,
  icns_image_prepare_icp4_icp5_for_icns,
  icns_image_read_pixel_array_from_png_or_argb,
  icns_image_write_pixel_array_to_icp4_icp5,
  NULL,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};

/* PNG allegedly may be bugged, but write it anyway since RGB has no alpha. */
const struct icns_format icns_format_icp5 =
{
  icns_magic_icp5, "icp5", "icon_32x32.png",
  ICNS_24_BIT_OR_PNG,
  32, 32, 1,
  0x1070,
  icns_image_prepare_icp4_icp5_for_icns,
  icns_image_read_pixel_array_from_png_or_argb,
  icns_image_write_pixel_array_to_icp4_icp5,
  NULL,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};

/* PNG allegedly may be bugged so import as ARGB always. */
const struct icns_format icns_format_ic04 =
{
  icns_magic_ic04, "ic04", "icon_ic04.png",
  ICNS_ARGB_OR_PNG,
  16, 16, 1,
  0x1100,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_png_or_argb,
  icns_image_write_pixel_array_to_argb,
  NULL,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};

/* PNG allegedly may be bugged so import as ARGB always. */
const struct icns_format icns_format_ic05 =
{
  icns_magic_ic05, "ic05", "icon_ic05.png",
  ICNS_ARGB_OR_PNG,
  32, 32, 1,
  0x1100,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_png_or_argb,
  icns_image_write_pixel_array_to_argb,
  NULL,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};

/* PNG allegedly may be bugged so import as ARGB always. */
const struct icns_format icns_format_icsb =
{
  icns_magic_icsb, "icsb", "icon_18x18.png",
  ICNS_ARGB_OR_PNG,
  18, 18, 1,
  0x1100,
  icns_image_prepare_rgb_for_icns,
  icns_image_read_pixel_array_from_png_or_argb,
  icns_image_write_pixel_array_to_argb,
  NULL,
  icns_image_read_pixel_array_from_external,
  icns_image_write_pixel_array_to_png
};
