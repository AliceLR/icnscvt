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
#include "../src/icns_io.h"

/***** Open callbacks/memory/file for read/write. *****/

struct test_read_data
{
  const uint8_t *base;
  size_t pos;
  size_t size;
};

struct test_write_data
{
  uint8_t *base;
  size_t pos;
  size_t size;
};

static size_t test_read_func(void *dest, size_t size, void *priv)
{
  struct test_read_data *data = (struct test_read_data *)priv;
  if(data->pos >= data->size)
    return 0;
  if(size > data->size - data->pos)
    size = data->size - data->pos;

  memcpy(dest, data->base + data->pos, size);
  data->pos += size;
  return size;
}

static size_t test_write_func(const void *src, size_t size, void *priv)
{
  struct test_write_data *data = (struct test_write_data *)priv;
  if(data->pos >= data->size)
    return 0;
  if(size > data->size - data->pos)
    size = data->size - data->pos;

  memcpy(data->base + data->pos, src, size);
  data->pos += size;
  return size;
}

static void check_read(struct icns_data *icns, int type, void *priv,
 size_t (*fn)(void *, size_t, void *))
{
  ASSERTEQ((int)icns->io.type, type, "%d != %d", (int)icns->io.type, type);
  if(priv)
    ASSERTEQ(icns->read_priv, priv, "");
  if(fn)
    ASSERTEQ(icns->read_fn, fn, "");
  ASSERTEQ(icns->write_priv, NULL, "");
  ASSERTEQ(icns->write_fn, NULL, "");
}

static void check_write(struct icns_data *icns, int type, void *priv,
 size_t (*fn)(const void *, size_t, void *))
{
  ASSERTEQ((int)icns->io.type, type, "%d != %d", icns->io.type, type);
  ASSERTEQ(icns->read_priv, NULL, "");
  ASSERTEQ(icns->read_fn, NULL, "");
  if(priv)
    ASSERTEQ(icns->write_priv, priv, "");
  if(fn)
    ASSERTEQ(icns->write_fn, fn, "");
}

static void check_buffer(struct icns_data *icns, const void *buffer,
 size_t pos, size_t size)
{
  ASSERTEQ(icns->io.ptr.dest, buffer, "");
  ASSERTEQ(icns->io.pos, pos, "%zu != %zu", icns->io.pos, pos);
  ASSERTEQ(icns->io.size, size, "%zu != %zu", icns->io.size, size);
}

UNITTEST(io_init_read)
{
  enum icns_error ret;
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_ok(&icns, ret);
  check_read(&icns, IO_CALLBACK, &icns, test_read_func);

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_CALLBACK, &icns, test_read_func);

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_CALLBACK, &icns, test_read_func);

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_init_write)
{
  enum icns_error ret;
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_ok(&icns, ret);
  check_write(&icns, IO_CALLBACK, &icns, test_write_func);

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_CALLBACK, &icns, test_write_func);

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_CALLBACK, &icns, test_write_func);

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_init_read_memory)
{
  enum icns_error ret;
  uint8_t buffer[64];
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  memset(buffer, 0, sizeof(buffer));

  ret = icns_io_init_read_memory(&icns, buffer, sizeof(buffer));
  check_ok(&icns, ret);
  check_read(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  ret = icns_io_init_read_memory(&icns, buffer, sizeof(buffer));
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_init_write_memory)
{
  enum icns_error ret;
  uint8_t buffer[64];
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  ret = icns_io_init_write_memory(&icns, buffer, sizeof(buffer));
  check_ok(&icns, ret);
  check_write(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  ret = icns_io_init_write_memory(&icns, buffer, sizeof(buffer));
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_MEMORY, NULL, NULL);
  check_buffer(&icns, buffer, 0, sizeof(buffer));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_init_read_file)
{
  enum icns_error ret;
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Nonexistent file should leave things in init mode. */
  ret = icns_io_init_read_file(&icns, DATA_DIR "/dirent/dfsjksdjfds");
  check_error(&icns, ret, ICNS_READ_OPEN_ERROR);
  check_init(&icns);

  ret = icns_io_init_read_file(&icns, DATA_DIR "/dirent/a_file");
  check_ok(&icns, ret);
  check_read(&icns, IO_FILE, NULL, NULL);

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_FILE, NULL, NULL);

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_FILE, NULL, NULL);

  ret = icns_io_init_read_file(&icns, DATA_DIR "/dirent/a_file");
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_read(&icns, IO_FILE, NULL, NULL);

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_init_write_file)
{
  enum icns_error ret;
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Unopenable path should leave things in init mode. */
  ret = icns_io_init_write_file(&icns, TEMP_DIR "////fhdkj/sdhjfdsfd///fjsds");
  check_error(&icns, ret, ICNS_WRITE_OPEN_ERROR);
  check_init(&icns);

  ret = icns_io_init_write_file(&icns, TEMP_DIR "/init_write_file");
  check_ok(&icns, ret);
  check_write(&icns, IO_FILE, NULL, NULL);

  /* Can't init IO over an open IO. */
  ret = icns_io_init_read(&icns, &icns, test_read_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_FILE, NULL, NULL);

  ret = icns_io_init_write(&icns, &icns, test_write_func);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_FILE, NULL, NULL);

  ret = icns_io_init_write_file(&icns, TEMP_DIR "/init_write_file");
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  check_write(&icns, IO_FILE, NULL, NULL);

  icns_io_end(&icns);
  check_init(&icns);
}

