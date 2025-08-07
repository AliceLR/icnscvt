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

#include "icns_io.h"
#include "icns_jp2.h"

#include <string.h>

/* Raw codestream */
static const uint8_t magic_j2k[4] =
{
  0xff, 0x4f, 0xff, 0x51
};

/* JPEG 2000 part 1 container */
static const uint8_t magic_jp2[12] =
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

static enum icns_error icns_scan_jp2_codestream(
 struct icns_data * RESTRICT icns, struct icns_jp2_stat * RESTRICT dest,
 const uint8_t *jp2_data, size_t jp2_size)
{
  unsigned depth = 0;
  size_t num_components;
  size_t i;

  /* File should start with start of stream marker 0xff4f,
   * SIZ marker 0xff51. SIZ segment must be 38 + components * 3 bytes long. */
  if(jp2_size < 4 + 38 ||
   memcmp(jp2_data, magic_j2k, sizeof(magic_j2k)) ||
   icns_get_u16be(jp2_data + 4) < 38)
  {
    E_("not a jp2 codestream or truncated");
    return ICNS_JP2_DATA_ERROR;
  }

  num_components = icns_get_u16be(jp2_data + 4 + 36);
  if(jp2_size - 4 - 38 < num_components * 3)
  {
    E_("jp2 codestream truncated, can't read components");
    return ICNS_JP2_DATA_ERROR;
  }

  for(i = 0; i < num_components; i++)
    depth += jp2_data[4 + 38 + i * 3] + 1;

  dest->width = icns_get_u32be(jp2_data + 8);
  dest->height = icns_get_u32be(jp2_data + 12);
  dest->depth = depth;
  return ICNS_OK;
}

static bool icns_get_jp2_box(uint32_t *length, uint32_t *magic,
 const uint8_t *jp2_data, size_t jp2_size)
{
  if(jp2_size < 8)
    return false;

  *length = icns_get_u32be(jp2_data + 0);
  *magic = icns_get_u32be(jp2_data + 4);
  return true;
}

static enum icns_error icns_scan_jp2_part_1(
 struct icns_data * RESTRICT icns, struct icns_jp2_stat * RESTRICT dest,
 const uint8_t *jp2_data, size_t jp2_size)
{
  uint32_t length;
  uint32_t magic;

  /* Looking for the box "jp2c". Skip the initial identification box. */
  jp2_data += sizeof(magic_jp2);
  jp2_size -= sizeof(magic_jp2);

  while(true)
  {
    if(!icns_get_jp2_box(&length, &magic, jp2_data, jp2_size) || length > jp2_size)
    {
      E_("truncated jp2 container");
      return ICNS_JP2_DATA_ERROR;
    }

    if(magic != MAGIC('j','p','2','c'))
    {
      jp2_data += length;
      jp2_size -= length;
      continue;
    }

    /* Reposition to codestream data. */
    jp2_data += 8;
    jp2_size -= 8;
    return icns_scan_jp2_codestream(icns, dest, jp2_data, jp2_size);
  }
}

/**
 * Get the dimensions and bit depth of a JP2 in a buffer without decoding
 * the entire image. This is intended for autodetecting the format for an
 * arbitrary input JP2.
 *
 * @param icns      current state data.
 * @param dest      the JP2 header data will be stored to this pointer on
 *                  success. On failure, this struct will be unchanged.
 * @param jp2_data  pointer to JP2/J2K data in memory.
 * @param jp2_size  size of JP2/J2K data in memory.
 * @return          `ICNS_OK` on success;
 *                  `ICNS_JP2_NOT_A_JP2` if this buffer isn't a JP2/J2K;
 *                  `ICNS_JP2_DATA_ERROR` if parsing the JP2/J2K header failed.
 */
enum icns_error icns_get_jp2_info(
 struct icns_data * RESTRICT icns, struct icns_jp2_stat * RESTRICT dest,
 const uint8_t *jp2_data, size_t jp2_size)
{
  if(!icns_is_file_jp2(jp2_data, jp2_size))
  {
    E_("not a JP2 codestream or JPEG 2000");
    return ICNS_JP2_NOT_A_JP2;
  }

  if(!memcmp(jp2_data, magic_j2k, sizeof(magic_j2k)))
    return icns_scan_jp2_codestream(icns, dest, jp2_data, jp2_size);

  if(!memcmp(jp2_data, magic_jp2, sizeof(magic_jp2)))
    return icns_scan_jp2_part_1(icns, dest, jp2_data, jp2_size);

  /* ? */
  E_("internal error scanning JP2");
  return ICNS_INTERNAL_ERROR;
}
