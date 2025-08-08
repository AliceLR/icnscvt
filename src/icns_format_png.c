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

#include "icns_format_png.h"
#include "icns_image.h"
#include "icns_io.h"
#include "icns_jp2.h"
#include "icns_png.h"

/* PNG direct read/write formats.
 *
 * For these formats, PNG is loaded directly to the png or jp2 fields of the
 * `icns_image`. This is only decoded to a pixel array if required or if
 * requested by the caller, in which case, it needs to be re-encoded. */

/**
 * Prepare an image to be written to ICNS as a PNG. This will (re)encode the
 * output PNG if required or pass through the image's current PNG or JP2
 * data (if present).
 *
 * @param icns      current state data.
 * @param image     target image to prepare ICNS data for.
 * @param sz        on success, the total output data size is written to the
 *                  memory referenced by this pointer.
 * @return          `ICNS_OK` on success;
 *                  `ICNS_INTERNAL_ERROR` if no PNG, JP2, or pixel data is
 *                  present; possibly other errors on PNG encode failure.
 */
enum icns_error icns_image_prepare_png_for_icns(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image,
 size_t *sz)
{
  enum icns_error ret;

  if(IMAGE_IS_PNG(image))
  {
    *sz = image->png_size;
    return ICNS_OK;
  }
  else

  if(IMAGE_IS_JPEG_2000(image))
  {
    *sz = image->jp2_size;
    return ICNS_OK;
  }
  else

  if(IMAGE_IS_PIXELS(image))
  {
    /* Recode pixels into a new PNG. */
    uint8_t *data;
    size_t data_size;

    ret = icns_encode_png_to_buffer(icns, &data, &data_size,
      image->pixels, image->real_width, image->real_height);
    if(ret)
    {
      E_("failed to encode pixels to PNG");
      return ret;
    }

    image->png = data;
    image->png_size = data_size;
    *sz = data_size;
    return ICNS_OK;
  }

  E_("missing internal PNG or JPEG 2000 data");
  return ICNS_INTERNAL_ERROR;
}

/**
 * Generic function to load a PNG from the input stream into an image.
 * If the image format supports JPEG 2000, this function can also load a
 * JPEG 2000, but it is not currently capable of decoding it. This function
 * can be configured to keep unrecognized raw input or to force PNG decoding
 * (even if normally this format would be handled as a direct PNG copy).
 *
 * @param icns      current state data.
 * @param image     target image to load PNG/JP2/raw data to.
 * @param sz        expected size of PNG/JP2 data within stream (mandatory).
 * @param options   bitmask of options specified by `icns_image_read_png_options`
 * @return          `ICNS_OK` on success;
 *                  `ICNS_INVALID_DIMENSIONS` if the image has the wrong
 *                  dimensions for the target image's format;
 *                  `ICNS_UNIMPLEMENTED_FORMAT` if the operation specified
 *                  by the provided options isn't supported;
 *                  `ICNS_DATA_ERROR` if the image is not a PNG/JP2 and the
 *                  option `ICNS_PNG_ALLOW_RAW_DATA` wasn't provided;
 *                  other errors if input IO or PNG decoding fails.
 */
enum icns_error icns_image_read_png(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t sz,
 enum icns_image_read_png_options options)
{
  uint8_t *data;
  enum icns_error ret;
  bool allow_png = !!(options & ICNS_PNG_MASK);
  bool allow_jp2 = !!(options & ICNS_JP2_MASK) &&
   icns_format_supports_jpeg_2000(image->format);
  bool allow_raw = !!(options & ICNS_RAW_MASK);

  ret = icns_load_direct(icns, &data, sz);
  if(ret)
  {
    E_("failed to load data:%s%s%s",
      allow_png ? " PNG" : "",
      allow_jp2 ? " JPEG 2000" : "",
      allow_raw ? " (A)RGB" : "");
    return ret;
  }

  if(icns_is_file_jp2(data, sz))
  {
    /* Verify a small amount of information for this
     * JPEG 2000 to filter out any junk at the very least. */
    struct icns_jp2_stat st;
    if(!allow_jp2)
      goto bad_format;

    ret = icns_get_jp2_info(icns, &st, data, sz);
    if(ret)
    {
      free(data);
      E_("failed to verify JPEG 2000 data");
      return ret;
    }

    if(st.width != image->real_width || st.height != image->real_height)
    {
      free(data);
      E_("JP2 dimensions %" PRIu32 " x %" PRIu32 " don't match expected %zu x %zu",
       st.width, st.height, image->real_width, image->real_height);
      return ICNS_INVALID_DIMENSIONS;
    }

    if(options & ICNS_JP2_DECODE)
    {
      /* FIXME: decoding requires libopenjp2 */
      free(data);
      E_("can't decode JPEG 2000 image (requires libopenjp2)");
      return ICNS_UNIMPLEMENTED_FORMAT;
    }
    else
      icns_clear_image(image);

    if(options & ICNS_JP2_KEEP)
    {
      image->jp2 = data;
      image->jp2_size = sz;
    }
    else
      free(data);

    return ICNS_OK;
  }

