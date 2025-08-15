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
#include "../src/icns.h"
#include "../src/icns_format.h"
#include "../src/icns_image.h"

#define test_magic_abcd MAGIC('a','b','c','d')
#define test_magic_ABCE MAGIC('A','B','C','E')
#define test_magic_Baad MAGIC('B','a','a','d')
#define test_magic_d00d MAGIC('d','0','0','d')

/* Placeholder formats--image management doesn't require real format
 * structs, it just needs valid dimensions and unique addresses to
 * key images to. */
const struct icns_format format_abcd =
{
  test_magic_abcd, "abcd", "abcd.png",
  ICNS_PNG,
  128, 128, 1,
  0x1000,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const struct icns_format format_ABCE =
{
  test_magic_ABCE, "ABCE", "ABCE.png",
  ICNS_PNG,
  16, 16, 1,
  0x1000,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const struct icns_format format_Baad =
{
  test_magic_Baad, "Baad", "Baad.png",
  ICNS_PNG,
  16, 16, 2,
  0x1000,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

const struct icns_format format_d00d =
{
  test_magic_d00d, "d00d", "d00d.png",
  ICNS_PNG,
  128, 128, 2,
  0x1000,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

UNITTEST(image_icns_get_luma_for_pixel)
{
  static const struct rgba_color rgb_luma[] =
  {
    /*                 0.299
     *                 0.587
     *              x  0.144 */
    {   0,   0,   0,   0 },
    { 255, 255, 255, 255 },
    {  85, 170, 255, 154 },
    { 255,  85, 170, 146 },
    { 170, 255,  85, 210 },
    {   0,  85, 170,  69 },
    { 170,   0,  85,  61 },
    {  85, 170,   0, 125 },
    { 255,   0,   0,  76 },
    {   0, 255,   0, 150 },
    {   0,   0, 255,  29 },
  };
  size_t num_colors = sizeof(rgb_luma) / sizeof(rgb_luma[0]);
  size_t i;

  for(i = 0; i < num_colors; i++)
  {
    uint8_t luma = icns_get_luma_for_pixel(rgb_luma[i]);
    ASSERTEQ(luma, rgb_luma[i].a, "%zu: %d != %d", i, luma, rgb_luma[i].a);
  }
}

UNITTEST(image_is_macros)
{
  struct rgba_color pixel = { 0, 0, 0, 0 };
  uint8_t raw = 0;
  uint8_t raw2 = 0;
  uint8_t raw3 = 0;

  struct icns_image image;
  memset(&image, 0, sizeof(image));

  ASSERTEQ(IMAGE_IS_RAW(&image),        false, "");
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     false, "");
  ASSERTEQ(IMAGE_IS_PNG(&image),        false, "");
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  false, "");

  image.data = &raw;
  ASSERTEQ(IMAGE_IS_RAW(&image),        true,  "");
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     false, "");
  ASSERTEQ(IMAGE_IS_PNG(&image),        false, "");
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  false, "");
  image.pixels = &pixel;
  ASSERTEQ(IMAGE_IS_RAW(&image),        true,  "");
  image.png = &raw2;
  ASSERTEQ(IMAGE_IS_RAW(&image),        true,  "");
  image.jp2 = &raw3;
  ASSERTEQ(IMAGE_IS_RAW(&image),        true,  "");

  memset(&image, 0, sizeof(image));

  image.pixels = &pixel;
  ASSERTEQ(IMAGE_IS_RAW(&image),        false, "");
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     true,  "");
  ASSERTEQ(IMAGE_IS_PNG(&image),        false, "");
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  false, "");
  image.data = &raw;
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     true,  "");
  image.png = &raw2;
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     true,  "");
  image.jp2 = &raw3;
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     true,  "");

  memset(&image, 0, sizeof(image));

  image.png = &raw;
  ASSERTEQ(IMAGE_IS_RAW(&image),        false, "");
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     false, "");
  ASSERTEQ(IMAGE_IS_PNG(&image),        true,  "");
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  false, "");
  image.data = &raw2;
  ASSERTEQ(IMAGE_IS_PNG(&image),        true,  "");
  image.pixels = &pixel;
  ASSERTEQ(IMAGE_IS_PNG(&image),        true,  "");
  image.jp2 = &raw3;
  ASSERTEQ(IMAGE_IS_PNG(&image),        true,  "");

  memset(&image, 0, sizeof(image));

  image.jp2 = &raw;
  ASSERTEQ(IMAGE_IS_RAW(&image),        false, "");
  ASSERTEQ(IMAGE_IS_PIXELS(&image),     false, "");
  ASSERTEQ(IMAGE_IS_PNG(&image),        false, "");
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  true,  "");
  image.data = &raw2;
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  true,  "");
  image.pixels = &pixel;
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  true,  "");
  image.png = &raw3;
  ASSERTEQ(IMAGE_IS_JPEG_2000(&image),  true,  "");
}

