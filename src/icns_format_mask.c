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

#include <assert.h>

#include "icns_format.h"
#include "icns_format_mask.h"
#include "icns_format_png.h"
#include "icns_image.h"
#include "icns_io.h"
#include "icns_png.h"

/**
 * Copy an 8-bit mask over the alpha channel of a 24-bit RGB or ARGB image.
 *
 * @param icns      current state data.
 * @param rgb       RGB image. The alpha channel will be overwritten with
 *                  mask data. This image must have loaded pixel data.
 * @param mask      mask image. The mask data from this image will be copied
 *                  over the alpha channel of the RGB image.
 * @return          `INCS_OK` on success;
 *                  `ICNS_INTERNAL_ERROR` if `rgb` isn't 24-bit RGB, if `mask`
 *                  isn't an 8-bit mask, if `rgb` and `mask` have mismatched
 *                  dimensions, if `rgb` is missing a pixel array, or if
 *                  `mask` is missing mask data.
 */
enum icns_error icns_add_alpha_from_8_bit_mask(struct icns_data * RESTRICT icns,
 struct icns_image * RESTRICT rgb, const struct icns_image *mask)
{
  struct rgba_color *pixels = rgb->pixels;
  const uint8_t *m = mask->data;
  size_t sz = rgb->format->width * rgb->format->height;
  size_t i;

  if(rgb->format->type != ICNS_24_BIT ||
     mask->format->type != ICNS_8_BIT_MASK ||
     rgb->format->width != mask->format->width ||
     rgb->format->height != mask->format->height ||
     !IMAGE_IS_PIXELS(rgb) || !IMAGE_IS_RAW(mask) || mask->data_size != sz)
  {
    E_("image and mask formats are incompatible");
    return ICNS_INTERNAL_ERROR;
  }

  for(i = 0; i < sz; i++)
    pixels[i].a = m[i];

  return ICNS_OK;
}

/**
 * Generate an 8-bit mask from the alpha channel of a 24-bit RGB or ARGB image
 * into a second image. This destroys all data in the destination image.
 *
 * @param icns      current state data.
 * @param mask      mask image. Any data in this image will be replaced with
 *                  newly generated mask data, if it exists.
 * @param rgb       RGB image. The alpha channel will be used to generate the
 *                  new data for the mask.
 * @return          `INCS_OK` on success;
 *                  `ICNS_INTERNAL_ERROR` if `rgb` isn't 24-bit RGB, if `mask`
 *                  isn't an 8-bit mask, if `rgb` and `mask` have mismatched
 *                  dimensions, or if `rgb` is missing a pixel array.
 */
enum icns_error icns_split_alpha_to_8_bit_mask(struct icns_data * RESTRICT icns,
 struct icns_image * RESTRICT mask, const struct icns_image *rgb)
{
  const struct icns_format *format = rgb->format;
  const struct rgba_color *pixels = rgb->pixels;
  uint8_t *data;
  size_t num_pixels = format->width * format->height;
  size_t i;

  if(rgb->format->type != ICNS_24_BIT ||
     mask->format->type != ICNS_8_BIT_MASK ||
     rgb->format->width != mask->format->width ||
     rgb->format->height != mask->format->height ||
     !IMAGE_IS_PIXELS(rgb))
  {
    E_("image and mask formats are incompatible");
    return ICNS_INTERNAL_ERROR;
  }

  data = (uint8_t *)malloc(num_pixels);
  if(!data)
  {
    E_("failed to alloc 8-bit mask array");
    return ICNS_ALLOC_ERROR;
  }

  for(i = 0; i < num_pixels; i++)
    data[i] = pixels[i].a;

  icns_clear_image(mask);
  mask->data = data;
  mask->data_size = num_pixels;
  mask->format = icns_get_mask_for_format(format);
  return ICNS_OK;
}

/**
 * Generate an 8-bit mask from a pixel array for import.
 * If the image is fully opaque, use luma to get the mask value.
 * Otherwise, use the alpha value for the mask.
 */
