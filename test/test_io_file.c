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
  memset(&icns, 0, sizeof(icns));
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
  memset(&icns, 0, sizeof(icns));
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
  memset(&icns, 0, sizeof(icns));
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
  memset(&icns, 0, sizeof(icns));
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
#ifndef ICNSCVT_NO_FILESYSTEM
  enum icns_error ret;
  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
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
#endif
}

UNITTEST(io_init_write_file)
{
#ifndef ICNSCVT_NO_FILESYSTEM
  enum icns_error ret;
  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
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
#endif
}

/***** Generic IO *****/

/* Randomly generated binary data. */
static const uint8_t test_data[] =
{
  0x0A,0xFF,0xE9,0x06,0x94,0xEE,0xC2,0xCE,
  0x87,0x72,0x5C,0xCE,0x5A,0xBD,0x72,0xE8,
  0x3A,0xD3,0x47,0x5F,0x2B,0x3F,0x8E,0x31,
  0x04,0x04,0x87,0xA7,0xBB,0xD6,0x6C,0x2A,
  0xE3,0x8A,0x0B,0x58,0x73,0xCC,0xB4,0x57,
  0xC9,0x10,0x55,0x69,0xEF,0xCB,0xE4,0xC1,
  0x1C,0xE2,0xFD,0x96,0x7E,0x62,0xCD,0x75,
  0x36,0x73,0x2E,0xDB,0xA5,0x87,0x13,0x03,
  0xE6,0xD7,0x27,0xFD,0x9B,0x10,0xB1,0x0D,
  0xED,0x49,0xBB,0x01,0xCE,0x39,0x73,0x84,
  0x81,0xA7,0xD1,0xB3,0x5D,0x21,0x7A,0xAD,
  0xCE,0xF1,0xE5,0x20,0x9C,0x4D,0xBC,0x1A,
  0x70,0xCE,0x85,0x1C,0x94,0x24,0x81,0x71,
  0xFA,0x07,0xD4,0xBD,0x80,0x70,0xE7,0xD9,
  0x72,0x0A,0x0B,0xDE,0x50,0xE3,0x5F,0x80,
  0xD2,0x68,0xF3,0x9E,0x2A,0x90,0x2D,0x16,
  0x89,0x57,0x6C,0xDE,0xE4,0x6E,0xEC,0x51,
  0xF8,0x31,0xEA,0xC6,0x8C,0xD9,0x08,0x67,
  0xF6,0xF3,0xF2,0x41,0xE0,0x11,0x42,0x97,
  0x4C,0xBF,0xA1,0x7C,0xD6,0xB8,0x2F,0xA1,
  0x39,0x5A,0x26,0x6B,0x15,0x58,0xBA,0x48,
  0xF0,0xAF,0x43,0x44,0x7A,0xDB,0x9D,0xD7,
  0x15,0x4A,0xCF,0x01,0x94,0x11,0xEC,0x99,
  0x43,0xDE,0x37,0xE3,0x28,0x2D,0x8A,0x60,
  0x89,0xBF,0xF9,0xEA,0xAF,0x48,0xB2,0x07,
  0xE9,0x69,0x27,0x5E,0xD3,0xDD,0x70,0xD0,
  0xD7,0xF7,0xEA,0x49,0xF5,0x4C,0x25,0x2F,
  0xC0,0xAD,0xFD,0xFA,0xA9,0x58,0x06,0xFD,
  0x80,0x6E,0x2E,0x83,0x38,0xA8,0x9D,0x1E,
  0xEB,0x46,0xE0,0x3C,0x1D,0x49,0x47,0xFB,
  0x45,0xE2,0x8B,0x3F,0x8A,0x2A,0xB4,0x01,
  0xCA,0x13,0x3A,0xEA,0xE0,0x9F,0x6B,0x00,
};

