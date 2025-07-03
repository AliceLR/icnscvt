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
#include "../src/icns_jp2.h"

static const uint8_t jp2_a[] =
{
  0xff, 0x4f, 0xff, 0x51, 0, 120, 49, 'a', 29, 255
};

static const uint8_t jp2_b[] =
{
  0x00, 0x00, 0x00, 0x0c, 0x6a, 0x50,
  0x20, 0x20, 0x0d, 0x0a, 0x87, 0x0a,
  0xff, 0x51, 0, 120, 49, 'a', 29, 255
};

static const uint8_t not_a[] =
{
  'a'
};

static const uint8_t not_b[] =
{
  0x89, 'P', 'N', 'G', 0x0d, 0x0a, 0x1a, 0x0a
};

UNITTEST(format_jp2_icns_is_file_jp2)
{
  ASSERTEQ(icns_is_file_jp2(jp2_a, sizeof(jp2_a)), true);
  ASSERTEQ(icns_is_file_jp2(jp2_b, sizeof(jp2_b)), true);

  ASSERTEQ(icns_is_file_jp2(jp2_a, 3), false);
  ASSERTEQ(icns_is_file_jp2(jp2_b, 11), false);
  ASSERTEQ(icns_is_file_jp2(not_a, sizeof(not_a)), false);
  ASSERTEQ(icns_is_file_jp2(not_b, sizeof(not_b)), false);
  ASSERTEQ(icns_is_file_jp2("", 0), false);
}