static enum icns_error icns_generate_8_bit_mask_from_pixel_array(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  const struct rgba_color *pixels = image->pixels;
  uint8_t *data;
  size_t num_pixels = image->real_width * image->real_height;
  size_t i;
  bool use_alpha = false;

  if(!IMAGE_IS_PIXELS(image))
  {
    E_("missing pixel array");
    return ICNS_INTERNAL_ERROR;
  }

  /* Prescan--is it opaque or is it alpha? */
  for(i = 0; i < num_pixels; i++)
  {
    if(pixels[i].a < 255)
    {
      use_alpha = true;
      break;
    }
  }

  data = (uint8_t *)malloc(num_pixels);
  if(!data)
  {
    E_("failed to alloc 8-bit mask array");
    return ICNS_ALLOC_ERROR;
  }

  if(use_alpha)
  {
    for(i = 0; i < num_pixels; i++)
      data[i] = pixels[i].a;
  }
  else
  {
    for(i = 0; i < num_pixels; i++)
      data[i] = icns_get_luma_for_pixel(pixels[i]);
  }

  free(image->data);
  image->data = data;
  image->data_size = num_pixels;
  return ICNS_OK;
}

/**
 * Generate a greyscale fully opaque image from an 8-bit mask for PNG export.
 */
static enum icns_error icns_generate_pixel_array_from_8_bit_mask(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  struct rgba_color *pixels;
  struct rgba_color *pos;
  const uint8_t *src;
  size_t num_pixels = image->real_width * image->real_height;
  size_t i;

  if(!IMAGE_IS_RAW(image))
  {
    E_("missing internal 8-bit mask data");
    return ICNS_INTERNAL_ERROR;
  }

  pixels = icns_allocate_pixel_array_for_image(image);
  if(!pixels)
  {
    E_("failed to allocate pixel array for 8-bit mask");
    return ICNS_ALLOC_ERROR;
  }

  pos = pixels;
  src = image->data;
  for(i = 0; i < num_pixels; i++)
  {
    pos->r = pos->g = pos->b = *src;
    pos->a = 255;
    pos++;
    src++;
  }

  free(image->pixels);
  image->pixels = pixels;
  return ICNS_OK;
}


/**
 * 8-bit masks are generated at load time or from 24-bit images and
 * should never be pixel arrays.
 */
static enum icns_error icns_image_prepare_8_bit_mask_for_icns(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t *sz)
{
  if(!IMAGE_IS_RAW(image))
  {
    E_("missing internal 8-bit mask data");
    return ICNS_INTERNAL_ERROR;
  }
  image->dirty_icns = false;
  *sz = image->data_size;
  return ICNS_OK;
}

/**
 * 8-bit mask needs to be converted to RGBA for export.
 */
static enum icns_error icns_image_prepare_8_bit_mask_for_external(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  enum icns_error ret;

  ret = icns_generate_pixel_array_from_8_bit_mask(icns, image);
  if(ret)
  {
    E_("failed to prepare 8-bit mask for external");
    return ret;
  }
  image->dirty_external = false;
  return ICNS_OK;
}

/* New mask -> corresponding RGB export needs to prepare new alpha data. */
static void icns_image_mask_dirty_rgb(struct icns_data * RESTRICT icns,
 struct icns_image *RESTRICT mask)
{
  const struct icns_format *format;
  struct icns_image *image;

  format = icns_get_format_from_mask(mask->format);
  assert(format);
  if(format)
  {
    image = icns_get_image_by_format(icns, format);
    if(image)
      image->dirty_external = true;
  }
}

/* Load internal or iconset 8-bit mask that is always stored uncompressed.
 */
static enum icns_error icns_image_read_8_bit_mask_direct(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t sz)
{
  uint8_t *data;
  enum icns_error ret;

  if(sz != image->real_width * image->real_height)
  {
    E_("mask size %zu invalid; expected %zu",
     sz, image->real_width * image->real_height);
    return ICNS_DATA_ERROR;
  }

