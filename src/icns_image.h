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

#ifndef ICNSCVT_IMAGE_H
#define ICNSCVT_IMAGE_H

#include "common.h"

/* Functions for manipulating icns_image instances and for managing
 * individual images inside of a loaded ICNS image set. */

ICNS_BEGIN_DECLS

struct rgba_color
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
  uint8_t a;
};

struct icns_image
{
#define IMAGE_IS_RAW(img)       ((img)->data != NULL)
#define IMAGE_IS_PIXELS(img)    ((img)->pixels != NULL)
#define IMAGE_IS_PNG(img)       ((img)->png != NULL)
#define IMAGE_IS_JPEG_2000(img) ((img)->jp2 != NULL)

  struct icns_image *next;
  struct icns_image *prev;
  const struct icns_format *format;
  size_t real_width;
  size_t real_height;

  struct rgba_color *pixels;
  uint8_t *data;
  uint8_t *png;
  uint8_t *jp2;
  size_t data_size;
  size_t png_size;
  size_t jp2_size;

  bool dirty_external;
  bool dirty_icns;
};

/* Get the apparent brightness (luma) for an RGBX pixel. */
static inline uint8_t icns_get_luma_for_pixel(struct rgba_color pixel)
{
  unsigned total =
   (unsigned)pixel.r * 306u +
   (unsigned)pixel.g * 601u +
   (unsigned)pixel.b * 117u;

  return (total + 512u) / 1024u;
}

void icns_clear_image(struct icns_image *image) NOT_NULL;

struct rgba_color *icns_allocate_pixel_array_for_image(
 const struct icns_image *image) NOT_NULL;

struct icns_image *icns_get_image_by_format(struct icns_data *icns,
 const struct icns_format *format) NOT_NULL;
enum icns_error icns_add_image_for_format(struct icns_data *icns,
 struct icns_image **dest, struct icns_image *insert_after,
 const struct icns_format *format) NOT_NULL_2(1,4);
enum icns_error icns_delete_image_by_format(struct icns_data *icns,
 const struct icns_format *format) NOT_NULL;
void icns_delete_all_images(struct icns_data *icns) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_IMAGE_H */
