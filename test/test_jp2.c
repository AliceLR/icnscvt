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
#include "../src/icns.h"
#include "../src/icns_jp2.h"

struct test_jp2
{
  const char *path;
  struct icns_jp2_stat st;
};

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

static const struct test_jp2 jp2_files[] =
{
  { PNG_DIR "/16x16.jp2",     {   16,   16, 32 }},
  { PNG_DIR "/16x16.j2k",     {   16,   16, 32 }},
  { PNG_DIR "/18x18.j2k",     {   18,   18, 32 }},
  { PNG_DIR "/24x24.j2k",     {   24,   24, 32 }},
  { PNG_DIR "/32x32.j2k",     {   32,   32, 32 }},
  { PNG_DIR "/36x36.j2k",     {   36,   36, 32 }},
  { PNG_DIR "/48x48.j2k",     {   48,   48, 32 }},
  { PNG_DIR "/64x64.j2k",     {   64,   64, 32 }},
  { PNG_DIR "/128x128.j2k",   {  128,  128, 32 }},
  { PNG_DIR "/256x256.j2k",   {  256,  256, 32 }},
  { PNG_DIR "/512x512.j2k",   {  512,  512, 32 }},
  { PNG_DIR "/1024x1024.j2k", { 1024, 1024, 32 }},
};
static const size_t num_jp2_files = sizeof(jp2_files) / sizeof(jp2_files[0]);

static const char * const not_jp2_files[] =
{
  PNG_DIR "/16x16.png",
  PNG_DIR "/16x16.tga.gz",
};
static const size_t num_not_jp2_files =
  sizeof(not_jp2_files) / sizeof(not_jp2_files[0]);


UNITTEST(jp2_icns_is_file_jp2)
{
  uint8_t *buf;
  size_t buf_size;
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ASSERTEQ(icns_is_file_jp2(jp2_a, sizeof(jp2_a)), true, "");
  ASSERTEQ(icns_is_file_jp2(jp2_b, sizeof(jp2_b)), true, "");

  ASSERTEQ(icns_is_file_jp2(jp2_a, 3), false, "");
  ASSERTEQ(icns_is_file_jp2(jp2_b, 11), false, "");
  ASSERTEQ(icns_is_file_jp2(not_a, sizeof(not_a)), false, "");
  ASSERTEQ(icns_is_file_jp2(not_b, sizeof(not_b)), false, "");
  ASSERTEQ(icns_is_file_jp2("", 0), false, "");

  for(i = 0; i < num_jp2_files; i++)
  {
    test_load(&icns, &buf, &buf_size, jp2_files[i].path);
    ASSERTEQ(icns_is_file_jp2(buf, buf_size), true, "%s", jp2_files[i].path);
    free(buf);
  }

  for(i = 0; i < num_not_jp2_files; i++)
  {
    test_load(&icns, &buf, &buf_size, not_jp2_files[i]);
    ASSERTEQ(icns_is_file_jp2(buf, buf_size), false, "%s", not_jp2_files[i]);
    free(buf);
  }
}

UNITTEST(jp2_icns_get_jp2_info)
{
  struct icns_jp2_stat st;
  struct icns_jp2_stat chk;
  enum icns_error ret;
  uint8_t *buf;
  size_t buf_size;
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  for(i = 0; i < num_jp2_files; i++)
  {
    const struct test_jp2 *test = &jp2_files[i];
    test_load(&icns, &buf, &buf_size, test->path);

    ret = icns_get_jp2_info(&icns, &st, buf, buf_size);
    check_ok(&icns, ret);
    ASSERTEQ(st.width, test->st.width, "%s: width %u != %u",
      test->path, st.width, test->st.width);
    ASSERTEQ(st.height, test->st.height, "%s: height %u != %u",
      test->path, st.height, test->st.height);
    ASSERTEQ(st.depth, test->st.depth, "%s: depth %u != %u",
      test->path, st.depth, test->st.depth);

    free(buf);
  }

  /* All failed calls should leave st unmodified. */
  memset(&st, 0xff, sizeof(st));
  memcpy(&chk, &st, sizeof(st));

  for(i = 0; i < num_not_jp2_files; i++)
  {
    test_load(&icns, &buf, &buf_size, not_jp2_files[i]);

    ret = icns_get_jp2_info(&icns, &st, buf, buf_size);
    check_error(&icns, ret, ICNS_JP2_NOT_A_JP2);
    ASSERTMEM(&st, &chk, sizeof(st), "%s: st was modified by failed call",
      not_jp2_files[i]);

    free(buf);
  }

  /* Both of these are invalid data streams and should als fail. */

  ret = icns_get_jp2_info(&icns, &st, jp2_a, sizeof(jp2_a));
  check_error(&icns, ret, ICNS_JP2_DATA_ERROR);
  ASSERTMEM(&st, &chk, sizeof(st), "st was modified by failed call");

  ret = icns_get_jp2_info(&icns, &st, jp2_b, sizeof(jp2_b));
  check_error(&icns, ret, ICNS_JP2_DATA_ERROR);
  ASSERTMEM(&st, &chk, sizeof(st), "st was modified by failed call");
}
