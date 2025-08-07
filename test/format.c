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
#include "../src/icns_format_png.h"
#include "../src/icns_image.h"
#include "../src/icns_io.h"
#include "../src/icns_png.h"

#include "format_read_from_icns.h"
#include "format_prepare_for_icns.h"
#include "format_write_to_icns.h"
#include "format_read_from_external.h"
#include "format_prepare_for_external.h"
#include "format_write_to_external.h"

static const struct test_format test_formats[] =
{
  { &icns_format_icp6, true,  true,   PNG_DIR "/48x48",     NULL },
  { &icns_format_ic07, true,  true,   PNG_DIR "/128x128",   NULL },
  { &icns_format_ic08, true,  true,   PNG_DIR "/256x256",   NULL },
  { &icns_format_ic09, true,  true,   PNG_DIR "/512x512",   NULL },
  { &icns_format_ic10, true,  true,   PNG_DIR "/1024x1024", NULL },
  { &icns_format_ic11, true,  true,   PNG_DIR "/32x32",     NULL },
  { &icns_format_ic12, true,  true,   PNG_DIR "/64x64",     NULL },
  { &icns_format_ic13, true,  true,   PNG_DIR "/256x256",   NULL },
  { &icns_format_ic14, true,  true,   PNG_DIR "/512x512",   NULL },
  { &icns_format_icsB, true,  true,   PNG_DIR "/36x36",     NULL },
  { &icns_format_sb24, true,  true,   PNG_DIR "/24x24",     NULL },
  { &icns_format_SB24, true,  true,   PNG_DIR "/48x48",     NULL },
};
static const size_t num_formats = sizeof(test_formats) / sizeof(test_formats[0]);


void test_format_functions(const struct icns_format *format)
{
  const struct icns_format *list[64];
  const struct test_format *which = NULL;
  size_t num;
  size_t i;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  num = icns_get_format_list(list, 64);
  ASSERT(num < 64, "format list buffer too small");

  for(i = 0; i < num_formats; i++)
  {
    /* Verify all formats are represented in the test data. */
    ASSERTEQ(list[i], test_formats[i].format,
      "%s != %s", list[i]->name, test_formats[i].format->name);

    if(test_formats[i].format == format)
      which = &test_formats[i];
  }
  ASSERT(which, "%s: not in provided list", format->name);

  ASSERTEQ(icns_get_format_by_magic(format->magic), format, "%s: not in list", format->name);
  ASSERT(format->prepare_for_icns, "%s", format->name);
  ASSERT(format->read_from_icns, "%s", format->name);
  ASSERT(format->write_to_icns, "%s", format->name);
  ASSERT(format->read_from_external, "%s", format->name);
  ASSERT(format->write_to_external, "%s", format->name);

  test_format_read_from_icns(&icns, test_formats, num_formats, which, i);
  test_format_write_to_icns(&icns, which);
  test_format_prepare_for_icns(&icns, which);
  test_format_read_from_external(&icns, test_formats, num_formats, which, i);
  test_format_write_to_external(&icns, which);
  test_format_prepare_for_external(&icns, which);
}
