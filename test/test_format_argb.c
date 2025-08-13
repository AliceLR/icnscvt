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
#include "format.h"
#include "../src/icns_format_argb.h"

UNITTEST(format_icns_format_is32)
{
  test_format_maybe_generate_raw(&icns_format_is32);
  test_format_functions(&icns_format_is32);
}

UNITTEST(format_icns_format_il32)
{
  test_format_maybe_generate_raw(&icns_format_il32);
  test_format_functions(&icns_format_il32);
}

UNITTEST(format_icns_format_ih32)
{
  test_format_maybe_generate_raw(&icns_format_ih32);
  test_format_functions(&icns_format_ih32);
}

UNITTEST(format_icns_format_it32)
{
  test_format_maybe_generate_raw(&icns_format_it32);
  test_format_functions(&icns_format_it32);
}

UNITTEST(format_icns_format_icp4)
{
  test_format_maybe_generate_raw(&icns_format_icp4);
  test_format_functions(&icns_format_icp4);
}

UNITTEST(format_icns_format_icp5)
{
  test_format_maybe_generate_raw(&icns_format_icp5);
  test_format_functions(&icns_format_icp5);
}

UNITTEST(format_icns_format_ic04)
{
  test_format_maybe_generate_raw(&icns_format_ic04);
  test_format_functions(&icns_format_ic04);
}

UNITTEST(format_icns_format_ic05)
{
  test_format_maybe_generate_raw(&icns_format_ic05);
  test_format_functions(&icns_format_ic05);
}

UNITTEST(format_icns_format_icsb)
{
  test_format_maybe_generate_raw(&icns_format_icsb);
  test_format_functions(&icns_format_icsb);
}
