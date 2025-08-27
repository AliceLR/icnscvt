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

/* Include for magic numbers only--can't use the format specs directly. */
#include "../src/icns_format.h"
/*
#include "../src/icns_format_1bit.h"
*/
/*
#include "../src/icns_format_4bit.h"
*/
/*
#include "../src/icns_format_8bit.h"
*/
#include "../src/icns_format_argb.h"
#include "../src/icns_format_mask.h"
#include "../src/icns_format_png.h"

struct test_format_api
{
  uint32_t magic;
  const char *name;
  unsigned width;
  unsigned height;
  unsigned factor;
  enum icns_format_type type;
};

static const struct test_format_api format_list[] =
{
  /*
  { icns_magic_ICON,  "ICON",   32,   32, 1,  ICNS_1_BIT },
  { icns_magic_icmp,  "icmp",   16,   12, 1,  ICNS_1_BIT_WITH_MASK },
  { icns_magic_icsp,  "icsp",   16,   16, 1,  ICNS_1_BIT_WITH_MASK },
  { icns_magic_ICNp,  "ICNp",   32,   32, 1,  ICNS_1_BIT_WITH_MASK },
  { icns_magic_ichp,  "ichp",   48,   48, 1,  ICNS_1_BIT_WITH_MASK },
  */
  /*
  { icns_magic_icm4,  "icm4",   16,   12, 1,  ICNS_4_BIT },
  { icns_magic_ics4,  "ics4",   16,   16, 1,  ICNS_4_BIT },
  { icns_magic_icl4,  "icl4",`  32,   32, 1,  ICNS_4_BIT },
  { icns_magic_ich4,  "ich4",   48,   48, 1,  ICNS_4_BIT },
  */
  /*
  { icns_magic_icm8,  "icm8",   16,   12, 1,  ICNS_8_BIT },
  { icns_magic_ics8,  "ics8",   16,   16, 1,  ICNS_8_BIT },
  { icns_magic_icl8,  "icl8",   32,   32, 1,  ICNS_8_BIT },
  { icns_magic_ich8,  "ich8",   48,   48, 1,  ICNS_8_BIT },
  */
  { icns_magic_is32,  "is32",   16,   16, 1,  ICNS_24_BIT },
  { icns_magic_il32,  "il32",   32,   32, 1,  ICNS_24_BIT },
  { icns_magic_ih32,  "ih32",   48,   48, 1,  ICNS_24_BIT },
  { icns_magic_it32,  "it32",  128,  128, 1,  ICNS_24_BIT },
  { icns_magic_s8mk,  "s8mk",   16,   16, 1,  ICNS_8_BIT_MASK },
  { icns_magic_l8mk,  "l8mk",   32,   32, 1,  ICNS_8_BIT_MASK },
  { icns_magic_h8mk,  "h8mk",   48,   48, 1,  ICNS_8_BIT_MASK },
  { icns_magic_t8mk,  "t8mk",  128,  128, 1,  ICNS_8_BIT_MASK },
  { icns_magic_icp4,  "icp4",   16,   16, 1,  ICNS_24_BIT_OR_PNG },
  { icns_magic_icp5,  "icp5",   32,   32, 1,  ICNS_24_BIT_OR_PNG },
  { icns_magic_icp6,  "icp6",   48,   48, 1,  ICNS_PNG },
  { icns_magic_ic04,  "ic04",   16,   16, 1,  ICNS_ARGB_OR_PNG },
  { icns_magic_ic05,  "ic05",   32,   32, 1,  ICNS_ARGB_OR_PNG },
  { icns_magic_ic07,  "ic07",  128,  128, 1,  ICNS_PNG },
  { icns_magic_ic08,  "ic08",  256,  256, 1,  ICNS_PNG },
  { icns_magic_ic09,  "ic09",  512,  512, 1,  ICNS_PNG },
  { icns_magic_ic10,  "ic10",  512,  512, 2,  ICNS_PNG },
  { icns_magic_ic11,  "ic11",   16,   16, 2,  ICNS_PNG },
  { icns_magic_ic12,  "ic12",   32,   32, 2,  ICNS_PNG },
  { icns_magic_ic13,  "ic13",  128,  128, 2,  ICNS_PNG },
  { icns_magic_ic14,  "ic14",  256,  256, 2,  ICNS_PNG },
  { icns_magic_icsb,  "icsb",   18,   18, 1,  ICNS_PNG },
  { icns_magic_icsB,  "icsB",   18,   18, 2,  ICNS_PNG },
  { icns_magic_sb24,  "sb24",   24,   24, 1,  ICNS_PNG },
  { icns_magic_SB24,  "SB24",   24,   24, 2,  ICNS_PNG },
};
static const size_t num_formats = sizeof(format_list) / sizeof(format_list[0]);