  if(icns_is_file_png(data, sz))
  {
    if(!allow_png)
      goto bad_format;

    /* Decoding the PNG acts as a validation check, so always do it. */
    ret = icns_decode_png_to_pixel_array(icns, image, data, sz);
    if(ret)
    {
      free(data);
      E_("PNG data failed checks");
      return ret;
    }

    /* FIXME: if macOS can't display this PNG, force to ICNS_PNG_DECODE */

    /* Discard the decoded pixel array if it wasn't requested by the caller.
     * This was already done by icns_decode_png_to_pixel_array otherwise. */
    if(!(options & ICNS_PNG_DECODE))
      icns_clear_image(image);

    if(options & ICNS_PNG_KEEP)
    {
      image->png = data;
      image->png_size = sz;
    }
    else
      free(data);

    return ICNS_OK;
  }

  if(allow_raw)
  {
    /* This is up to the caller to verify, since it is most likely packed. */
    icns_clear_image(image);
    image->data = data;
    image->data_size = sz;
    return ICNS_OK;
  }

bad_format:
  E_("file format is not one of:%s%s%s",
    allow_png ? " PNG" : "",
    allow_jp2 ? " JPEG 2000" : "",
    allow_raw ? " (A)RGB" : "");
  free(data);
  return ICNS_DATA_ERROR;
}

static enum icns_error icns_image_read_png_direct(
 struct icns_data * RESTRICT icns, struct icns_image * RESTRICT image, size_t sz)
{
  enum icns_image_read_png_options options = ICNS_PNG_KEEP | ICNS_JP2_KEEP;

  if(icns->force_recoding)
    options = ICNS_PNG_DECODE | ICNS_JP2_DECODE;

  return icns_image_read_png(icns, image, sz, options);
}

static enum icns_error icns_image_write_png_direct(
 struct icns_data * RESTRICT icns, const struct icns_image *image)
{
  enum icns_error ret;

  const uint8_t *data;
  size_t data_size;

  if(IMAGE_IS_PNG(image))
  {
    data = image->png;
    data_size = image->png_size;
  }
  else

  if(IMAGE_IS_JPEG_2000(image))
  {
    data = image->jp2;
    data_size = image->jp2_size;
  }
  else
  {
    E_("missing internal PNG or JPEG 2000 data");
    return ICNS_INTERNAL_ERROR;
  }

  ret = icns_write_direct(icns, data, data_size);
  if(ret)
  {
    E_("failed to write PNG data");
    return ret;
  }
  return ICNS_OK;
}

/**
 * Used by most formats to export external PNG files. This will keep PNG or
 * JP2 data, if present, and otherwise write a newly encoded PNG directly to
 * the output stream.
 *
 * @param icns      current state data.
 * @param image     image to write PNG data from. This image should have
 *                  associated PNG, JP2, or pixel array data.
 * @return          `ICNS_OK` on success, otherwise an error code.
 */
enum icns_error icns_image_write_pixel_array_to_png(
 struct icns_data * RESTRICT icns, const struct icns_image *image)
{
  const struct rgba_color *pixels = image->pixels;
  size_t width = image->real_width;
  size_t height = image->real_height;

  /* Write PNG direct if original PNG data is present (RGB/ARGB). */
  if(IMAGE_IS_PNG(image) || IMAGE_IS_JPEG_2000(image))
    return icns_image_write_png_direct(icns, image);

  if(!IMAGE_IS_PIXELS(image))
  {
    E_("missing pixel array");
    return ICNS_INTERNAL_ERROR;
  }
  return icns_encode_png_to_stream(icns, pixels, width, height);
}


const struct icns_format icns_format_icp6 =
{
  icns_magic_icp6, "icp6", "icon_48x48.png",
  ICNS_PNG,
  48, 48, 1,
  0x1070,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic07 =
{
  icns_magic_ic07, "ic07", "icon_128x128.png",
  ICNS_PNG,
  128, 128, 1,
  0x1070,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic08 =
{
  icns_magic_ic08, "ic08", "icon_256x256.png",
  ICNS_PNG,
  256, 256, 1,
  0x1050,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic09 =
{
  icns_magic_ic09, "ic09", "icon_512x512.png",
  ICNS_PNG,
  512, 512, 1,
  0x1050,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic10 =
{
  icns_magic_ic10, "ic10", "icon_512x512@2.png",
  ICNS_PNG,
  512, 512, 2,
  0x1070,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic11 =
{
  icns_magic_ic11, "ic11", "icon_16x16@2.png",
  ICNS_PNG,
  16, 16, 2,
  0x1080,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic12 =
{
  icns_magic_ic12, "ic12", "icon_32x32@2.png",
  ICNS_PNG,
  32, 32, 2,
  0x1080,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic13 =
{
  icns_magic_ic13, "ic13", "icon_128x128@2.png",
  ICNS_PNG,
  128, 128, 2,
  0x1080,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_ic14 =
{
  icns_magic_ic14, "ic14", "icon_256x256@2.png",
  ICNS_PNG,
  256, 256, 2,
  0x1080,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_icsB =
{
  icns_magic_icsB, "icsB", "icon_18x18@2.png",
  ICNS_PNG,
  18, 18, 2,
  0x1100,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_sb24 =
{
  icns_magic_sb24, "sb24", "icon_24x24.png",
  ICNS_PNG,
  24, 24, 1,
  0x1100,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};

const struct icns_format icns_format_SB24 =
{
  icns_magic_SB24, "SB24", "icon_24x24@2.png",
  ICNS_PNG,
  24, 24, 2,
  0x1100,
  icns_image_prepare_png_for_icns,
  icns_image_read_png_direct,
  icns_image_write_png_direct,
  NULL,
  icns_image_read_png_direct,
  icns_image_write_pixel_array_to_png
};
