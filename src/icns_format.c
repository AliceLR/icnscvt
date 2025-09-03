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

#include "common.h"
#include "icns_io.h"
#include "icns_format.h"
/*
#include "icns_format_1bit.h"
*/
/*
#include "icns_format_4bit.h"
*/
/*
#include "icns_format_8bit.h"
*/
#include "icns_format_argb.h"
#include "icns_format_mask.h"
#include "icns_format_png.h"

static const struct icns_format * const icns_format_list[] =
{
  /* 1-bit */
  /* FIXME:
  &icns_format_ICON,
  &icns_format_icmp,
  &icns_format_icsp,
  &icns_format_ICNp,
  &icns_format_ichp,
  */

  /* 4-bit */
  /* FIXME:
  &icns_format_icm4,
  &icns_format_ics4,
  &icns_format_icl4,
  &icns_format_ich4,
  */

  /* 8-bit */
  /* FIXME:
  &icns_format_icm8,
  &icns_format_ics8,
  &icns_format_icl8,
  &icns_format_ich8,
  */

  /* RGB */
  &icns_format_is32,
  &icns_format_il32,
  &icns_format_ih32,
  &icns_format_it32,

  /* 8-bit mask */
  &icns_format_s8mk,
  &icns_format_l8mk,
  &icns_format_h8mk,
  &icns_format_t8mk,

  /* PNG/JP2 */
  &icns_format_icp4, /* + RGB; import as PNG despite bugs to preserve alpha */
  &icns_format_icp5, /* + RGB; import as PNG despite bugs to preserve alpha */
  &icns_format_icp6,
  &icns_format_ic04, /* + ARGB; import as ARGB to avoid potential PNG issues */
  &icns_format_ic05, /* + ARGB; import as ARGB to avoid potential PNG issues */
  &icns_format_ic07,
  &icns_format_ic08,
  &icns_format_ic09,
  &icns_format_ic10,
  &icns_format_ic11,
  &icns_format_ic12,
  &icns_format_ic13,
  &icns_format_ic14,
  &icns_format_icsb, /* + ARGB; import as ARGB to avoid potential PNG issues */
  &icns_format_icsB,
  &icns_format_sb24,
  &icns_format_SB24
};

static const size_t num_formats =
 sizeof(icns_format_list) / sizeof(icns_format_list[0]);


static const char *icns_format_type_string[] =
{
  "",
  "1-bit",
  "1-bit with mask",
  "4-bit",
  "8-bit",
  "24-bit RGB",
  "8-bit mask",
  "24-bit RGB or PNG or JPEG 2000",
  "PNG or JPEG 2000",
  "ARGB or PNG or JPEG 2000"
};

/**
 * Get a human-readable string description of an ICNS image format.
 * This includes a human-readable format magic, the nominal dimensions and
 * scale, the real dimensions in pixels, a brief image type description,
 * and the oldest System/Mac OS/Mac OS X/macOS version to support the format.
 *
 * This function is a simple wrapper around `snprintf`. Error values are
 * coerced to 0.
 *
 * @param dest      destination buffer to write the description to.
 *                  May be `NULL` to get a string length for allocation. If
 *                  the size of this buffer (as indicated by `dest_len`) is
 *                  less than the value returned by this function, plus 1 for
 *                  the nul terminator, the string written to this buffer will
 *                  be truncated.
 * @param dest_len  size of the destination buffer in bytes. If `dest` is
 *                  `NULL`, this parameter is ignored.
 * @param format    ICNS image format to get description for.
 * @return          the length of the description string written to `dest` (not
 *                  including the nul terminator, or the length of the string
 *                  string that would be written (if `dest` is `NULL`).
 */
size_t icns_get_format_string(void *dest, size_t dest_len,
 const struct icns_format *format)
{
  int ret;

  /* More permissive handling of length when pointer is NULL than the C spec. */
  if(!dest)
    dest_len = 0;

  if(format->factor > 1)
  {
    ret = snprintf(dest, dest_len,
      "  %s: %dx%d@%d (%dx%d) %s (OS %d.%d)\n",
      format->name, format->width, format->height, format->factor,
      format->width * format->factor, format->height * format->factor,
      icns_format_type_string[format->type],
      format->macos_ver >> 8, (format->macos_ver & 0xff) >> 4
    );
  }
  else
  {
    ret = snprintf(dest, dest_len,
      "  %s: %dx%d %s (OS %d.%d)\n",
      format->name, format->width, format->height,
      icns_format_type_string[format->type],
      format->macos_ver >> 8, (format->macos_ver & 0xff) >> 4
    );
  }
  return ret > 0 ? ret : 0;
}

/**
 * Get the full list of supported ICNS image formats.
 *
 * @param list      pointer to an array of `const struct icns_format *` to be
 *                  populated. If NULL, no entries will be written.
 * @param size      size of the array pointed to by `list`.
 * @return          the total number of supported formats. If this is greater
 *                  than the provided `size`, only the first `size` formats
 *                  will be stored to `list`.
 */
size_t icns_get_format_list(const struct icns_format **list, size_t size)
{
  if(list)
  {
    size_t i;
    if(size > num_formats)
      size = num_formats;

    for(i = 0; i < size; i++)
      list[i] = icns_format_list[i];
  }
  return num_formats;
}