#define clear_check(img, orig) do { \
  icns_clear_image(&(img)); \
  ASSERTMEM(&(img), &(orig), sizeof(img), ""); \
} while(0)

UNITTEST(image_icns_clear_image)
{
  struct icns_image_set *images;
  struct icns_image image_a;
  struct icns_image image_b;
  struct icns_image image_c;
  struct icns_image image_a_original, image_b_original, image_c_original;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  images = &(icns.images);

  /* Should do nothing for an initialized image. */
  memset(&image_a, 0, sizeof(image_a));
  memset(&image_b, 0, sizeof(image_b));
  memset(&image_c, 0, sizeof(image_c));
  icns_clear_image(&image_a);

  /* Spoof image data so this can be tested independently. */
  images->head = &image_a;
  image_a.next = &image_b;
  image_b.prev = &image_a;
  image_b.next = &image_c;
  image_c.prev = &image_b;
  images->tail = &image_c;
  image_a.format = &format_abcd;
  image_b.format = &format_ABCE;
  image_c.format = &format_Baad;
  image_a.real_width = 1;
  image_a.real_height = 2;
  image_b.real_width = 3;
  image_b.real_height = 4;
  image_c.real_width = 5;
  image_c.real_height = 6;

  image_a_original = image_a;
  image_b_original = image_b;
  image_c_original = image_c;

  /* Clear should not affect metadata and data structure fields. */
  clear_check(image_a, image_a_original);
  clear_check(image_b, image_b_original);
  clear_check(image_c, image_c_original);

  /* Should clear all data fields independently. */
  image_a.pixels = (struct rgba_color *)malloc(sizeof(struct rgba_color));
  ASSERT(image_a.pixels, "");
  clear_check(image_a, image_a_original);

  image_a.data = (uint8_t *)malloc(1);
  ASSERT(image_a.data, "");
  clear_check(image_a, image_a_original);

  image_a.png = (uint8_t *)malloc(1);
  ASSERT(image_a.png, "");
  clear_check(image_a, image_a_original);

  image_a.jp2 = (uint8_t *)malloc(1);
  ASSERT(image_a.jp2, "");
  clear_check(image_a, image_a_original);

  image_a.data_size = 97531;
  clear_check(image_a, image_a_original);

  image_a.png_size = 12345;
  clear_check(image_a, image_a_original);

  image_a.jp2_size = 54321;
  clear_check(image_a, image_a_original);

  /* Should clear all in one call. */
  image_a.pixels = (struct rgba_color *)malloc(sizeof(struct rgba_color));
  image_a.data = (uint8_t *)malloc(1);
  image_a.png = (uint8_t *)malloc(1);
  image_a.jp2 = (uint8_t *)malloc(1);
  image_a.data_size = 398413;
  image_a.png_size = 54064;
  image_a.jp2_size = 7777766;
  ASSERT(image_a.pixels, "");
  ASSERT(image_a.data, "");
  ASSERT(image_a.png, "");
  ASSERT(image_a.jp2, "");
  clear_check(image_a, image_a_original);
}

#define set_format(image, fmt) do { \
  (image).format = fmt; \
  (image).real_width = (fmt)->width * (fmt)->factor; \
  (image).real_height = (fmt)->height * (fmt)->factor; \
} while(0)

