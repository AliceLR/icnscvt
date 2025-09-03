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

#include <errno.h>

#ifdef _WIN32
#include "icns_io_win32.h"
#else
#include "icns_io_posix.h"
#endif

/**
 * Prepare the current state for a generic read operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param read_priv   private data for read callback.
 * @param read_fn     read callback.
 * @return            `ICNS_OK` on success, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_read(struct icns_data *icns,
 void *read_priv, size_t (*read_fn)(void *, size_t, void *))
{
  /* Can't nonnull everything since priv can be NULL. */
  if(!icns)
    return ICNS_INTERNAL_ERROR;

  if(icns->read_priv || icns->read_fn)
  {
    E_("read function already set!");
    return ICNS_INTERNAL_ERROR;
  }

  if(icns->write_priv || icns->write_fn)
  {
    E_("can't set read function--already in write mode");
    return ICNS_INTERNAL_ERROR;
  }

  icns->io.type = IO_CALLBACK;
  icns->read_priv = read_priv;
  icns->read_fn = read_fn;
  icns->bytes_in = 0;
  return ICNS_OK;
}

/**
 * Prepare the current state for a generic write operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param read_priv   private data for write callback.
 * @param read_fn     write callback.
 * @return            `ICNS_OK` on success, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_write(struct icns_data *icns,
 void *write_priv, size_t (*write_fn)(const void *, size_t, void *))
{
  /* Can't generic nonnull since priv can be NULL. */
  if(!icns)
    return ICNS_INTERNAL_ERROR;

  if(icns->write_priv || icns->write_fn)
  {
    E_("write function already set!");
    return ICNS_INTERNAL_ERROR;
  }

  if(icns->read_priv || icns->read_fn)
  {
    E_("can't set write function--already in read mode");
    return ICNS_INTERNAL_ERROR;
  }

  icns->io.type = IO_CALLBACK;
  icns->write_priv = write_priv;
  icns->write_fn = write_fn;
  icns->bytes_out = 0;
  return ICNS_OK;
}

static size_t icns_io_read_mem_func(void *dest, size_t count, void *priv)
{
  struct icns_data *icns = (struct icns_data *)priv;
  size_t left;

  if(!icns || icns->io.pos >= icns->io.size)
    return 0;

  left = icns->io.size - icns->io.pos;
  if(count > left)
    count = left;

  memcpy(dest, icns->io.ptr.src + icns->io.pos, count);
  icns->io.pos += count;
  return count;
}

static size_t icns_io_write_mem_func(const void *src, size_t count, void *priv)
{
  struct icns_data *icns = (struct icns_data *)priv;
  size_t left;

  if(!icns || icns->io.pos >= icns->io.size)
    return 0;

  left = icns->io.size - icns->io.pos;
  if(count > left)
    count = left;

  memcpy(icns->io.ptr.dest + icns->io.pos, src, count);
  icns->io.pos += count;
  return count;
}

