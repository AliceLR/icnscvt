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
#include "../src/icns_io.h"
#include "../src/icns_format_argb.h"
#include "../src/icns_format_mask.h"

static void random_pixels(struct icns_image *image)
{
  struct rgba_color *pos;
  size_t sz;
  size_t i;

  icns_clear_image(image);
  image->pixels = icns_allocate_pixel_array_for_image(image);
  ASSERT(image->pixels, "failed to allocate pixel array");

  sz = image->real_width * image->real_height;
  pos = image->pixels;
  for(i = 0; i < sz; i++)
  {
    pos->r = rand();
    pos->g = rand();
    pos->b = rand();
    pos->a = rand();
    pos++;
  }
}

static void random_mask(struct icns_image *mask)
{
  uint8_t *pos;
  size_t sz;
  size_t i;

  sz = mask->real_width * mask->real_height;

  icns_clear_image(mask);
  mask->data = (uint8_t *)malloc(sz);
  mask->data_size = sz;
  ASSERT(mask->data, "failed to allocate pixel array");

  pos = mask->data;
  for(i = 0; i < sz; i++)
    *(pos++) = rand();
}

static size_t image_size(const struct icns_image *image,
 const struct icns_image *mask)
{
  ASSERT(image->pixels, "%s is missing pixel data", image->format->name);
  ASSERT(mask->data, "%s is missing mask data", mask->format->name);
  ASSERTEQ(image->real_width, mask->real_width,
    "%s incompatible with %s", image->format->name, mask->format->name);
  ASSERTEQ(image->real_height, mask->real_height,
    "%s incompatible with %s", image->format->name, mask->format->name);

  return image->real_width * image->real_height;
}

static void check_non_match(const struct icns_image *image,
 const struct icns_image *mask)
{
  size_t sz;
  size_t i;

  /* Automatically a non-match */
  if(!mask->data)
    return;

  sz = image_size(image, mask);
  for(i = 0; i < sz; i++)
    if(image->pixels[i].a != mask->data[i])
      break;

  ASSERT(i < sz, "%s alpha is identical to %s mask",
    image->format->name, mask->format->name);
}

static void check_match(const struct icns_image *image,
 const struct icns_image *mask)
{
  size_t sz = image_size(image, mask);
  size_t i;

  for(i = 0; i < sz; i++)
  {
    ASSERTEQ(image->pixels[i].a, mask->data[i],
      "%s[%zu] != %s[%zu]: %d != %d",
      image->format->name, i,
      mask->format->name, i,
      image->pixels[i].a, mask->data[i]);
  }
}


static const struct icns_format fmt_s8mk_rgb =
{
  icns_magic_is32, "is!fakeformat", "",
  ICNS_24_BIT,
  16, 16, 1,
  0, NULL, NULL, NULL, NULL, NULL, NULL
};

static const struct icns_format fmt_l8mk_rgb =
{
  icns_magic_il32, "il!fakeformat", "",
  ICNS_24_BIT,
  32, 32, 1,
  0, NULL, NULL, NULL, NULL, NULL, NULL
};

static const struct icns_format fmt_h8mk_rgb =
{
  icns_magic_ih32, "ih!fakeformat", "",
  ICNS_24_BIT,
  48, 48, 1,
  0, NULL, NULL, NULL, NULL, NULL, NULL
};

static const struct icns_format fmt_t8mk_rgb =
{
  icns_magic_it32, "it!fakeformat", "",
  ICNS_24_BIT,
  128, 128, 1,
  0, NULL, NULL, NULL, NULL, NULL, NULL
};