/**
 * Get an ICNS image format by its magic string in uint32_t form (big endian).
 *
 * @param magic     magic string for the format (big endian uint32_t).
 * @return          the format specification, if it exists, otherwise NULL.
 */
const struct icns_format *icns_get_format_by_magic(uint32_t magic)
{
  size_t i;
  for(i = 0; i < num_formats; i++)
    if(magic == icns_format_list[i]->magic)
      return icns_format_list[i];

  return NULL;
}

/**
 * Get an ICNS image format by its magic string in nul-terminated string form.
 *
 * @param magic     magic string for the format (nul-terminated string).
 * @return          the format specification, if it exists, otherwise NULL.
 */
const struct icns_format *icns_get_format_by_name(const char *name)
{
  size_t i;
  for(i = 0; i < num_formats; i++)
  {
    const struct icns_format *format = icns_format_list[i];

    if(format->name && !strcmp(format->name, name))
      return format;
  }
  return NULL;
}

/**
 * Get the best matching ICNS image format by its dimensions and depth.
 * This is not very accurate; it always assumes 1-bit desires a 1-bit mask,
 * and it assumes 16x16, 32x32, 48x48, and 128x128 images always want the
 * most portable is32/il32/ih32/it32 formats.
 *
 * @param width     width of the image to get a format for.
 * @param height    height of the image to get a format for.
 * @param depth     depth of the image. This assumes that a depth of 8 bits
 *                  means that the 8-bit formats are most desired, which is
 *                  not necessarily true.
 * @param factor    0: use real pixel resolution to match (ignore Retina).
 *                  1: only match old and non-Retina formats (logical pixels).
 *                  2: only match Retina formats (logical pixels).
 *                     e.g. width=16, height=16, depth=2 matches ic11.
 * @return          the format specification, if one exists, otherwise NULL.
 */
const struct icns_format *icns_get_format_by_attributes(
 unsigned width, unsigned height, unsigned depth, unsigned factor)
{
  size_t i;
  for(i = 0; i < num_formats; i++)
  {
    const struct icns_format *format = icns_format_list[i];
    size_t compare_width = format->width;
    size_t compare_height = format->height;

    if(factor == 0)
    {
      compare_width *= format->factor;
      compare_height *= format->factor;
    }

    if(width != compare_width || height != compare_height)
      continue;

    if(factor && factor != (unsigned)format->factor)
      continue;

    switch(format->type)
    {
      /* These formats are currently not matched here. */
      case ICNS_UNKNOWN:
      case ICNS_1_BIT:
      case ICNS_8_BIT_MASK:
        continue;

      case ICNS_1_BIT_WITH_MASK:
        if(depth != 1)
          continue;
        break;

      case ICNS_4_BIT:
        if(depth != 4)
          continue;
        break;

      case ICNS_8_BIT:
        if(depth != 8)
          continue;
        break;

      case ICNS_24_BIT:
      case ICNS_24_BIT_OR_PNG:
      case ICNS_ARGB_OR_PNG:
      case ICNS_PNG:
        if(depth < 15)
          continue;
        break;
    }
    return format;
  }
  return NULL;
}

/**
 * Get an ICNS mask format for a given image format.
 *
 * @param format    ICNS image format to get the corresponding mask format for.
 * @return          the mask specification, if it exists, otherwise NULL.
 */
const struct icns_format *icns_get_mask_for_format(
 const struct icns_format *format)
{
  if(format->type == ICNS_24_BIT)
  {
    if(format->width == 16)
      return &icns_format_s8mk;
    if(format->width == 32)
      return &icns_format_l8mk;
    if(format->width == 48)
      return &icns_format_h8mk;
    if(format->width == 128)
      return &icns_format_t8mk;
  }
  return NULL;
}

/**
 * Get an ICNS image format for a given mask.
 */
const struct icns_format *icns_get_format_from_mask(
 const struct icns_format *mask)
{
  if(mask->type == ICNS_8_BIT_MASK)
  {
    if(mask->width == 16)
      return &icns_format_is32;
    if(mask->width == 32)
      return &icns_format_il32;
    if(mask->width == 48)
      return &icns_format_ih32;
    if(mask->width == 128)
      return &icns_format_it32;
  }
  return NULL;
}

/**
 * Check if an ICNS image format is a mask format.
 *
 * @param format    ICNS image format to check.
 * @return          true if the image format is a mask format, otherwise false.
 */
bool icns_format_is_mask(const struct icns_format *format)
{
  return format->type == ICNS_8_BIT_MASK;
}

/**
 * Check if an ICNS image format supports storing PNG.
 *
 * @param format    ICNS image format to check.
 * @return          true if the format supports PNG, otherwise false.
 */
bool icns_format_supports_png(const struct icns_format *format)
{
  return format->type == ICNS_PNG ||
         format->type == ICNS_24_BIT_OR_PNG ||
         format->type == ICNS_ARGB_OR_PNG;
}

/**
 * Check if an ICNS image format supports storing JPEG 2000.
 *
 * @param format    ICNS image format to check.
 * @return          true if the format supports JPEG 2000, otherwise false.
 */
bool icns_format_supports_jpeg_2000(const struct icns_format *format)
{
  return format->type == ICNS_PNG ||
         format->type == ICNS_24_BIT_OR_PNG ||
         format->type == ICNS_ARGB_OR_PNG;
}