  ret = icns_load_direct(icns, &data, sz);
  if(ret)
  {
    E_("failed to load raw 8-bit mask");
    return ret;
  }

  icns_image_mask_dirty_rgb(icns, image);
  icns_clear_image(image);
  image->data = data;
  image->data_size = sz;
  return ICNS_OK;
}

/* Load 8-bit mask from an external PNG image.
 * If it has alpha data, use the alpha data to generate the mask.
 * If it does not have alpha data, use luma to generate the mask.
 * Usually, this should be imported/exported via its corresponding
 * IS32/IL32/IH32/IT32 instead.
 */
static enum icns_error icns_image_read_8_bit_mask_from_external(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image)
{
  enum icns_error ret;
  uint8_t *data;
  size_t sz;

  ret = icns_load_direct_auto(icns, &data, &sz);
  if(ret)
  {
    E_("failed to load PNG for mask");
    return ret;
  }

  ret = icns_decode_png_to_pixel_array(icns, image, data, sz);
  /* Destroy (do not keep) the PNG data. */
  free(data);
  if(ret)
  {
    E_("failed to decode PNG to pixel array");
    return ret;
  }

  ret = icns_generate_8_bit_mask_from_pixel_array(icns, image);
  if(ret)
  {
    E_("failed to generate 8-bit mask from pixel array");
    return ret;
  }

  /* Destroy stored pixel array--it is not guaranteed to match the preferred
   * output RGBA encoding, so it's better to just regenerate it. */
  free(image->pixels);
  image->pixels = NULL;

  icns_image_mask_dirty_rgb(icns, image);
  return ICNS_OK;
}

static enum icns_error icns_image_write_8_bit_mask_direct(
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
    E_("image is missing 8-bit mask data");
    return ICNS_INTERNAL_ERROR;
  }

  ret = icns_write_direct(icns, image->data, image->data_size);
  if(ret)
  {
    E_("failed to write 8-bit mask");
    return ret;
  }
  return ICNS_OK;
}


const struct icns_format icns_format_s8mk =
{
  icns_magic_s8mk, "s8mk", "icon_data_s8mk",
  ICNS_8_BIT_MASK,
  16, 16, 1,
  0x850,
  icns_image_prepare_8_bit_mask_for_icns,
  icns_image_read_8_bit_mask_direct,
  icns_image_write_8_bit_mask_direct,
  icns_image_prepare_8_bit_mask_for_external,
  icns_image_read_8_bit_mask_from_external,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_l8mk =
{
  icns_magic_l8mk, "l8mk", "icon_data_l8mk",
  ICNS_8_BIT_MASK,
  32, 32, 1,
  0x850,
  icns_image_prepare_8_bit_mask_for_icns,
  icns_image_read_8_bit_mask_direct,
  icns_image_write_8_bit_mask_direct,
  icns_image_prepare_8_bit_mask_for_external,
  icns_image_read_8_bit_mask_from_external,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_h8mk =
{
  icns_magic_h8mk, "h8mk", "icon_data_h8mk",
  ICNS_8_BIT_MASK,
  48, 48, 1,
  0x850,
  icns_image_prepare_8_bit_mask_for_icns,
  icns_image_read_8_bit_mask_direct,
  icns_image_write_8_bit_mask_direct,
  icns_image_prepare_8_bit_mask_for_external,
  icns_image_read_8_bit_mask_from_external,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_t8mk =
{
  icns_magic_t8mk, "t8mk", "icon_data_t8mk",
  ICNS_8_BIT_MASK,
  128, 128, 1,
  0x1000,
  icns_image_prepare_8_bit_mask_for_icns,
  icns_image_read_8_bit_mask_direct,
  icns_image_write_8_bit_mask_direct,
  icns_image_prepare_8_bit_mask_for_external,
  icns_image_read_8_bit_mask_from_external,
  icns_image_write_pixel_array_to_png
};
