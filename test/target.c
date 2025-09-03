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

#include "target.h"
#include "../src/icns_format_argb.h"
#include "../src/icns_format_mask.h"
#include "../src/icns_format_png.h"

const struct target_test external_load_sequence[] =
{
  { PNG_DIR "/16x16.png",         icns_magic_is32, icns_magic_is32, 0, 0 },
  { PNG_DIR "/16x16_rgba16.png",  icns_magic_is32, icns_magic_is32, 0, 0 },
  { PNG_DIR "/16x16.png",         icns_magic_ic04, icns_magic_ic04, 0, 0 },
  { PNG_DIR "/16x16_m.png",       icns_magic_s8mk, icns_magic_s8mk, 0, 0 },
  { PNG_DIR "/16x16_m.png",       icns_magic_is32, icns_magic_is32, 0, 0 },
  { PNG_DIR "/32x32.j2k",         icns_magic_ic05, icns_magic_ic05, 0, 0 },

  { PNG_DIR "/16x16.png",         0x12345678, 0ul,
    ICNS_UNKNOWN_FORMAT, -ICNS_UNKNOWN_FORMAT },
  { PNG_DIR "/16x16sdfasdf.png",  icns_magic_ic14, 0ul,
    ICNS_READ_OPEN_ERROR, -ICNS_READ_OPEN_ERROR },
  { PNG_DIR "/32x32.j2k",         icns_magic_il32, icns_magic_il32,
    ICNS_DATA_ERROR, -ICNS_DATA_ERROR, },
  { PNG_DIR "/16x16.png",         icns_magic_ih32, icns_magic_ih32,
    ICNS_INVALID_DIMENSIONS, -ICNS_INVALID_DIMENSIONS },
};
const size_t num_external_load_sequence =
 sizeof(external_load_sequence) / sizeof(external_load_sequence[0]);


const struct target_test external_save_sequence[] =
{
  { NULL,                         icns_magic_ic04, icns_magic_ic04, 0, 0 },
  { NULL,                         icns_magic_s8mk, icns_magic_s8mk, 0, 0 },
  { NULL,                         icns_magic_is32, icns_magic_is32, 0, 0 },
  { NULL,                         icns_magic_ic05, icns_magic_ic05, 0, 0 },

  { NULL,                         icns_magic_ic14, 0ul,
    ICNS_NO_IMAGE, -ICNS_NO_IMAGE },
  { NULL,                         0x12345678, 0ul,
    ICNS_UNKNOWN_FORMAT, -ICNS_UNKNOWN_FORMAT },
};
const size_t num_external_save_sequence =
 sizeof(external_save_sequence) / sizeof(external_save_sequence[0]);


struct icns_image *get_image_by_magic(struct icns_data *icns, uint32_t magic)
{
  struct icns_image *pos;
  for(pos = icns->images.head; pos; pos = pos->next)
  {
    ASSERT(pos->format, "");
    if(pos->format->magic == magic)
      return pos;
  }
  return NULL;
}
