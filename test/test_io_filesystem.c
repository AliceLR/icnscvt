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

/***** Filesystem functions for .iconset *****/

static void check_open_test_file(const char *path_relative)
{
  FILE *f;
  char buf[12];
  size_t num;

  f = fopen(path_relative, "r");
  ASSERT(f, "failed to open DATA_DIR/dirent/a_file");

  num = fread(buf, 1, 11, f);
  fclose(f);
  ASSERTEQ(num, 11, "failed to read a_file");
  buf[11] = '\0';
  ASSERTCMP(buf, "placeholder", "file contents are incorrect");
}

UNITTEST(io_get_file_type)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  int ret;

  ret = icns_get_file_type(DATA_DIR "/dirent/a_file");
  ASSERTEQ(ret, IO_REG, "not a file");
  ret = icns_get_file_type(DATA_DIR "/dirent/a_dir");
  ASSERTEQ(ret, IO_DIR, "not a dir");
  ret = icns_get_file_type(".");
  ASSERTEQ(ret, IO_DIR, "not a dir");
  ret = icns_get_file_type(DATA_DIR "/dirent/wjerjejkdsjfd");
  ASSERTEQ(ret, IO_NOTEXIST, "shouldn't exist");
#endif
}

UNITTEST(io_chdir)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  enum icns_error ret;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  /* Check test file from initial dir. */
  check_open_test_file(DATA_DIR "/dirent/a_file");

  ret = icns_chdir(&icns, DATA_DIR);
  check_ok(&icns, ret);
  check_open_test_file("dirent/a_file");

  /* Can't change to nonexistent dir */
  ret = icns_chdir(&icns, "sdijfsldkjfdslk");
  check_error(&icns, ret, ICNS_FILESYSTEM_ERROR);

  /* Should still be in same location */
  check_open_test_file("dirent/a_file");

  ret = icns_chdir(&icns, "../..");
  check_ok(&icns, ret);

  /* Check test file from what should be the original dir. */
  check_open_test_file(DATA_DIR "/dirent/a_file");
#endif
}

UNITTEST(io_getcwd)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  static char buf[4096];
  static char buf2[4096];
  enum icns_error ret;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ret = icns_getcwd(&icns, buf, sizeof(buf));
  check_ok(&icns, ret);

  ret = icns_chdir(&icns, DATA_DIR);
  check_ok(&icns, ret);

  ret = icns_getcwd(&icns, buf2, sizeof(buf2));
  check_ok(&icns, ret);

  ASSERT(strlen(buf) < strlen(buf2), "first getcwd should be shorter");
  ASSERTMEM(buf, buf2, strlen(buf), "first getcwd should be a prefix of second");
#endif
}

UNITTEST(io_mkdir)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  enum icns_error ret;
  FILE *f;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/mkdir_test"), IO_NOTEXIST, "");
  ASSERTEQ(icns_get_file_type(TEMP_DIR "/mkdir_file"), IO_NOTEXIST, "");

  ret = icns_mkdir(&icns, TEMP_DIR "/mkdir_test");
  check_ok(&icns, ret);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/mkdir_test"), IO_DIR, "");

  /* Can't mkdir over existing dir */
  ret = icns_mkdir(&icns, TEMP_DIR "/mkdir_test");
  check_error(&icns, ret, ICNS_FILESYSTEM_ERROR);

  f = fopen(TEMP_DIR "/mkdir_file", "w");
  ASSERT(f, "failed to open mkdir file");
  fclose(f);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/mkdir_file"), IO_FILE, "");

  /* Can't mkdir over existing file */
  ret = icns_mkdir(&icns, TEMP_DIR "/mkdir_file");
  check_error(&icns, ret, ICNS_FILESYSTEM_ERROR);
#endif
}

UNITTEST(io_unlink)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  enum icns_error ret;
  FILE *f;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_file"), IO_NOTEXIST, "");
  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_dir"), IO_NOTEXIST, "");

  f = fopen(TEMP_DIR "/unlink_file", "w");
  ASSERT(f, "failed to open unlink file");
  fclose(f);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_file"), IO_FILE, "");

  ret = icns_mkdir(&icns, TEMP_DIR "/unlink_dir");
  check_ok(&icns, ret);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_dir"), IO_DIR, "");

  /* Should be able to unlink file */
  ret = icns_unlink(&icns, TEMP_DIR "/unlink_file");
  check_ok(&icns, ret);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_file"), IO_NOTEXIST, "");

  /* Can't unlink nonexistent file */
  ret = icns_unlink(&icns, TEMP_DIR "/unlink_file");
  check_error(&icns, ret, ICNS_FILESYSTEM_ERROR);

  /* Can't unlink dir */
  ret = icns_unlink(&icns, TEMP_DIR "/unlink_dir");
  check_error(&icns, ret, ICNS_FILESYSTEM_ERROR);

  ASSERTEQ(icns_get_file_type(TEMP_DIR "/unlink_dir"), IO_DIR, "");
#endif
}

struct dir_expected
{
  const char *name;
  int type;
};

static const struct icns_dir_entry *get_file_in_directory_by_name(
 const struct icns_dir_entry *list, const char *name)
{
  while(list)
  {
    if(!strcmp(list->name, name))
      return list;
    list = list->next;
  }
  return NULL;
}

UNITTEST(io_read_directory)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  static const struct dir_expected expected[] =
  {
    { "README", IO_FILE },
    { "a_file", IO_FILE },
    { "a_dir", IO_DIR },
  };
  size_t num_expected = sizeof(expected) / sizeof(expected[0]);
  size_t num;
  size_t i;
  struct icns_dir_entry *base;
  struct icns_dir_entry *pos;
  enum icns_error ret;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  ASSERTEQ(icns_get_file_type(DATA_DIR "/dirent"), IO_DIR, "");
  ASSERTEQ(icns_get_file_type(DATA_DIR "/dirent/README"), IO_FILE, "");
  ASSERTEQ(icns_get_file_type(DATA_DIR "/dirent/a_file"), IO_FILE, "");
  ASSERTEQ(icns_get_file_type(DATA_DIR "/dirent/a_dir"), IO_DIR, "");
  ASSERTEQ(icns_get_file_type(DATA_DIR "/dirent/whatever"), IO_NOTEXIST, "");

  ret = icns_read_directory(&icns, &base, DATA_DIR "/dirent");
  check_ok(&icns, ret);

  /* Should be the same number of entries. */
  num = 0;
  for(pos = base; pos; pos = pos->next)
    num++;

  ASSERTEQ(num, num_expected, "%zu != %zu", num, num_expected);

  /* Check individual files. */
  for(i = 0; i < num_expected; i++)
  {
    const struct icns_dir_entry *which =
     get_file_in_directory_by_name(base, expected[i].name);

    ASSERT(which, "'%s' not in directory", expected[i].name);
    ASSERT(which->type == IO_UNKNOWN || (int)which->type == expected[i].type,
     "%d (not supported by all implementations)", which->type);
  }

  ASSERTEQ(get_file_in_directory_by_name(base, "whatever"), NULL,
   "file 'whatever' should not exist in directory");

  icns_free_directory(base);
#endif
}