UNITTEST(io_icns_read_direct)
{
  enum icns_error ret;
  struct test_read_data data = { test_data, 0, sizeof(test_data) };
  uint8_t buf[sizeof(test_data)];

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
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

  ret = icns_read_direct(&icns, buf, sizeof(test_data));
  check_ok(&icns, ret);
  ASSERTMEM(buf, test_data, sizeof(test_data), "");
  ASSERTEQ(icns.bytes_in, sizeof(test_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_data));

  /* Can't read past end. */
  ret = icns_read_direct(&icns, buf, 1);
  check_error(&icns, ret, ICNS_READ_ERROR);
  ASSERTEQ(icns.bytes_in, sizeof(test_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_data));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_load_direct)
{
  enum icns_error ret;
  struct test_read_data data = { test_data, 0, sizeof(test_data) };
  uint8_t *buf = NULL;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  buf = NULL;
  ret = icns_load_direct(&icns, &buf, 1);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);
  ASSERT(!buf, "reurn buffer should still be null");

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

  ret = icns_load_direct(&icns, &buf, sizeof(test_data));
  check_ok(&icns, ret);
  ASSERT(buf, "didn't set return buffer");
  ASSERTMEM(buf, test_data, sizeof(test_data), "");
  ASSERTEQ(icns.bytes_in, sizeof(test_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_data));
  free(buf);

  /* Can't read past end. */
  buf = NULL;
  ret = icns_load_direct(&icns, &buf, 1);
  check_error(&icns, ret, ICNS_READ_ERROR);
  ASSERT(!buf, "return buffer should still be null");
  ASSERTEQ(icns.bytes_in, sizeof(test_data),
   "%zu != %zu", icns.bytes_in, sizeof(test_data));

  icns_io_end(&icns);
  check_init(&icns);
}

UNITTEST(io_icns_write_direct)
{
  enum icns_error ret;
  uint8_t buf[sizeof(test_data)];
  struct test_write_data data = { buf, 0, sizeof(buf) };

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  ret = icns_write_direct(&icns, test_data, 1);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_write(&icns, &data, test_write_func);
  check_ok(&icns, ret);
  check_write(&icns, IO_CALLBACK, &data, test_write_func);

  /* Write of zero should succeed and not advance cursor.
   * This might be different for user callbacks. */
  ret = icns_write_direct(&icns, test_data, 0);
  check_ok(&icns, ret);
  ASSERTEQ(icns.bytes_out, 0, "%zu", icns.bytes_out);

  ret = icns_write_direct(&icns, test_data, sizeof(test_data));
  check_ok(&icns, ret);
  ASSERTMEM(buf, test_data, sizeof(test_data), "");
  ASSERTEQ(icns.bytes_out, sizeof(test_data),
   "%zu != %zu", icns.bytes_out, sizeof(test_data));

  /* Can't write past end. */
  ret = icns_write_direct(&icns, test_data, 1);
  check_error(&icns, ret, ICNS_WRITE_ERROR);
  ASSERTEQ(icns.bytes_out, sizeof(test_data),
   "%zu != %zu", icns.bytes_out, sizeof(test_data));

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
  memset(&icns, 0, sizeof(icns));
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
  size_t i;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  /* Can't use on uninitialized stream. */
  ret = icns_write_chunk_header(&icns, 1, 2);
  check_error(&icns, ret, ICNS_INTERNAL_ERROR);

  ret = icns_io_init_write(&icns, &data, test_write_func);
  check_ok(&icns, ret);
  check_write(&icns, IO_CALLBACK, &data, test_write_func);

  for(i = 0; i < num_test_chunks; i++)
  {
    struct icns_chunk_header chunk = test_chunk_unpacked[i];
    ret = icns_write_chunk_header(&icns, chunk.magic, chunk.length);
    check_ok(&icns, ret);
    ASSERTEQ(icns.bytes_out, (i + 1) * 8, "i = %zu", i);
  }
  ASSERTMEM(test_chunk_raw, buf, sizeof(buf), "");

  /* Can't write past end. */
  ret = icns_write_chunk_header(&icns, 123, 456);
  check_error(&icns, ret, ICNS_WRITE_ERROR);
  ASSERTMEM(test_chunk_raw, buf, sizeof(buf), "");
  ASSERTEQ(icns.bytes_out, sizeof(buf),
   "%zu != %zu", icns.bytes_out, sizeof(buf));

#if SIZE_MAX > UINT32_MAX
  /* 64-bit: also should not be able to write with data size > UINT32_MAX */
  data.pos = 0;
  icns.bytes_out = 0;
  ret = icns_write_chunk_header(&icns, 12345, SIZE_MAX);
  check_error(&icns, ret, ICNS_DATA_ERROR);
  ASSERTEQ(data.pos, 0, "%zu", data.pos);
  ASSERTEQ(icns.bytes_out, 0, "%zu", icns.bytes_out);
#endif

  icns_io_end(&icns);
  check_init(&icns);
}
