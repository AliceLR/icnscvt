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

#include "icns_format.h"
#include "icns_image.h"

static struct icns_image *icns_alloc_image(const struct icns_format *format)
{
  struct icns_image *image;

  image = (struct icns_image *)malloc(sizeof(struct icns_image));
  if(!image)
    return NULL;

  memset(image, 0, sizeof(struct icns_image));

  image->format = format;
  image->real_width = format->width * format->factor;
  image->real_height = format->height * format->factor;
  return image;
}

/**
 * Clear all loaded image data for an image.
 * This function preserves the format of an image and its position in the
 * current image set. To delete an image, use `icns_delete_image_by_format`
 * instead.
 *
 * @param   image   image to clear data of.
 */
void icns_clear_image(struct icns_image *image)
{
  free(image->pixels);
  free(image->data);
  free(image->png);
  free(image->jp2);

  /* Only wipe storage fields; leave all other fields intact. */
  image->pixels = NULL;
  image->data = NULL;
  image->png = NULL;
  image->jp2 = NULL;
  image->data_size = 0;
  image->png_size = 0;
  image->jp2_size = 0;
}

/**
 * Allocate a pixel array of the appropriate size for a particular image.
 * This does not modify the provided image; the caller must replace the
 * image's pixel array with this buffer, if necessary.
 *
 * @param   image       image to create an RGBA pixel array for.
 * @return              RGBA pixel array of the correct size for this image.
 */
struct rgba_color *icns_allocate_pixel_array_for_image(
 const struct icns_image *image)
{
  const struct icns_format *format = image->format;
  size_t real_width = format->width * format->factor;
  size_t real_height = format->width * format->factor;
  size_t num_pixels = real_width * real_height;

  return (struct rgba_color *)malloc(num_pixels * sizeof(struct rgba_color));
}

/* Insert image into the images list. */
static enum icns_error icns_imageset_add_image(struct icns_data *icns,
 struct icns_image *image, struct icns_image *insert_after)
{
  struct icns_image_set *images = &icns->images;
  struct icns_image *tmp;

  if(insert_after)
  {
    /* Verify image exists in this image set. */
    bool ok = false;
    for(tmp = images->head; tmp; tmp = tmp->next)
    {
      if(tmp == insert_after)
      {
        ok = true;
        break;
      }
    }
    if(!ok)
    {
      E_("image to insert this image after does not exist in image set");
      return ICNS_INTERNAL_ERROR;
    }
  }
  else
    insert_after = images->tail;

  if(!insert_after)
  {
    /* First image in set. */
    images->head = images->tail = image;
    image->next = NULL;
    image->prev = NULL;
  }
  else
  {
    image->next = insert_after->next;
    image->prev = insert_after;
    insert_after->next = image;

    if(image->next)
      image->next->prev = image;
    else
      images->tail = image;
  }
  images->num_images++;

  return ICNS_OK;
}

/* Remove image from the images list, if it exists. */
static enum icns_error icns_imageset_remove_image(struct icns_data *icns,
 struct icns_image *image)
{
  struct icns_image_set *images = &icns->images;
  struct icns_image *tmp;

  if(!image)
  {
    E_("null image");
    return ICNS_INTERNAL_ERROR;
  }

  /* Find and remove image, if it exists. */
  for(tmp = images->head; tmp; tmp = tmp->next)
  {
    if(tmp != image)
      continue;

    if(tmp == images->head)
      images->head = tmp->next;
    if(tmp == images->tail)
      images->tail = tmp->prev;

    if(tmp->next)
      tmp->next->prev = tmp->prev;
    if(tmp->prev)
      tmp->prev->next = tmp->next;

    break;
  }
  images->num_images--;

  return ICNS_OK;
}

/**
 * Get the image data for a specific format, if it exists.
 *
 * @param   icns          current state data.
 * @param   format        format to get image for.
 * @return                pointer to the existing image for `format`, otherwise NULL.
 */
struct icns_image *icns_get_image_by_format(struct icns_data *icns,
 const struct icns_format *format)
{
  struct icns_image_set *images = &icns->images;
  struct icns_image *image;

  for(image = images->head; image; image = image->next)
    if(image->format == format)
      return image;

  return NULL;
}

/**
 * Add an image for a specific format, or return the existing image for that
 * format if it already exists.
 *
 * @param   icns          current state data.
 * @param   dest          pointer to return new (or existing) image data,
 *                        or NULL if this pointer isn't needed.
 * @param   insert_after  existing image to insert new image after, or NULL
 *                        to insert the new image at the end of the list.
 * @param   format        format to create (or get) image for.
 * @return                `ICNS_OK` on new image creation;
 *                        `ICNS_IMAGE_EXISTS_FOR_FORMAT` if it already exists;
 *                        otherwise, an icns_error value.
 */
enum icns_error icns_add_image_for_format(struct icns_data *icns,
 struct icns_image **dest, struct icns_image *insert_after,
 const struct icns_format *format)
{
  struct icns_image *image;
  enum icns_error ret;

  image = icns_get_image_by_format(icns, format);
  if(image)
  {
    if(dest)
      *dest = image;

    return ICNS_IMAGE_EXISTS_FOR_FORMAT;
  }

  image = icns_alloc_image(format);
  if(!image)
  {
    E_("failed to allocate image");
    return ICNS_ALLOC_ERROR;
  }

  ret = icns_imageset_add_image(icns, image, insert_after);
  if(ret)
  {
    E_("failed to insert image into set");
    free(image);
    return ret;
  }
  if(dest)
    *dest = image;

  return ICNS_OK;
}

/**
 * Delete an image for a specific format if it exists.
 *
 * @param   icns          current state data.
 * @param   format        format to delete an image for, if it exists.
 * @return                `ICNS_OK` on successful deletion;
 *                        `ICNS_NO_IMAGE` if no matching image exists;
 *                        otherwise, an icns_error value.
 */
enum icns_error icns_delete_image_by_format(struct icns_data *icns,
 const struct icns_format *format)
{
  struct icns_image *image;
  enum icns_error ret;

  image = icns_get_image_by_format(icns, format);
  if(!image)
    return ICNS_NO_IMAGE;

  ret = icns_imageset_remove_image(icns, image);
  if(ret)
  {
    E_("failed to remove image from set");
    return ret;
  }

  icns_clear_image(image);
  free(image);
  return ICNS_OK;
}

/**
 * Delete all images within the image set.
 *
 * @param   icns          current state data.
 */
void icns_delete_all_images(struct icns_data *icns)
{
  struct icns_image_set *images = &icns->images;
  struct icns_image *image;
  struct icns_image *next;

  for(image = images->head; image; image = next)
  {
    next = image->next;
    icns_clear_image(image);
    free(image);
  }
  images->head = NULL;
  images->tail = NULL;
}
