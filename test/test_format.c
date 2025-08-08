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
/*
#include "../src/icns_format_argb.h"
*/
#include "../src/icns_format_mask.h"
#include "../src/icns_format_png.h"

struct format_test
{
  const struct icns_format *format;
  const struct icns_format *mask;
  bool is_mask;
  bool is_jp2;
};

static const struct format_test format_test_list[] =
{
  /*
  { &icns_format_ICON, NULL,              false, false },
  { &icns_format_icmp, NULL,              false, false },
  { &icns_format_icsp, NULL,              false, false },
  { &icns_format_ICNp, NULL,              false, false },
  { &icns_format_ichp, NULL,              false, false },
  */
  /*
  { &icns_format_icm4, NULL,              false, false },
  { &icns_format_ics4, NULL,              false, false },
  { &icns_format_icl4, NULL,              false, false },
  { &icns_format_ich4, NULL,              false, false },
  */
  /*
  { &icns_format_icm8, NULL,              false, false },
  { &icns_format_ics8, NULL,              false, false },
  { &icns_format_icl8, NULL,              false, false },
  { &icns_format_ich8, NULL,              false, false },
  */
  /*
  { &icns_format_is32, &icns_format_s8mk, false, false },
  { &icns_format_il32, &icns_format_l8mk, false, false },
  { &icns_format_ih32, &icns_format_h8mk, false, false },
  { &icns_format_it32, &icns_format_t8mk, false, false },
  */
  { &icns_format_s8mk, NULL,              true,  false },
  { &icns_format_l8mk, NULL,              true,  false },
  { &icns_format_h8mk, NULL,              true,  false },
  { &icns_format_t8mk, NULL,              true,  false },
  /*
  { &icns_format_icp4, NULL,              false, true },
  { &icns_format_icp5, NULL,              false, true },
  */
  { &icns_format_icp6, NULL,              false, true },
  /*
  { &icns_format_ic04, NULL,              false, true },
  { &icns_format_ic05, NULL,              false, true },
  */
  { &icns_format_ic07, NULL,              false, true },
  { &icns_format_ic08, NULL,              false, true },
  { &icns_format_ic09, NULL,              false, true },
  { &icns_format_ic10, NULL,              false, true },
  { &icns_format_ic11, NULL,              false, true },
  { &icns_format_ic12, NULL,              false, true },
  { &icns_format_ic13, NULL,              false, true },
  { &icns_format_ic14, NULL,              false, true },
  /*
  { &icns_format_icsb, NULL,              false, true },
  */
  { &icns_format_icsB, NULL,              false, true },
  { &icns_format_sb24, NULL,              false, true },
  { &icns_format_SB24, NULL,              false, true },
};
static const size_t num_test_list =
 sizeof(format_test_list) / sizeof(format_test_list[0]);


static const uint32_t bad_magics[] =
{
  0,
  0xfffffffful,
  MAGIC('i','c','n','s'),
  MAGIC('T','O','C',' '),
  MAGIC('i','c','n','V'),
  MAGIC('n','a','m','e'),
  MAGIC('i','n','f','o'),
  MAGIC('s','b','t','p'),
  MAGIC('s','l','c','t'),
  MAGIC(0xfd, 0xd9, 0x2f, 0xa8),
};
static const size_t num_bad_magics = sizeof(bad_magics) / sizeof(bad_magics[0]);


static const char *bad_names[] =
{
  "",
  "icns",
  "TOC ",
  "icnv",
  "name",
  "info",
  "sbtp",
  "slct",
  "\xfd\xd9\x2f\xa8",
  "sdjkflsdkjfsdklf",
  "icl444444",
  "t8mj",
  "ic15",
};
static const size_t num_bad_names = sizeof(bad_names) / sizeof(bad_names[0]);


UNITTEST(format_check_pointers)
{
  /* There should never be any null pointers except for the
   * completely optional prepare_for_external */
  size_t i;
  size_t j;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;

    ASSERT(format->prepare_for_icns, "%s", format->name);
    ASSERT(format->read_from_icns, "%s", format->name);
    ASSERT(format->write_to_icns, "%s", format->name);
    ASSERT(format->read_from_external, "%s", format->name);
    ASSERT(format->write_to_external, "%s", format->name);

    for(j = i + 1; j < num_test_list; j++)
    {
      const struct icns_format *check = format_test_list[j].format;
      ASSERT(format != check, "duplicate entry for %s in test list", format->name);
    }
  }
}