UNITTEST(format_mask_icns_add_alpha_from_8_bit_mask)
{
  enum icns_error ret;
  struct icns_image *s8mk;
  struct icns_image *l8mk;
  struct icns_image *h8mk;
  struct icns_image *t8mk;
  struct icns_image *s8mk_rgb;
  struct icns_image *l8mk_rgb;
  struct icns_image *h8mk_rgb;
  struct icns_image *t8mk_rgb;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ret = icns_add_image_for_format(&icns, &s8mk, NULL, &icns_format_s8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &l8mk, NULL, &icns_format_l8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &h8mk, NULL, &icns_format_h8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &t8mk, NULL, &icns_format_t8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &s8mk_rgb, NULL, &fmt_s8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &l8mk_rgb, NULL, &fmt_l8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &h8mk_rgb, NULL, &fmt_h8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &t8mk_rgb, NULL, &fmt_t8mk_rgb);
  check_ok(&icns, ret);

  random_mask(s8mk);
  random_mask(l8mk);
  random_mask(h8mk);
  /* random_mask(t8mk); */
  random_pixels(s8mk_rgb);
  random_pixels(l8mk_rgb);
  /* random_pixels(h8mk_rgb); */
  random_pixels(t8mk_rgb);

  /* image format != 24-bit -> internal error */
  ret = icns_add_alpha_from_8_bit_mask(&icns, s8mk, l8mk);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* mask format != mask -> internal error */
  ret = icns_add_alpha_from_8_bit_mask(&icns, s8mk_rgb, l8mk_rgb);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* non-matched dimensions -> internal error */
  ret = icns_add_alpha_from_8_bit_mask(&icns, s8mk_rgb, h8mk);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* image is missing pixel array -> internal error */
  ret = icns_add_alpha_from_8_bit_mask(&icns, h8mk_rgb, h8mk);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* mask is missing mask data -> internal error */
  ret = icns_add_alpha_from_8_bit_mask(&icns, t8mk_rgb, t8mk);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  random_mask(t8mk);
  random_pixels(h8mk_rgb);

  check_non_match(s8mk_rgb, s8mk);
  check_non_match(l8mk_rgb, l8mk);
  check_non_match(h8mk_rgb, h8mk);
  check_non_match(t8mk_rgb, t8mk);

  /* All real combinations should work */
  ret = icns_add_alpha_from_8_bit_mask(&icns, s8mk_rgb, s8mk);
  check_ok(&icns, ret);
  check_match(s8mk_rgb, s8mk);

  ret = icns_add_alpha_from_8_bit_mask(&icns, l8mk_rgb, l8mk);
  check_ok(&icns, ret);
  check_match(l8mk_rgb, l8mk);

  ret = icns_add_alpha_from_8_bit_mask(&icns, h8mk_rgb, h8mk);
  check_ok(&icns, ret);
  check_match(h8mk_rgb, h8mk);

  ret = icns_add_alpha_from_8_bit_mask(&icns, t8mk_rgb, t8mk);
  check_ok(&icns, ret);
  check_match(t8mk_rgb, t8mk);

  icns_delete_all_images(&icns);
}

UNITTEST(format_mask_icns_split_alpha_to_8_bit_mask)
{
  enum icns_error ret;
  struct icns_image *s8mk;
  struct icns_image *l8mk;
  struct icns_image *h8mk;
  struct icns_image *t8mk;
  struct icns_image *s8mk_rgb;
  struct icns_image *l8mk_rgb;
  struct icns_image *h8mk_rgb;
  struct icns_image *t8mk_rgb;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ret = icns_add_image_for_format(&icns, &s8mk, NULL, &icns_format_s8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &l8mk, NULL, &icns_format_l8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &h8mk, NULL, &icns_format_h8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &t8mk, NULL, &icns_format_t8mk);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &s8mk_rgb, NULL, &fmt_s8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &l8mk_rgb, NULL, &fmt_l8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &h8mk_rgb, NULL, &fmt_h8mk_rgb);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &t8mk_rgb, NULL, &fmt_t8mk_rgb);
  check_ok(&icns, ret);

  random_pixels(s8mk_rgb);
  random_pixels(l8mk_rgb);
  /* random_pixels(h8mk_rgb); */
  random_pixels(t8mk_rgb);

  /* image format != 24-bit -> internal error */
  ret = icns_split_alpha_to_8_bit_mask(&icns, s8mk, l8mk);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* mask format != mask -> internal error */
  ret = icns_split_alpha_to_8_bit_mask(&icns, s8mk_rgb, l8mk_rgb);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* non-matched dimensions -> internal error */
  ret = icns_split_alpha_to_8_bit_mask(&icns, s8mk, h8mk_rgb);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  /* image is missing pixel array -> internal error */
  ret = icns_split_alpha_to_8_bit_mask(&icns, h8mk, h8mk_rgb);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  random_pixels(h8mk_rgb);

  check_non_match(s8mk_rgb, s8mk);
  check_non_match(l8mk_rgb, l8mk);
  check_non_match(h8mk_rgb, h8mk);
  check_non_match(t8mk_rgb, t8mk);

  /* All real combinations should work */
  ret = icns_split_alpha_to_8_bit_mask(&icns, s8mk, s8mk_rgb);
  check_ok(&icns, ret);
  check_match(s8mk_rgb, s8mk);

  ret = icns_split_alpha_to_8_bit_mask(&icns, l8mk, l8mk_rgb);
  check_ok(&icns, ret);
  check_match(l8mk_rgb, l8mk);

  ret = icns_split_alpha_to_8_bit_mask(&icns, h8mk, h8mk_rgb);
  check_ok(&icns, ret);
  check_match(h8mk_rgb, h8mk);

  ret = icns_split_alpha_to_8_bit_mask(&icns, t8mk, t8mk_rgb);
  check_ok(&icns, ret);
  check_match(t8mk_rgb, t8mk);

  icns_delete_all_images(&icns);
}

UNITTEST(format_icns_format_s8mk)
{
  test_format_functions(&icns_format_s8mk);
}

UNITTEST(format_icns_format_l8mk)
{
  test_format_functions(&icns_format_l8mk);
}

UNITTEST(format_icns_format_h8mk)
{
  test_format_functions(&icns_format_h8mk);
}

UNITTEST(format_icns_format_t8mk)
{
  test_format_functions(&icns_format_t8mk);
}