/**
 * Prepare the current state for a memory read operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param src         memory to read data from.
 * @param src_size    size of memory.
 * @return            `ICNS_OK` on success, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_read_memory(struct icns_data *icns,
 const void *src, size_t src_size)
{
  enum icns_error ret;

  ret = icns_io_init_read(icns, icns, icns_io_read_mem_func);
  if(ret)
    return ret;

  icns->io.type = IO_MEMORY;
  icns->io.ptr.src = (const uint8_t *)src;
  icns->io.pos = 0;
  icns->io.size = src_size;
  return ICNS_OK;
}

/**
 * Prepare the current state for a memory write operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param dest        memory to write to.
 * @param dest_size   size of memory.
 * @return            `ICNS_OK` on success, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_write_memory(struct icns_data *icns,
 void *dest, size_t dest_size)
{
  enum icns_error ret;

  ret = icns_io_init_write(icns, icns, icns_io_write_mem_func);
  if(ret)
    return ret;

  icns->io.type = IO_MEMORY;
  icns->io.ptr.dest = (uint8_t *)dest;
  icns->io.pos = 0;
  icns->io.size = dest_size;
  return ICNS_OK;
}

static size_t icns_io_read_file_func(void *dest, size_t count, void *priv)
{
  struct icns_data *icns = (struct icns_data *)priv;
  if(!icns || !count)
    return 0;

  return fread(dest, 1, count, icns->io.ptr.f);
}

static size_t icns_io_write_file_func(const void *src, size_t count, void *priv)
{
  struct icns_data *icns = (struct icns_data *)priv;
  if(!icns || !count)
    return 0;

  return fwrite(src, 1, count, icns->io.ptr.f);
}

/**
 * Prepare the current state for a filesystem read operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param filename    name of the file to read from.
 * @return            `ICNS_OK` on success, `ICNS_READ_OPEN_ERROR` if the file
 *                    fails to open, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_read_file(struct icns_data *icns, const char *filename)
{
  enum icns_error ret;
  FILE *f;

  ret = icns_io_init_read(icns, icns, icns_io_read_file_func);
  if(ret)
    return ret;

  f = icns_io_fopen(filename, "rb");
  if(!f)
  {
    E_("failed to open file '%s'", filename);
    icns_io_end(icns);
    return ICNS_READ_OPEN_ERROR;
  }
  icns->io.type = IO_FILE;
  icns->io.ptr.f = f;
  return ICNS_OK;
}

/**
 * Prepare the current state for a filesystem write operation.
 * If the current state is already prepared for either read or write,
 * this function will fail.
 *
 * @param icns        current state data.
 * @param filename    name of the file to write to.
 * @return            `ICNS_OK` on success, `ICNS_WRITE_OPEN_ERROR` if the file
 *                    fails to open, otherwise `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_io_init_write_file(struct icns_data *icns, const char *filename)
{
  enum icns_error ret;
  FILE *f;

  ret = icns_io_init_write(icns, icns, icns_io_write_file_func);
  if(ret)
    return ret;

  f = icns_io_fopen(filename, "wb");
  if(!f)
  {
    E_("failed to open file '%s' for write", filename);
    icns_io_end(icns);
    return ICNS_WRITE_OPEN_ERROR;
  }
  icns->io.type = IO_FILE;
  icns->io.ptr.f = f;
  return ICNS_OK;
}

/**
 * Reset the current state for either read or write operations.
 * This should be called when a read operation or a write operation finishes.
 *
 * @param icns        curent state data.
 */
void icns_io_end(struct icns_data *icns)
{
  if(icns->io.type == IO_FILE)
  {
    fclose(icns->io.ptr.f);
    icns->io.ptr.f = NULL;
  }
  icns->io.type = IO_NONE;

  icns->read_priv = NULL;
  icns->read_fn = NULL;
  icns->bytes_in = 0;

  icns->write_priv = NULL;
  icns->write_fn = NULL;
  icns->bytes_out = 0;
}

/**
 * Read from the currently open stream to an existing buffer.
 *
 * @param icns        current state data.
 * @param dest        buffer to read data to.
 * @param count       amount of data to read.
 * @return            `ICNS_OK` on success, otherwise `ICNS_READ_ERROR` or
 *                    `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_read_direct(struct icns_data *icns,
 uint8_t *dest, size_t count)
{
  size_t count_in;
  if(!icns->read_fn)
  {
    E_("reader is NULL");
    return ICNS_INTERNAL_ERROR;
  }

  count_in = icns->read_fn(dest, count, icns->read_priv);
  icns->bytes_in += count_in;
  if(count_in < count)
  {
    E_("failed to read file into buffer");
    return ICNS_READ_ERROR;
  }
  return ICNS_OK;
}

/**
 * Read from the currently open stream to a newly allocated buffer.
 * A buffer will always be allocated on success, even if the requested
 * count was zero.
 *
 * @param icns        current state data.
 * @param dest        the allocated buffer pointer containing the read data
 *                    will be stored here on success.
 * @param count       amount of data to read.
 * @return            `ICNS_OK` on success, otherwise `ICNS_ALLOC_ERROR`,
 *                    `ICNS_READ_ERROR`, or `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_load_direct(struct icns_data *icns,
 uint8_t **dest, size_t count)
{
  enum icns_error ret;
  void *buf;
  if(!icns->read_fn)
  {
    E_("reader is NULL");
    return ICNS_INTERNAL_ERROR;
  }

  buf = malloc(count ? count : 1);
  if(!buf)
  {
    E_("failed to allocate buffer");
    return ICNS_ALLOC_ERROR;
  }
  /* Initialize the minimal buffer in case len == 0. */
  *((uint8_t *)buf) = '\0';

  ret = icns_read_direct(icns, (uint8_t *)buf, count);
  if(ret)
  {
    free(buf);
    return ret;
  }
  *dest = (uint8_t *)buf;
  return ICNS_OK;
}