static void check_alloc_pixels(const struct icns_format *format)
{
  struct rgba_color *pixels;
  struct rgba_color *pos;
  struct icns_image image;

  size_t width = format->width * format->factor;
  size_t height = format->height * format->factor;
  size_t x;
  size_t y;

  set_format(image, format);
  pixels = icns_allocate_pixel_array_for_image(&image);
  ASSERT(pixels, "%s", format->name);

  /* Should be able to write w * h * f^2 pixels to the allocated buffer for
   * every format. */
  pos = pixels;
  for(y = 0; y < height; y++)
  {
    for(x = 0; x < width; x++)
    {
      pos->r = x * y;
      pos->g = (x + 1) * y;
      pos->b = x * (y + 1);
      pos->a = (x + 1) * (y + 1);
      pos++;
    }
  }
  free(pixels);
}

UNITTEST(image_icns_allocate_pixel_array_for_image)
{
  check_alloc_pixels(&format_abcd);
  check_alloc_pixels(&format_ABCE);
  check_alloc_pixels(&format_Baad);
  check_alloc_pixels(&format_d00d);
}

UNITTEST(image_icns_get_image_by_format)
{
  struct icns_image_set *images;
  struct icns_image image_a;
  struct icns_image image_b;
  struct icns_image image_c;
  struct icns_image *image;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  memset(&image_a, 0, sizeof(image_a));
  memset(&image_b, 0, sizeof(image_b));
  memset(&image_c, 0, sizeof(image_c));

  images = &(icns.images);

  /* Should not be able to get images from init state. */
  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, NULL, "");

  /* Spoof image data so this can be tested independently. */
  images->head = &image_a;
  image_a.next = &image_b;
  image_b.prev = &image_a;
  image_b.next = &image_c;
  image_c.prev = &image_b;
  images->tail = &image_c;

  image_a.format = &format_abcd;
  image_b.format = &format_ABCE;
  image_c.format = &format_Baad;

  /* Should be able to get the three added, not the fourth. */
  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, &image_a, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, &image_b, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, &image_c, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, NULL, "");

  /* Change formats. */
  image_a.format = &format_d00d;
  image_b.format = &format_Baad;
  image_c.format = &format_ABCE;

  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, &image_c, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, &image_b, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, &image_a, "");

  /* "Remove" some--should no longer be able to get them. */
  image_a.format = NULL;
  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, &image_c, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, &image_b, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, NULL, "");

  image_b.format = NULL;
  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, &image_c, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, NULL, "");

  image_c.format = NULL;
  image = icns_get_image_by_format(&icns, &format_abcd);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_ABCE);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_Baad);
  ASSERTEQ(image, NULL, "");
  image = icns_get_image_by_format(&icns, &format_d00d);
  ASSERTEQ(image, NULL, "");
}

UNITTEST(image_icns_add_image_for_format)
{
  enum icns_error ret;
  struct icns_image *image = NULL;
  struct icns_image *image2 = NULL;
  struct icns_image *image3 = NULL;
  struct icns_image *image4 = NULL;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Add for format--ok. */
  ret = icns_add_image_for_format(&icns, &image, NULL, &format_abcd);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");

  /* Can't add duplicate of any format--this just returns the existing one. */
  ret = icns_add_image_for_format(&icns, &image2, NULL, &format_abcd);
  check_ok_var(&icns, ret, ICNS_IMAGE_EXISTS_FOR_FORMAT);
  ASSERTEQ(image, image2, "");

  /* Add another format to the end. */
  ret = icns_add_image_for_format(&icns, &image2, NULL, &format_d00d);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, image2, "");
  ASSERTEQ(image2->prev, image, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");

  /* Add a format between the two. */
  ret = icns_add_image_for_format(&icns, &image3, image, &format_ABCE);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, image3, "");
  ASSERTEQ(image3->prev, image, "");
  ASSERTEQ(image3->next, image2, "");
  ASSERTEQ(image2->prev, image3, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), image3, "");

  /* Still can't add duplicates of any of them... */
  ret = icns_add_image_for_format(&icns, &image4, image, &format_abcd);
  check_ok_var(&icns, ret, ICNS_IMAGE_EXISTS_FOR_FORMAT);
  ASSERTEQ(image, image4, "");
  ret = icns_add_image_for_format(&icns, &image4, image, &format_d00d);
  check_ok_var(&icns, ret, ICNS_IMAGE_EXISTS_FOR_FORMAT);
  ASSERTEQ(image2, image4, "");
  ret = icns_add_image_for_format(&icns, &image4, image, &format_ABCE);
  check_ok_var(&icns, ret, ICNS_IMAGE_EXISTS_FOR_FORMAT);
  ASSERTEQ(image3, image4, "");

  /* This one should work, though. */
  ret = icns_add_image_for_format(&icns, &image4, image, &format_Baad);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, image4, "");
  ASSERTEQ(image4->prev, image, "");
  ASSERTEQ(image4->next, image3, "");
  ASSERTEQ(image3->prev, image4, "");
  ASSERTEQ(image3->next, image2, "");
  ASSERTEQ(image2->prev, image3, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), image3, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), image4, "");

  icns_clear_state_data(&icns);
}

