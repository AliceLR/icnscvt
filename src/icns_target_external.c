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

#include "common.h"
#include "icns.h"
#include "icns_format.h"
#include "icns_image.h"
#include "icns_io.h"
#include "icns_jp2.h"
#include "icns_png.h"

static enum icns_error icns_stat_external_image(
  struct icns_data * RESTRICT icns, const uint8_t *data, size_t data_size,
  unsigned *width, unsigned *height, unsigned *depth)
{
  struct icns_png_stat png_st;
  struct icns_jp2_stat jp2_st;
  enum icns_error ret;

  ret = icns_get_png_info(icns, &png_st, data, data_size);
  if(ret == ICNS_OK)
  {
    *width = png_st.width;
    *height = png_st.height;
    *depth = png_st.depth;
    return ICNS_OK;
  }
  if(ret != ICNS_PNG_NOT_A_PNG)
  {
    E_("failed to stat PNG");
    return ret;
  }

  ret = icns_get_jp2_info(icns, &jp2_st, data, data_size);
  if(ret == ICNS_OK)
  {
    *width = jp2_st.width;
    *height = jp2_st.height;
    *depth = jp2_st.depth;
    return ICNS_OK;
  }
  if(ret != ICNS_JP2_NOT_A_JP2)
  {
    E_("failed to stat JP2");
    return ret;
  }

  E_("image is not a PNG or JP2");
  return ICNS_PNG_NOT_A_PNG;
}

static enum icns_error icns_load_image_from_external(
  struct icns_data * RESTRICT icns, uint32_t format_id)
{
  const struct icns_format *format;
  struct icns_image *image;
  enum icns_error ret;

  /* Special: attempt to autodetect a format. */
  if(format_id == 0)
  {
    uint8_t *tmp = NULL;
    size_t sz = 0;
    unsigned w;
    unsigned h;
    unsigned d;

    ret = icns_load_direct_auto(icns, &tmp, &sz);
    icns_io_end(icns);
    if(ret)
    {
      E_("failed to read stream");
      return ret;
    }

    ret = icns_stat_external_image(icns, tmp, sz, &w, &h, &d);
    if(ret)
    {
      free(tmp);
      E_("could not stat external image to autodetect format");
      return ret;
    }

    format = icns_get_format_by_attributes(w, h, d, 0);
    if(!format || format->magic == 0)
    {
      free(tmp);
      E_("could not autodetect a format for image of %ux%u (%u-bit)", w, h, d);
      return ret;
    }

    ret = icns_io_init_read_memory(icns, tmp, sz);
    if(ret)
    {
      free(tmp);
      E_("failed to open new stream");
      return ret;
    }
    W_("autodetect: %ux%u (%u-bit) -> format '%s'", w, h, d, format->name);

    ret = icns_load_image_from_external(icns, format->magic);
    free(tmp);
    return ret;
  }
  else
    format = icns_get_format_by_magic(format_id);

  if(!format)
  {
    E_("unknown format %08" PRIx32, format_id);
    return ICNS_UNKNOWN_FORMAT;
  }

  ret = icns_add_image_for_format(icns, &image, NULL, format);
  if(ret != ICNS_OK && ret != ICNS_IMAGE_EXISTS_FOR_FORMAT)
  {
    E_("can't add image");
    return ret;
  }

  ret = format->read_from_external(icns, image);
  if(ret)
    E_("failed to load image from external");

  image->dirty_external = true;
  image->dirty_icns = true;
  return ret;
}

static enum icns_error icns_save_image_to_external(
  struct icns_data * RESTRICT icns, uint32_t format_id)
{
  const struct icns_format *format;
  struct icns_image *image;
  enum icns_error ret;

  format = icns_get_format_by_magic(format_id);
  if(!format)
  {
    E_("can't find format for image");
    return ICNS_UNKNOWN_FORMAT;
  }