/**
 * Read from the currently open stream to a newly allocated buffer.
 * This function will read as many bytes from the stream as possible,
 * adjusting the size of the allocation accordingly.
 *
 * @param icns        current state data.
 * @param dest        the allocated buffer pointer containing the read data
 *                    will be stored here on success.
 * @param size        the final size of the allocated buffer will be stored
 *                    here on success.
 * @return            `ICNS_OK` on success, otherwise `ICNS_ALLOC_ERROR`,
 *                    `ICNS_READ_ERROR`, or `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_load_direct_auto(struct icns_data *icns,
 uint8_t **dest, size_t *size)
{
  uint8_t *buf = NULL;
  void *tmp;
  size_t alloc = 0;
  size_t sz = 0;

  if(!icns->read_fn)
  {
    E_("reader is NULL");
    return ICNS_INTERNAL_ERROR;
  }

  sz = 0;
  alloc = 0;
  while(sz == alloc)
  {
    alloc = alloc ? alloc << 1 : 8192;
    if(alloc < sz)
    {
      free(buf);
      E_("failed to allocate buffer");
      return ICNS_ALLOC_ERROR;
    }
    tmp = realloc(buf, alloc);
    if(!tmp)
    {
      free(buf);
      E_("failed to allocate buffer");
      return ICNS_ALLOC_ERROR;
    }
    buf = (uint8_t *)tmp;

    sz += icns->read_fn(buf + sz, alloc - sz, icns->read_priv);
  }
  tmp = realloc(buf, sz ? sz : 1);
  if(tmp)
    buf = (uint8_t *)tmp;

  *dest = buf;
  *size = sz;
  return ICNS_OK;
}

/**
 * Write to the currently open stream from a buffer.
 *
 * @param icns        current state data.
 * @param dest        buffer to write data from.
 * @param count       amount of data to write.
 * @return            `ICNS_OK` on success, otherwise `ICNS_WRITE_ERROR` or
 *                    `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_write_direct(struct icns_data *icns,
 const uint8_t *src, size_t count)
{
  size_t count_out;
  if(!icns->write_fn)
  {
    E_("writer is NULL");
    return ICNS_INTERNAL_ERROR;
  }

  count_out = icns->write_fn(src, count, icns->write_priv);
  icns->bytes_out += count_out;
  if(count_out < count)
  {
    E_("write of size %zu failed", count);
    return ICNS_WRITE_ERROR;
  }
  return ICNS_OK;
}

/**
 * Read a chunk header from the currently open stream.
 *
 * @param icns        current state data.
 * @param dest        header struct to write the header data to.
 * @return            `ICNS_OK` on success, otherwise `ICNS_READ_ERROR` or
 *                    `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_read_chunk_header(struct icns_data *icns,
 struct icns_chunk_header *dest)
{
  uint8_t header[8];
  enum icns_error ret;

  ret = icns_read_direct(icns, header, 8);
  if(ret)
  {
    E_("failed to read chunk header");
    return ret;
  }
  dest->magic = icns_get_u32be(header + 0);
  dest->length = icns_get_u32be(header + 4);
  return ICNS_OK;
}

/**
 * Write a chunk header to the currently open stream.
 *
 * @param icns        current state data.
 * @param src         chunk header to write to file.
 * @return            `ICNS_OK` on success, `ICNS_DATA_ERROR` if the size is
 *                    larger than `UINT32_MAX`, otherwise `ICNS_WRITE_ERROR`
 *                    or `ICNS_INTERNAL_ERROR`.
 */
enum icns_error icns_write_chunk_header(struct icns_data *icns,
 const struct icns_chunk_header *src)
{
  uint8_t header[8];
  enum icns_error ret;

  icns_put_u32be(header + 0, src->magic);
  icns_put_u32be(header + 4, src->length);

  ret = icns_write_direct(icns, header, 8);
  if(ret)
  {
    E_("failed to write chunk header: %08" PRIx32 " %" PRIu32,
     src->magic, src->length);
    return ret;
  }
  return ICNS_OK;
}

/**
 * Get the type of the filesystem object that exists at the provided path.
 *
 * @param path        path to check.
 * @return            `IO_NOTEXIST` if no object exists, `IO_REG` if the
 *                    object is a file, `IO_DIR` if the object is a directory,
 *                    otherwise `IO_OTHER`.
 */