/***** Generic IO *****/

UNITTEST(io_icns_read_direct)
{
  enum icns_error ret;
  struct test_read_data data = { test_random_data, 0, sizeof(test_random_data) };
  uint8_t buf[sizeof(test_random_data)];

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  ret = icns_read_direct(&icns, buf, 1);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_read(&icns, &data, test_read_func);
  check_ok(&icns, ret);
  check_read(&icns, IO_CALLBACK, &data, test_read_func);

  /* Read of zero should succeed and not advance cursor.
   * This might be different for user callbacks. */
  ret = icns_read_direct(&icns, buf, 0);
  check_ok(&icns, ret);
  ASSERTEQ(icns.bytes_in, 0, "%zu", icns.bytes_in);

  ret = icns_read_direct(&icns, buf, sizeof(test_random_data));
  check_ok(&icns, ret);
  ASSERTMEM(buf, test_random_data, sizeof(test_random_data), "");
  ASSERTEQ(icns.bytes_in, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_random_data));

  /* Can't read past end. */
  ret = icns_read_direct(&icns, buf, 1);
  check_error(&icns, ret, ICNS_READ_ERROR);
  ASSERTEQ(icns.bytes_in, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_random_data));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_load_direct)
{
  enum icns_error ret;
  struct test_read_data data = { test_random_data, 0, sizeof(test_random_data) };
  uint8_t *buf = NULL;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  buf = NULL;
  ret = icns_load_direct(&icns, &buf, 1);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  ASSERT(!buf, "return buffer should still be null");

  ret = icns_io_init_read(&icns, &data, test_read_func);
  check_ok(&icns, ret);
  check_read(&icns, IO_CALLBACK, &data, test_read_func);

  /* Read of zero should succeed, allocate a buffer of size 1,
   * and not advance cursor. This might be different for user callbacks. */
  ret = icns_load_direct(&icns, &buf, 0);
  check_ok(&icns, ret);
  ASSERT(buf, "didn't set return buffer");
  free(buf);
  ASSERTEQ(icns.bytes_in, 0, "%zu", icns.bytes_in);

  ret = icns_load_direct(&icns, &buf, sizeof(test_random_data));
  check_ok(&icns, ret);
  ASSERT(buf, "didn't set return buffer");
  ASSERTMEM(buf, test_random_data, sizeof(test_random_data), "");
  ASSERTEQ(icns.bytes_in, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_random_data));
  free(buf);

  /* Can't read past end. */
  buf = NULL;
  ret = icns_load_direct(&icns, &buf, 1);
  check_error(&icns, ret, ICNS_READ_ERROR);
  ASSERT(!buf, "return buffer should still be null");
  ASSERTEQ(icns.bytes_in, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_random_data));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_load_direct_auto)
{
  enum icns_error ret;
  struct test_read_data data = { test_random_data, 0, sizeof(test_random_data) };
  uint8_t *buf = NULL;
  size_t sz = 0;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  buf = NULL;
  ret = icns_load_direct_auto(&icns, &buf, &sz);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  ASSERT(!buf, "return buffer should still be null");
  ASSERTEQ(sz, 0, "return buffer size should still be 0");

  ret = icns_io_init_read(&icns, &data, test_read_func);
  check_ok(&icns, ret);
  check_read(&icns, IO_CALLBACK, &data, test_read_func);

  /* Stream of size zero should return an allocation and size 0. */
  data.size = 0;
  ret = icns_load_direct_auto(&icns, &buf, &sz);
  check_ok(&icns, ret);
  ASSERT(buf, "return buffer should not be null");
  ASSERTEQ(sz, 0, "return buffer size should be 0");
  free(buf);

  /* Any other value should consume the entire stream. */
  data.pos = 0;
  data.size = sizeof(test_random_data);
  ret = icns_load_direct_auto(&icns, &buf, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, sizeof(test_random_data), "");
  ASSERTMEM(buf, test_random_data, sz, "");
  free(buf);

  data.pos = 127;
  ret = icns_load_direct_auto(&icns, &buf, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, 129, "");
  ASSERTMEM(buf, test_random_data + 127, 129, "");
  free(buf);

  data.pos = 27;
  data.size = 171;
  ret = icns_load_direct_auto(&icns, &buf, &sz);
  check_ok(&icns, ret);
  ASSERTEQ(sz, 144, "");
  ASSERTMEM(buf, test_random_data + 27, 144, "");
  free(buf);

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_write_direct)
{
  enum icns_error ret;
  uint8_t buf[sizeof(test_random_data)];
  struct test_write_data data = { buf, 0, sizeof(buf) };

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  ret = icns_write_direct(&icns, test_random_data, 1);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_write(&icns, &data, test_write_func);
  check_ok(&icns, ret);
  check_write(&icns, IO_CALLBACK, &data, test_write_func);

  /* Write of zero should succeed and not advance cursor.
   * This might be different for user callbacks. */
  ret = icns_write_direct(&icns, test_random_data, 0);
  check_ok(&icns, ret);
  ASSERTEQ(icns.bytes_out, 0, "%zu", icns.bytes_out);

  ret = icns_write_direct(&icns, test_random_data, sizeof(test_random_data));
  check_ok(&icns, ret);
  ASSERTMEM(buf, test_random_data, sizeof(test_random_data), "");
  ASSERTEQ(icns.bytes_out, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_out, sizeof(test_random_data));

  /* Can't write past end. */
  ret = icns_write_direct(&icns, test_random_data, 1);
  check_error(&icns, ret, ICNS_WRITE_ERROR);
  ASSERTEQ(icns.bytes_out, sizeof(test_random_data),
   "%zu != %zu", icns.bytes_out, sizeof(test_random_data));

  icns_io_end(&icns);
  check_init(&icns);
}