  image = icns_get_image_by_format(icns, format);
  if(!image)
  {
    E_("can't add image");
    return ICNS_NO_IMAGE;
  }

  ret = format->write_to_external(icns, image);
  if(ret)
    E_("failed to save image to external");

  return ret;
}

static enum icns_error icns_prepare_images_for_external(
  struct icns_data * RESTRICT icns)
{
  struct icns_image_set *images = &icns->images;
  struct icns_image *image;
  enum icns_error ret;

  for(image = images->head; image; image = image->next)
  {
    const struct icns_format *format = image->format;

    if(!image->dirty_external)
      continue;

    if(format->prepare_for_external)
    {
      ret = format->prepare_for_external(icns, image);
      if(ret)
      {
        E_("failed to prepare image '%s' for external", format->name);
        return ret;
      }
    }
    image->dirty_external = false;
  }
  return ICNS_OK;
}


/**
 * FIXME:
 */
enum icns_error icns_load_image_from_external_memory(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const uint8_t *src, size_t src_size)
{
  enum icns_error ret;

  ret = icns_io_init_read_memory(icns, src, src_size);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_load_image_from_external(icns, format_id);
  if(ret)
    E_("failed to load image from external memory");

  icns_io_end(icns);
  return ret;
}

/**
 * FIXME:
 */
enum icns_error icns_save_image_to_external_memory(
  struct icns_data * RESTRICT icns, uint32_t format_id, size_t *total_out,
  uint8_t * RESTRICT dest, size_t dest_size)
{
  enum icns_error ret;

  ret = icns_prepare_images_for_external(icns);
  if(ret)
  {
    E_("failed to prepare images for external memory");
    return ret;
  }

  ret = icns_io_init_write_memory(icns, dest, dest_size);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_save_image_to_external(icns, format_id);
  if(ret == ICNS_OK)
    *total_out = icns->io.pos;
  else
    E_("failed to save image to external memory");

  icns_io_end(icns);
  return ret;
}

/**
 * FIXME:
 */
enum icns_error icns_load_image_from_external_callback(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  void *priv, size_t (*read_func)(void *dest, size_t sz, void *priv))
{
  enum icns_error ret;

  ret = icns_io_init_read(icns, priv, read_func);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_load_image_from_external(icns, format_id);
  if(ret)
    E_("failed to load image from external callback");

  icns_io_end(icns);
  return ret;
}

/**
 * FIXME:
 */
enum icns_error icns_save_image_to_external_callback(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  void *priv, size_t (*write_func)(const void *src, size_t sz, void *priv))
{
  enum icns_error ret;

  ret = icns_prepare_images_for_external(icns);
  if(ret)
  {
    E_("failed to prepare images for external callback");
    return ret;
  }

  ret = icns_io_init_write(icns, priv, write_func);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_save_image_to_external(icns, format_id);
  if(ret)
    E_("failed to save image to external callback");

  icns_io_end(icns);
  return ret;
}

#ifndef ICNSCVT_NO_FILESYSTEM

/**
 * FIXME:
 */
enum icns_error icns_load_image_from_external_file(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const char *filename)
{
  enum icns_error ret;

  ret = icns_io_init_read_file(icns, filename);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_load_image_from_external(icns, format_id);
  if(ret)
    E_("failed to load image from external file");

  icns_io_end(icns);
  return ret;
}

/**
 * FIXME:
 */
enum icns_error icns_save_image_to_external_file(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const char *filename)
{
  enum icns_error ret;

  ret = icns_prepare_images_for_external(icns);
  if(ret)
  {
    E_("failed to prepare images for external file");
    return ret;
  }

  ret = icns_io_init_write_file(icns, filename);
  if(ret)
  {
    E_("failed to init IO");
    return ret;
  }

  ret = icns_save_image_to_external(icns, format_id);
  if(ret)
    E_("failed to save image to external file");

  icns_io_end(icns);
  return ret;
}

#endif /* ICNS_NO_FILESYSTEM */
