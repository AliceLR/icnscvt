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

#include "icns_jp2.h"

#include <string.h>

static const uint8_t magic_jp2[4] =
{
  0xff, 0x4f, 0xff, 0x51
};

static const uint8_t magic_j2k[12] =
{
  0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50,
  0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a
};

/**
 * Test if a memory buffer contains the start of a JPEG 2000 file.
 *
 * @param data        buffer to test.
 * @param data_size   size of buffer to test.
 * @return            true if buffer is a JPEG 2000 file, otherwise false.
 */
bool icns_is_file_jp2(const void *data, size_t data_size)
{
  if(data_size >= sizeof(magic_jp2))
    if(!memcmp(data, magic_jp2, sizeof(magic_jp2)))
      return true;

  if(data_size >= sizeof(magic_j2k))
    if(!memcmp(data, magic_j2k, sizeof(magic_j2k)))
      return true;

  return false;
}
