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

#include <stddef.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

NOT_NULL
FILE *icns_io_fopen(const char *path, const char *mode)
{
  return fopen(path, mode);
}

NOT_NULL
int icns_io_get_file_type(const char *path)
{
  struct stat st;

  /* broken MemorySanitizer instrumentation workaround */
  memset(&st, 0, sizeof(struct stat));

  if(stat(path, &st))
    return IO_NOTEXIST;

  if(S_ISREG(st.st_mode))
    return IO_REG;
  if(S_ISDIR(st.st_mode))
    return IO_DIR;

  return IO_OTHER;
}

NOT_NULL
int icns_io_chdir(const char *path)
{
  return chdir(path);
}

NOT_NULL
char *icns_io_getcwd(char *dest, size_t dest_size)
{
  return getcwd(dest, dest_size);
}

NOT_NULL
int icns_io_mkdir(const char *path)
{
  return mkdir(path, 0755);
}

NOT_NULL
int icns_io_unlink(const char *path)
{
  return unlink(path);
}

typedef DIR icns_dirtype;

NOT_NULL
icns_dirtype *icns_io_opendir(const char *path)
{
  return opendir(path);
}

NOT_NULL
struct icns_dir_entry *icns_io_readdir(icns_dirtype *dir)
{
  struct icns_dir_entry *ent;
  struct dirent *d;
  size_t name_len;
  size_t sz;
  size_t bad_implementation_workaround = 0;

  while(1)
  {
    d = readdir(dir);
    if(!d)
      return NULL;

    /* Reject "", ".", "..", which may exist in some implementations. */
    if(d->d_name[0] && strcmp(d->d_name, ".") && strcmp(d->d_name, ".."))
      break;

    /* so broken libc implementations like pspdev don't hang forever */
    if((++bad_implementation_workaround) > 100)
      return NULL;
  }

  name_len = strlen(d->d_name);
  sz = name_len + 1 + offsetof(struct icns_dir_entry, name);
  if(sz < sizeof(struct icns_dir_entry))
    sz = sizeof(struct icns_dir_entry);

  ent = (struct icns_dir_entry *)malloc(sz);
  if(!ent)
    return NULL;

  memcpy(ent->name, d->d_name, name_len + 1);
  ent->next = NULL;

#ifdef DT_UNKNOWN
  switch(d->d_type)
  {
    case DT_UNKNOWN:
      ent->type = IO_UNKNOWN;
      break;
    case DT_REG:
      ent->type = IO_REG;
      break;
    case DT_DIR:
      ent->type = IO_DIR;
      break;
    default:
      ent->type = IO_OTHER;
      break;
  }
#else
  ent->type = IO_UNKNOWN;
#endif

  return ent;
}

NOT_NULL
void icns_io_closedir(icns_dirtype *dir)
{
  closedir(dir);
}
