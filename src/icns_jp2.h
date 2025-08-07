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

#ifndef ICNSCVT_JP2_H
#define ICNSCVT_JP2_H

#include "common.h"

ICNS_BEGIN_DECLS

struct icns_jp2_stat
{
  unsigned width;           /* width, pixels */
  unsigned height;          /* height, pixels */
  unsigned depth;           /* sum bit depth of all components */
};

bool icns_is_file_jp2(const void *data, size_t data_size);

enum icns_error icns_get_jp2_info(
 struct icns_data * RESTRICT icns, struct icns_jp2_stat * RESTRICT dest,
 const uint8_t *jp2_data, size_t jp2_size) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_JP2_H */
