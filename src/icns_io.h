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

#ifndef ICNSCVT_IO_H
#define ICNSCVT_IO_H

#include "common.h"

ICNS_BEGIN_DECLS

NOT_NULL
static inline void icns_put_u32be(uint8_t *buf, uint32_t v)
{
  buf[0] = v >> 24;
  buf[1] = v >> 16;
  buf[2] = v >> 8;
  buf[3] = v;
}

NOT_NULL
static inline uint32_t icns_get_u32be(const uint8_t *buf)
{
  return MAGIC(buf[0], buf[1], buf[2], buf[3]);
}

enum icns_error icns_io_init_read(struct icns_data *icns,
 void *read_priv, size_t (*read_fn)(void *, size_t, void *));
enum icns_error icns_io_init_write(struct icns_data *icns,
 void *write_priv, size_t (*write_fn)(const void *, size_t, void *));

enum icns_error icns_io_init_read_memory(struct icns_data *icns,
 const void *src, size_t src_size) NOT_NULL;
enum icns_error icns_io_init_write_memory(struct icns_data *icns,
 void *dest, size_t dest_size) NOT_NULL;

enum icns_error icns_io_init_read_file(struct icns_data *icns,
 const char *filename) NOT_NULL;
enum icns_error icns_io_init_write_file(struct icns_data *icns,
 const char *filename) NOT_NULL;

void icns_io_end(struct icns_data *icns) NOT_NULL;

enum icns_error icns_read_direct(struct icns_data *icns,
 uint8_t *dest, size_t count) NOT_NULL;
enum icns_error icns_load_direct(struct icns_data *icns,
 uint8_t **dest, size_t count) NOT_NULL;
enum icns_error icns_write_direct(struct icns_data *icns,
 const uint8_t *src, size_t count) NOT_NULL;

enum icns_error icns_read_chunk_header(struct icns_data *icns,
 struct icns_chunk_header *dest) NOT_NULL;
enum icns_error icns_write_chunk_header(struct icns_data *icns,
 const struct icns_chunk_header *src) NOT_NULL;

/* Filesystem functions for .iconset read/write. */
struct icns_dir_entry
{
  struct icns_dir_entry *next;
  enum
  {
    IO_NOTEXIST,
    IO_UNKNOWN,
    IO_REG,
    IO_DIR,
    IO_OTHER
  } type;
  char name[1];
};

int icns_get_file_type(const char *path) NOT_NULL;
enum icns_error icns_chdir(struct icns_data *icns, const char *path) NOT_NULL;
enum icns_error icns_getcwd(struct icns_data *icns,
 char *dest, size_t dest_size) NOT_NULL;
enum icns_error icns_mkdir(struct icns_data *icns, const char *path) NOT_NULL;
enum icns_error icns_unlink(struct icns_data *icns, const char *path) NOT_NULL;
enum icns_error icns_read_directory(struct icns_data *icns,
 struct icns_dir_entry **dest, const char *path) NOT_NULL;
void icns_free_directory(struct icns_dir_entry *entries);

ICNS_END_DECLS

#endif /* ICNSCVT_IO_H */