static const uint8_t test_chunk_raw[16] =
  "it32\x01\x02\x03\x04"
  "\x89PNG\r\n\x1a\n";

static const struct icns_chunk_header test_chunk_unpacked[] =
{
  { MAGIC('i','t','3','2'), 0x01020304 },
  { MAGIC(0x89,'P','N','G'), 0x0d0a1a0a },
};
static const size_t num_test_chunks =
 sizeof(test_chunk_unpacked) / sizeof(test_chunk_unpacked[0]);

UNITTEST(io_icns_read_chunk_header)
{
  enum icns_error ret;
  struct icns_chunk_header buf;
  struct test_read_data data = { test_chunk_raw, 0, sizeof(test_chunk_raw) };
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  ret = icns_read_chunk_header(&icns, &buf);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_read(&icns, &data, test_read_func);
  check_ok(&icns, ret);
  check_read(&icns, IO_CALLBACK, &data, test_read_func);

  for(i = 0; i < num_test_chunks; i++)
  {
    struct icns_chunk_header expected = test_chunk_unpacked[i];
    ret = icns_read_chunk_header(&icns, &buf);
    check_ok(&icns, ret);
    ASSERTEQ(icns.bytes_in, (i + 1) * 8, "i = %zu", i);

    ASSERTEQ(buf.magic, expected.magic,
     "%" PRIx32 " != %" PRIx32, buf.magic, expected.magic);
    ASSERTEQ(buf.length, expected.length,
     "%" PRIx32 " != %" PRIx32, buf.magic, expected.magic);
  }

  /* Can't read past end. */
  ret = icns_read_chunk_header(&icns, &buf);
  check_error(&icns, ret, ICNS_READ_ERROR);

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_write_chunk_header)
{
  enum icns_error ret;
  uint8_t buf[sizeof(test_chunk_raw)];
  struct test_write_data data = { buf, 0, sizeof(buf) };
  struct icns_chunk_header c;
  size_t i;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  c.magic = 1;
  c.length = 2;
  ret = icns_write_chunk_header(&icns, &c);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_write(&icns, &data, test_write_func);
  check_ok(&icns, ret);
  check_write(&icns, IO_CALLBACK, &data, test_write_func);

  for(i = 0; i < num_test_chunks; i++)
  {
    ret = icns_write_chunk_header(&icns, &test_chunk_unpacked[i]);
    check_ok(&icns, ret);
    ASSERTEQ(icns.bytes_out, (i + 1) * 8, "i = %zu", i);
  }
  ASSERTMEM(test_chunk_raw, buf, sizeof(buf), "");

  /* Can't write past end. */
  c.magic = 123;
  c.length = 456;
  ret = icns_write_chunk_header(&icns, &c);
  check_error(&icns, ret, ICNS_WRITE_ERROR);
  ASSERTMEM(test_chunk_raw, buf, sizeof(buf), "");
  ASSERTEQ(icns.bytes_out, sizeof(buf),
   "%zu != %zu", icns.bytes_out, sizeof(buf));

  icns_io_end(&icns);
  check_init(&icns);
}