int icns_get_file_type(const char *path)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  return icns_io_get_file_type(path);
#else
  return IO_NOTEXIST;
#endif
}

/**
 * Set the current filesystem directory.
 *
 * @param icns        current state data.
 * @param path        path to change current directory to (can be relative).
 * @return            `ICNS_OK` on success, otherwise `ICNS_FILESYSTEM_ERROR`.
 */
enum icns_error icns_chdir(struct icns_data *icns, const char *path)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  if(icns_io_chdir(path))
  {
    E_("chdir failed: %s", strerror(errno));
    return ICNS_FILESYSTEM_ERROR;
  }
  return ICNS_OK;
#else
  E_("built without filesystem support: chdir(%s)", path);
  return ICNS_FILESYSTEM_ERROR;
#endif
}

/**
 * Get the current filesystem directory.
 *
 * @param icns        current state data.
 * @param dest        buffer to write path to.
 * @param dest_size   size of buffer.
 * @return            `ICNS_OK` on success, otherwise `ICNS_FILESYSTEM_ERROR`.
 */
enum icns_error icns_getcwd(struct icns_data *icns, char *dest, size_t dest_size)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  if(!icns_io_getcwd(dest, dest_size))
  {
    E_("getcwd failed: %s", strerror(errno));
    return ICNS_FILESYSTEM_ERROR;
  }
  return ICNS_OK;
#else
  (void)dest;
  (void)dest_size;
  E_("built without filesystem support: getcwd");
  return ICNS_FILESYSTEM_ERROR;
#endif
}

/**
 * Make a new filesystem directory at the provided path.
 *
 * @param icns        current state data.
 * @param path        path to create directory at.
 * @return            `ICNS_OK` on success, otherwise `ICNS_FILESYSTEM_ERROR`.
 */
enum icns_error icns_mkdir(struct icns_data *icns, const char *path)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  if(icns_io_mkdir(path))
  {
    E_("mkdir failed: %s", strerror(errno));
    return ICNS_FILESYSTEM_ERROR;
  }
  return ICNS_OK;
#else
  E_("built without filesystem support: mkdir(%s)", path);
  return ICNS_FILESYSTEM_ERROR;
#endif
}

/**
 * Unlink a non-directory file at the provided path.
 *
 * @param icns        current state data.
 * @param path        path to file to delete.
 * @return            `ICNS_OK` on success, otherwise `ICNS_FILESYSTEM_ERROR`.
 */
enum icns_error icns_unlink(struct icns_data *icns, const char *path)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  if(icns_io_unlink(path))
  {
    E_("unlink failed: %s", strerror(errno));
    return ICNS_FILESYSTEM_ERROR;
  }
  return ICNS_OK;
#else
  E_("built without filesystem support: unlink(%s)", path);
  return ICNS_FILESYSTEM_ERROR;
#endif
}

/**
 * Read the contents of a directory into a singly linked list in memory.
 *
 * @param icns        current state data.
 * @param dest        the base of the linked list, or NULL if the directory
 *                    has no entries, will be written here on success.
 * @param icns        path to read as a directory.
 * @return            `ICNS_OK` on success, otherwise `ICNS_FILESYSTEM_ERROR`.
 */
enum icns_error icns_read_directory(struct icns_data *icns,
 struct icns_dir_entry **dest, const char *path)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  struct icns_dir_entry *base = NULL;
  struct icns_dir_entry *current;

  icns_dirtype *dir = icns_io_opendir(path);
  if(!dir)
  {
    E_("failed to open dir '%s'", path);
    return ICNS_FILESYSTEM_ERROR;
  }

  while(1)
  {
    struct icns_dir_entry *next = icns_io_readdir(dir);
    if(!next)
      break;
    if(!base)
    {
      base = next;
      current = next;
    }
    else
    {
      current->next = next;
      current = next;
    }
  }
  icns_io_closedir(dir);
  *dest = base;
  return ICNS_OK;
#else
  (void)dest;
  E_("built without filesystem support: read_directory(%s)", path);
  return ICNS_FILESYSTEM_ERROR;
#endif
}

/**
 * Free a directory contents linked list (as provided by `icns_read_directory`).
 *
 * @param entries     the base entry of the linked list to be freed.
 */
void icns_free_directory(struct icns_dir_entry *entries)
{
  while(entries)
  {
    struct icns_dir_entry *next = entries->next;
    free(entries);
    entries = next;
  }
}