UNITTEST(image_icns_delete_image_by_format)
{
  enum icns_error ret;
  struct icns_image *image = NULL;
  struct icns_image *image2 = NULL;
  struct icns_image *image3 = NULL;
  struct icns_image *image4 = NULL;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Add images. */
  ret = icns_add_image_for_format(&icns, &image, NULL, &format_abcd);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &image2, NULL, &format_d00d);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &image3, image, &format_ABCE);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, &image4, image, &format_Baad);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, image4, "");
  ASSERTEQ(image4->prev, image, "");
  ASSERTEQ(image4->next, image3, "");
  ASSERTEQ(image3->prev, image4, "");
  ASSERTEQ(image3->next, image2, "");
  ASSERTEQ(image2->prev, image3, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), image3, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), image4, "");

  /* Deleting them should only remove the relevant image,
   * not damage the list structure. */

  /* Middle */
  ret = icns_delete_image_by_format(&icns, &format_ABCE);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image->prev, NULL, "");
  ASSERTEQ(image->next, image4, "");
  ASSERTEQ(image4->prev, image, "");
  ASSERTEQ(image4->next, image2, "");
  ASSERTEQ(image2->prev, image4, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), image, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), image4, "");

  /* Deleting an image that doesn't exist does nothing. */
  ret = icns_delete_image_by_format(&icns, &format_ABCE);
  check_ok_var(&icns, ret, ICNS_NO_IMAGE);

  /* First */
  ret = icns_delete_image_by_format(&icns, &format_abcd);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image4, "");
  ASSERTEQ(icns.images.tail, image2, "");
  ASSERTEQ(image4->prev, NULL, "");
  ASSERTEQ(image4->next, image2, "");
  ASSERTEQ(image2->prev, image4, "");
  ASSERTEQ(image2->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), image2, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), image4, "");

  /* Last */
  ret = icns_delete_image_by_format(&icns, &format_d00d);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, image4, "");
  ASSERTEQ(icns.images.tail, image4, "");
  ASSERTEQ(image4->prev, NULL, "");
  ASSERTEQ(image4->next, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), image4, "");

  /* Only */
  ret = icns_delete_image_by_format(&icns, &format_Baad);
  check_ok(&icns, ret);
  ASSERTEQ(icns.images.head, NULL, "");
  ASSERTEQ(icns.images.tail, NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_abcd), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_d00d), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_ABCE), NULL, "");
  ASSERTEQ(icns_get_image_by_format(&icns, &format_Baad), NULL, "");

  icns_clear_state_data(&icns);
}

UNITTEST(image_icns_delete_all_images)
{
  /* This is already effectively verified by the prior tests... */
  enum icns_error ret;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Should be able to use immediately after init without issues. */
  icns_delete_all_images(&icns);

  /* Add one and then delete. */
  ret = icns_add_image_for_format(&icns, NULL, NULL, &format_ABCE);
  check_ok(&icns, ret);
  icns_delete_all_images(&icns);

  /* Add several and then delete. */
  ret = icns_add_image_for_format(&icns, NULL, NULL, &format_abcd);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, NULL, NULL, &format_ABCE);
  check_ok(&icns, ret);
  ret = icns_add_image_for_format(&icns, NULL, NULL, &format_Baad);
  check_ok(&icns, ret);
  icns_delete_all_images(&icns);

  /* Double delete. */
  icns_delete_all_images(&icns);
  icns_clear_state_data(&icns);
}