static const char *not_formats[] =
{
  NULL,
  "",
  "ICONs",
  "s8mk;",
  "ic15",
  "ic03",
  "it32 2: electric boogaloo"
};
static const size_t num_not_formats = sizeof(not_formats) / sizeof(not_formats[0]);


UNITTEST(icnscvt_max_images)
{
  ASSERT(num_formats <= ICNSCVT_MAX_IMAGES,
    "%zu > %d", num_formats, ICNSCVT_MAX_IMAGES);
}

UNITTEST(icnscvt_get_formats_list)
{
  icnscvt context = NULL;
  struct icns_data compare;
  icns_format_id list[ICNSCVT_MAX_IMAGES];
  icns_format_id compare_list[ICNSCVT_MAX_IMAGES];
  unsigned num;
  unsigned tmp;
  unsigned i;

  memset(&compare, 0, sizeof(compare));
  memset(list, 0, sizeof(list));
  memset(compare_list, 0, sizeof(compare_list));

  /* Error on null context. */
  num = icnscvt_get_formats_list(context, list, ICNSCVT_MAX_IMAGES);
  ASSERTEQ(num, 0, "%d != %d", num, 0);

  /* Error on junk context. */
  num = icnscvt_get_formats_list((icnscvt)&compare, list, ICNSCVT_MAX_IMAGES);
  ASSERTEQ(num, 0, "%d != %d", num, 0);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  /* Null pointer or zero count -> get count. */
  num = icnscvt_get_formats_list(context, NULL, 0);
  ASSERTEQ(num, num_formats, "%u != %zu", num, num_formats);
  ASSERTMEM(list, compare_list, sizeof(compare_list), "");
  num = icnscvt_get_formats_list(context, NULL, 12345);
  ASSERTEQ(num, num_formats, "%u != %zu", num, num_formats);
  ASSERTMEM(list, compare_list, sizeof(compare_list), "");
  num = icnscvt_get_formats_list(context, list, 0);
  ASSERTEQ(num, num_formats, "%u != %zu", num, num_formats);
  ASSERTMEM(list, compare_list, sizeof(compare_list), "");

  /* Full list -> return full list, which should match the copy in this file. */
  num = icnscvt_get_formats_list(context, list, ICNSCVT_MAX_IMAGES);
  ASSERTEQ(num, num_formats, "%u != %zu", num, num_formats);
  for(i = 0; i < num; i++)
  {
    ASSERTEQ(list[i], format_list[i].magic,
      "%u: %08lx != %08x", i, list[i], format_list[i].magic);
  }
  ASSERTMEM(list + num, compare_list + num,
    sizeof(list) - num * sizeof(list[0]), "");

  memset(list, 0, sizeof(list));

  /* Only memory for partial -> don't write past bound, still return total. */
  tmp = 11;
  num = icnscvt_get_formats_list(context, list, tmp);
  ASSERTEQ(num, num_formats, "%u != %zu", num, num_formats);
  for(i = 0; i < tmp; i++)
  {
    ASSERTEQ(list[i], format_list[i].magic,
      "%u: %08lx != %08x", i, list[i], format_list[i].magic);
  }
  ASSERTMEM(list + tmp, compare_list + tmp,
    sizeof(list) - tmp * sizeof(list[0]), "");

  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_get_format_id_by_name)
{
  icnscvt context = NULL;
  struct icns_data compare;
  icns_format_id id;
  size_t i;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  id = icnscvt_get_format_id_by_name(context, "s8mk");
  ASSERTEQ(id, 0, "%08lx != 0", id);

  /* Error on junk context. */
  id = icnscvt_get_format_id_by_name((icnscvt)&compare, "s8mk");
  ASSERTEQ(id, 0, "%08lx != 0", id);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  /* Valid names -> should always get the correct ID. */
  for(i = 0; i < num_formats; i++)
  {
    const struct test_format_api *t = format_list + i;

    id = icnscvt_get_format_id_by_name(context, t->name);
    ASSERTEQ(id, t->magic, "%s: %08lx != %08" PRIx32, t->name, id, t->magic);
  }

  /* Non-format names -> error. */
  for(i = 0; i < num_not_formats; i++)
  {
    const char *name = not_formats[i];
    id = icnscvt_get_format_id_by_name(context, name);
    ASSERTEQ(id, 0, "%s: %08lx != 0", name ? name : "NULL", id);
  }

  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_get_format_string)
{
  icnscvt context = NULL;
  struct icns_data compare;
  char buf[256];
  char buf2[64];
  unsigned num;
  size_t i;

  memset(&compare, 0, sizeof(compare));
  memset(buf, 0, sizeof(buf));

  /* Error on null context. */
  num = icnscvt_get_format_string(context,
    buf, sizeof(buf), icns_magic_s8mk);
  ASSERTEQ(num, 0, "%u != 0", num);

  /* Error on junk context. */
  num = icnscvt_get_format_string((icnscvt)&compare,
    buf, sizeof(buf), icns_magic_s8mk);
  ASSERTEQ(num, 0, "%u != 0", num);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  /* Error if the format is junk. */
  num = icnscvt_get_format_string(context, buf, sizeof(buf), 0);
  ASSERTEQ(num, 0, "%u != 0", num);
  num = icnscvt_get_format_string(context, buf, sizeof(buf), 0x12345678);
  ASSERTEQ(num, 0, "%u != 0", num);

  for(i = 0; i < num_formats; i++)
  {
    const struct test_format_api *t = format_list + i;
    const char *chk;

    /* 1) valid IDs -> should always get a string. */
    num = icnscvt_get_format_string(context, buf, sizeof(buf), t->magic);
    ASSERT(num > 0, "%s", t->name);

    /* 2) output should include format->name verbatim. */
    ASSERT(strstr(buf, t->name), "%s: name not in output: %s",
      t->name, buf);

    /* 3) output should include WxH@F somewhere. */
    if(t->factor > 1)
    {
      snprintf(buf2, sizeof(buf2), "%dx%d@%d",
        t->width, t->height, t->factor);
      ASSERT(strstr(buf, buf2), "%s: dims (%s) not in output: %s",
        t->name, buf2, buf);
    }

    /* 4) output should include (W*F)x(H*F) somewhere. */
    snprintf(buf2, sizeof(buf2), "%dx%d",
      t->width * t->factor, t->height * t->factor);
    ASSERT(strstr(buf, buf2), "%s: dims (%s) not in output: %s",
      t->name, buf2, buf);

    /* 5) output should include a type string somewhere. */
    switch(t->type)
    {
      default:
        FAIL("wtf?");
        break;
      case ICNS_1_BIT:
      case ICNS_1_BIT_WITH_MASK:
        chk = "1-bit";
        break;
      case ICNS_4_BIT:
        chk = "4-bit";
        break;
      case ICNS_8_BIT:
        chk = "8-bit";
        break;
      case ICNS_24_BIT:
        chk = "24-bit RGB";
        break;
      case ICNS_8_BIT_MASK:
        chk = "8-bit mask";
        break;
      case ICNS_24_BIT_OR_PNG:
      case ICNS_PNG:
      case ICNS_ARGB_OR_PNG:
        chk = "PNG";
        break;
    }
    ASSERT(strstr(buf, chk), "%s: format check string (%s) not in output: %s",
      t->name, chk, buf);
  }

  icnscvt_destroy_context(context);
}