UNITTEST(format_icns_get_format_string)
{
  char buf[256];
  char buf2[256];
  const char *chk;
  size_t expected_length;
  size_t tmp;
  size_t i;

  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;

    /* 1) snprintf semantics: the length returned should always be the length
     *    of the full string, e.g. the call with the full length plus 1
     *    should never truncate. A call with the full length should truncate.
     */
    expected_length = icns_get_format_string(NULL, 0, format);
    ASSERT(expected_length,
      "%s: error calculating format string length", format->name);
    tmp = icns_get_format_string(NULL, 12345, format);
    ASSERTEQ(expected_length, tmp,
      "%s: null ptr and non-zero length should -> zero length", format->name);
    ASSERT(expected_length < sizeof(buf) + 1,
      "%s: format string length too long: %zu > %zu", format->name,
      expected_length, sizeof(buf) + 1);

    tmp = icns_get_format_string(buf, expected_length + 1, format);
    ASSERTEQ(expected_length, tmp, "%s: return value wrong: %zu != %zu",
      format->name, expected_length, tmp);
    tmp = icns_get_format_string(buf2, expected_length, format);
    ASSERTEQ(expected_length, tmp, "%s: return value wrong: %zu != %zu",
      format->name, expected_length, tmp);

    ASSERT(buf[expected_length - 1] != '\0', "%s: wrong snprintf return value",
      format->name);
    ASSERT(buf[expected_length] == '\0', "%s: nul terminator missing?!",
      format->name);
    ASSERT(buf2[expected_length - 1] == '\0', "%s: should be truncated",
      format->name);
    ASSERTMEM(buf, buf2, expected_length - 1, "%s: should be the same prefix",
      format->name);

    /* 2) output should include format->name verbatim. */
    ASSERT(strstr(buf, format->name), "%s: name not in output: %s",
      format->name, buf);

    /* 3) output should include WxH@F somewhere. */
    if(format->factor > 1)
    {
      snprintf(buf2, sizeof(buf2), "%dx%d@%d",
        format->width, format->height, format->factor);
      ASSERT(strstr(buf, buf2), "%s: dims (%s) not in output: %s",
        format->name, buf2, buf);
    }

    /* 4) output should include (W*F)x(H*F) somewhere. */
    snprintf(buf2, sizeof(buf2), "%dx%d",
      format->width * format->factor, format->height * format->factor);
    ASSERT(strstr(buf, buf2), "%s: dims (%s) not in output: %s",
      format->name, buf2, buf);

    /* 5) output should include a type string somewhere. */
    switch(format->type)
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
      format->name, chk, buf);
  }
}

UNITTEST(format_icns_get_format_list)
{
  size_t ret;
  size_t i;
#define LIST_SIZE 64
  const struct icns_format *list[LIST_SIZE];

  /* NULL -> get count only. */
  ret = icns_get_format_list(NULL, 0);
  ASSERTEQ(ret, num_test_list, "%zu != %zu", ret, num_test_list);
  ret = icns_get_format_list(NULL, 12345);
  ASSERTEQ(ret, num_test_list, "%zu != %zu", ret, num_test_list);

  /* Full list */
  memset(list, 0, sizeof(list));
  ret = icns_get_format_list(list, sizeof(list) / sizeof(list[0]));
  ASSERTEQ(ret, num_test_list, "%zu != %zu", ret, num_test_list);
  for(i = 0; i < ret; i++)
  {
    ASSERTEQ(list[i], format_test_list[i].format, "%s != %s",
      list[i]->name, format_test_list[i].format->name);
  }
  for(; i < LIST_SIZE; i++)
    ASSERTEQ(list[i], NULL, "%s != NULL", list[i]->name);

  /* Truncated list */
  memset(list, 0, sizeof(list));
  ret = icns_get_format_list(list, 9);
  for(i = 0; i < 9; i++)
  {
    ASSERTEQ(list[i], format_test_list[i].format, "%s != %s",
      list[i]->name, format_test_list[i].format->name);
  }
  for(; i < LIST_SIZE; i++)
    ASSERTEQ(list[i], NULL, "%s != NULL", list[i]->name);
}

UNITTEST(format_icns_get_format_by_magic)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    const struct icns_format *ret = icns_get_format_by_magic(format->magic);
    ASSERTEQ(ret, format, "%s: %p != %p", format->name, (void *)ret, (void *)format);
  }

  for(i = 0; i < num_bad_magics; i++)
  {
    const struct icns_format *ret = icns_get_format_by_magic(bad_magics[i]);
    ASSERTEQ(ret, NULL, "%08" PRIx32 ": %p != %p", bad_magics[i], (void *)ret, NULL);
  }
}

UNITTEST(format_icns_get_format_by_name)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    const struct icns_format *ret = icns_get_format_by_name(format->name);
    ASSERTEQ(ret, format, "%s: %p != %p", format->name, (void *)ret, (void *)format);
  }

  for(i = 0; i < num_bad_names; i++)
  {
    const struct icns_format *ret = icns_get_format_by_name(bad_names[i]);
    ASSERTEQ(ret, NULL, "%s: %p != %p",
      bad_names[i] ? bad_names[i] : "NULL", (void *)ret, NULL);
  }
}

UNITTEST(format_icns_get_mask_for_format)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    const struct icns_format *expected = format_test_list[i].mask;
    const struct icns_format *ret = icns_get_mask_for_format(format);
    ASSERTEQ(ret, expected, "%s: %p != %p", format->name, (void *)ret, (void *)expected);
  }
}

UNITTEST(format_icns_format_is_mask)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    bool expected = format_test_list[i].is_mask;
    ASSERTEQ(icns_format_is_mask(format), expected, "%s", format->name);
  }
}

UNITTEST(format_icns_format_supports_png)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    /* Currently, this 100% overlaps with the formats that support JPEG 2000. */
    bool expected = format_test_list[i].is_jp2;
    ASSERTEQ(icns_format_supports_png(format), expected, "%s", format->name);
  }
}

UNITTEST(format_icns_format_supports_jpeg_2000)
{
  size_t i;
  for(i = 0; i < num_test_list; i++)
  {
    const struct icns_format *format = format_test_list[i].format;
    bool expected = format_test_list[i].is_jp2;
    ASSERTEQ(icns_format_supports_jpeg_2000(format), expected, "%s", format->name);
  }
}
