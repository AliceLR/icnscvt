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

#ifndef ICNSCVT_TARGA_H
#define ICNSCVT_TARGA_H

#include "test.h"

ICNS_BEGIN_DECLS

struct rgba_color;

struct loaded_file
{
  char *which;
  uint8_t *data;
  size_t data_size;
  struct rgba_color *pixels;
  unsigned w;
  unsigned h;
};

extern const uint8_t test_random_data[256];

void test_load(struct icns_data *icns, uint8_t **dest, size_t *size,
 const char *filename) NOT_NULL;

void test_load_compressed(struct icns_data *icns,
 uint8_t **dest, size_t *size, const char *filename) NOT_NULL;

void test_save(struct icns_data *icns,
 const uint8_t *src, size_t size, const char *filename) NOT_NULL;

void test_save_compressed(struct icns_data *icns,
 const uint8_t *uncompressed, size_t uncompressed_size,
 const char *filename) NOT_NULL;

void test_save_tga(struct icns_data *icns,
 const struct rgba_color *pixels, unsigned w, unsigned h,
 const char *filename) NOT_NULL;

void test_load_tga(struct icns_data *icns, struct rgba_color **dest,
 unsigned *dest_w, unsigned *dest_h, const char *filename) NOT_NULL;

const struct loaded_file *test_load_cached(struct icns_data *icns,
 const char *filename) NOT_NULL;
const struct loaded_file *test_load_tga_cached(struct icns_data *icns,
 size_t width, size_t height, const char *filename) NOT_NULL;
void test_load_cached_cleanup(void);

ICNS_END_DECLS

#endif /* ICNSCVT_TARGA_H */
