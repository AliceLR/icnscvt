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
#include "../src/icns_io.h"

/***** Misc. IO functions that don't fall in the other categories. *****/

UNITTEST(io_icns_put_u32be)
{
  static const uint8_t expected[8] =
  {
    0x12, 0x34, 0x56, 0x78,
    0xfe, 0xda, 0x09, 0xad
  };
  uint8_t buf[8];

  icns_put_u32be(buf + 0, 0x12345678);
  icns_put_u32be(buf + 4, 0xfeda09ad);
  ASSERTMEM(buf, expected, sizeof(expected), "");
}

UNITTEST(io_icns_get_u32be)
{
  static const uint8_t data_a[4] = { 0x12, 0x34, 0x56, 0x78 };
  static const uint8_t data_b[4] = { 0xfe, 0xda, 0x09, 0xad };

  ASSERTEQ(icns_get_u32be(data_a), 0x12345678, "");
  ASSERTEQ(icns_get_u32be(data_b), 0xfeda09ad, "");
}
