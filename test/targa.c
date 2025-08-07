/* icnscvt
 *
 * Copyright (C) 2024-2025 Alice Rowan <petrifiedrowan@gmail.com>
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
#include "../src/icns_image.h"
#include "../src/icns_io.h"

#include <sys/stat.h>
#include <zlib.h>

const uint8_t test_random_data[] =
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

NOT_NULL
static inline void put_u16le(uint8_t *buf, uint16_t value)
{
  buf[0] = value;
  buf[1] = value >> 8;
}

NOT_NULL
static inline void put_u32le(uint8_t *buf, uint32_t value)
{
  buf[0] = value;
  buf[1] = value >> 8;
  buf[2] = value >> 16;
  buf[3] = value >> 24;
}

NOT_NULL
static inline uint16_t get_u16le(const uint8_t *buf)
{
  return buf[0] | (buf[1] << 8u);
}

NOT_NULL
static inline uint32_t get_u32le(const uint8_t *buf)
{
  return  buf[0] |
          (buf[1] << 8u) |
          (buf[2] << 16u) |
          ((uint32_t)buf[3] << 24u);
}

static size_t test_filelength(FILE *f)
{
#ifdef __linux__
  struct stat st;
  int ret;
  int fd = fileno(f);

  ASSERT(fd >= 0, "couldn't get file descriptor for file");

  memset(&st, 0, sizeof(st)); /* workaround MemorySanitizer bug */
  ret = fstat(fd, &st);
  ASSERT(ret == 0, "coudn't stat file descriptor");
  ASSERT(st.st_size > 0, "zero or negative file size");

  return st.st_size;
#else
  long length;
  long pos = ftell(f);
  int ret;

  ASSERT(pos >= 0, "couldn't ftell file");

  ret = fseek(f, 0, SEEK_END);
  ASSERT(ret == 0, "failed to seek end of file");

  length = ftell(f);
  ASSERT(length > 0, "zero file size or ftell failed");

  ret = fseek(f, pos, SEEK_SET);
  ASSERT(ret == 0, "failed to seek back to original pos");
  return length;
#endif
}

void test_load(struct icns_data *icns, uint8_t **dest, size_t *size,
 const char *filename)
{
  enum icns_error ret;
  uint8_t *buffer;
  size_t sz;

  ret = icns_io_init_read_file(icns, filename);
  check_ok(icns, ret);

  sz = test_filelength(icns->io.ptr.f);

  ret = icns_load_direct(icns, &buffer, sz);
  check_ok(icns, ret);

  icns_io_end(icns);
  *dest = buffer;
  *size = sz;
}

void test_load_compressed(struct icns_data *icns,
 uint8_t **dest, size_t *size, const char *filename)
{
  z_stream zs;
  uint8_t *compressed;
  uint8_t *uncompressed;
  size_t compressed_size;
  size_t uncompressed_size;
  int ret;

  test_load(icns, &compressed, &compressed_size, filename);

  uncompressed_size = get_u32le(compressed + compressed_size - 4);
  ASSERT(uncompressed_size, "uncompressed size should be non-zero");
  uncompressed = (uint8_t *)malloc(uncompressed_size);
  ASSERT(uncompressed, "failed alloc");

  memset(&zs, 0, sizeof(zs));
  zs.avail_out = uncompressed_size;
  zs.next_out = uncompressed;
  zs.avail_in = compressed_size;
  zs.next_in = compressed;

  ret = inflateInit2(&zs, MAX_WBITS | 16);
  ASSERTEQ(ret, Z_OK, "failed inflate init");
  ret = inflate(&zs, Z_FINISH);
  ASSERTEQ(ret, Z_STREAM_END, "failed inflate");
  inflateEnd(&zs);
  free(compressed);

  *dest = uncompressed;
  *size = uncompressed_size;
}

void test_save(struct icns_data *icns,
 const uint8_t *src, size_t size, const char *filename)
{
  enum icns_error ret;

  ret = icns_io_init_write_file(icns, filename);
  check_ok(icns, ret);

  ret = icns_write_direct(icns, src, size);
  check_ok(icns, ret);

  icns_io_end(icns);
}

void test_save_compressed(struct icns_data *icns,
 const uint8_t *uncompressed, size_t uncompressed_size, const char *filename)
{
  z_stream zs;
  uint8_t *compressed;
  size_t compressed_size;
  size_t compressed_bound;
  int ret;

  memset(&zs, 0, sizeof(zs));

  ret = deflateInit2(&zs, Z_BEST_COMPRESSION, Z_DEFLATED,
   MAX_WBITS | 16, 9, Z_DEFAULT_STRATEGY);
  ASSERTEQ(ret, Z_OK, "failed deflate init");

  compressed_bound = deflateBound(&zs, uncompressed_size);
  ASSERT(compressed_bound > 0, "compressed bound should be non-zero");

  compressed = (uint8_t *)malloc(compressed_bound);

  zs.avail_out = compressed_bound;
  zs.next_out = compressed;
  zs.avail_in = uncompressed_size;
  zs.next_in = (uint8_t *)uncompressed; /* zlib not const-correct */

  ret = deflate(&zs, Z_FINISH);
  ASSERTEQ(ret, Z_STREAM_END, "failed deflate");

  compressed_size = compressed_bound - zs.avail_out;
  deflateEnd(&zs);

  test_save(icns, compressed, compressed_size, filename);
  free(compressed);
}

NOT_NULL
static void get_tga_color(struct rgba_color *dest, const uint8_t * RESTRICT src)
{
  dest->r = src[2];
  dest->g = src[1];
  dest->b = src[0];
  dest->a = src[3];
}

NOT_NULL
static void put_tga_color(uint8_t * RESTRICT dest, const struct rgba_color *src)
{
  dest[0] = src->b;
  dest[1] = src->g;
  dest[2] = src->r;
  dest[3] = src->a;
}

/* TGA writer: always RLE, outputs with gzip compression if filename is .gz
 * Don't bother with indexed mode (gzip compression). */
void test_save_tga(struct icns_data *icns,
 const struct rgba_color *pixels, unsigned w, unsigned h, const char *filename)
{
  /* Absolute worst case--file should essentially never be this big. */
  size_t bound_size = w * h * (4 + 1) + 18 + 26;
  size_t size;
  size_t offset;
  size_t x;
  size_t y;
  size_t i;

  const uint32_t *pixels_u32 = (const uint32_t *)pixels;
  uint8_t *out;
  uint8_t *pos;
  ptrdiff_t ext_pos;

  out = (uint8_t *)malloc(bound_size);
  ASSERT(out, "failed to allocate TGA buffer");

  out[0] = 0;
  out[1] = 0;
  out[2] = 10;
  put_u16le(out + 3, 0);
  put_u16le(out + 5, 0);
  out[7] = 0;
  put_u16le(out + 8, 0);
  put_u16le(out + 10, h);
  put_u16le(out + 12, w);
  put_u16le(out + 14, h);
  out[16] = 32;
  out[17] = 0x28;

  pos = out + 18;
  offset = 0;
  for(y = 0; y < h; y++)
  {
    for(x = 0; x < w; )
    {
      size_t num = 1;
      size_t start = offset;
      uint32_t color = pixels_u32[offset++];
      x++;
      while(x < w && num < 128 && pixels_u32[offset] == color)
        offset++, x++, num++;
      if(num > 1)
      {
        *(pos++) = 0x80 | (num - 1);
        put_tga_color(pos, pixels + start);
        pos += 4;
        continue;
      }

      while(x < w && num < 128 &&
       !(x + 1 < w && pixels_u32[offset] == pixels_u32[offset + 1]))
        offset++, x++, num++;

      *(pos++) = num - 1;
      for(i = 0; i < num; i++)
      {
        put_tga_color(pos, pixels + start + i);
        pos += 4;
      }
    }
  }
  put_u32le(pos, 0);
  put_u32le(pos + 4, 0);
  memcpy(pos + 8, "TRUEVISION-XFILE.", 18);
  size = (pos - out) + 26;

  ext_pos = strlen(filename) - 3;
  if(ext_pos >= 0 && !strcmp(filename + ext_pos, ".gz"))
    test_save_compressed(icns, out, size, filename);
  else
    test_save(icns, out, size, filename);

  free(out);
}

/* TGA loader: only returns raw image data.
 * Currently only loads the subset of TGAs that are saved by save_tga. */
void test_load_tga(struct icns_data *icns, struct rgba_color **dest,
 unsigned *dest_w, unsigned *dest_h, const char *filename)
{
  struct rgba_color *pixels;
  uint8_t *in;
  uint8_t *pos;
  uint8_t *end;
  size_t offset;
  size_t in_size;
  size_t total_px;
  size_t i;
  uint16_t w;
  uint16_t h;

  ptrdiff_t ext_pos = strlen(filename) - 3;
  if(ext_pos >= 0 && !strcmp(filename + ext_pos, ".gz"))
    test_load_compressed(icns, &in, &in_size, filename);
  else
    test_load(icns, &in, &in_size, filename);

  ASSERT(in_size > 44, "'%s': minimum size check failed: %zu", filename, in_size);
  ASSERTMEM(in + in_size - 18, "TRUEVISION-XFILE.", 18,
    "'%s': magic check failed", filename);

  w = get_u16le(in + 12);
  h = get_u16le(in + 14);

  ASSERTEQ(in[1], 0, "'%s': indexed: %d", filename, in[1]);
  ASSERTEQ(in[2], 10, "'%s': not truecolor RLE: %d", filename, in[2]);
  ASSERT(w != 0, "'%s': bad width: %d", filename, w);
  ASSERT(h != 0, "'%s': bad height: %d", filename, h);
  ASSERTEQ(in[16], 32, "'%s': not 32bpp: %d", filename, in[16]);
  ASSERTEQ(in[17] & 0xf0, 0x20, "'%s': not top-left: %d", filename, in[17]);
  ASSERTEQ(in[17] & 0x0f, 8, "'%s': not 8 alphabits: %d", filename, in[17]);

  total_px = (size_t)w * h;
  pixels = (struct rgba_color *)calloc(total_px, sizeof(struct rgba_color));
  ASSERT(pixels, "failed to alloc pixel array");

  end = in + in_size - 26;
  pos = in + 18;
  offset = 0;
  while(pos < end && offset < total_px)
  {
    size_t num = (*pos & 0x7f) + 1;
    ASSERT(num < total_px && offset <= total_px - num, "'%s': bad RLE", filename);
    if(*(pos++) & 0x80)
    {
      struct rgba_color color;
      ASSERT(4 <= end - pos, "'%s': bad RLE", filename);
      get_tga_color(&color, pos);
      pos += 4;
      for(i = 0; i < num; i++)
        pixels[offset++] = color;
    }
    else
    {
      ASSERT((ptrdiff_t)num * 4 <= end - pos, "'%s': bad unpacked block", filename);
      for(i = 0; i < num; i++)
      {
        get_tga_color(pixels + offset, pos);
        pos += 4;
        offset++;
      }
    }
  }

  *dest = pixels;
  *dest_w = w;
  *dest_h = h;
  free(in);
}



static struct loaded_file loaded_files[256];
static size_t num_loaded;
#define max_loaded (sizeof(loaded_files) / sizeof(loaded_files[0]))

NOT_NULL
static struct loaded_file *get_loaded_file(const char *which)
{
  size_t i;
  for(i = 0; i < num_loaded; i++)
    if(!strcmp(loaded_files[i].which, which))
      return &loaded_files[i];

  ASSERT(num_loaded < max_loaded, "exceeded max loaded bound!");
  loaded_files[num_loaded].which = strdup(which);
  return &loaded_files[num_loaded++];
}

/* Load caching--only load individual files from a unique path once.
 * The filename will be backed up to the internal cache structure. */
const struct loaded_file *test_load_cached(struct icns_data *icns,
 const char *filename)
{
  struct loaded_file *ret = get_loaded_file(filename);
  if(!ret->data)
    test_load(icns, &ret->data, &ret->data_size, filename);

  return ret;
}

const struct loaded_file *test_load_tga_cached(struct icns_data *icns,
 size_t width, size_t height, const char *filename)
{
  struct loaded_file *ret = get_loaded_file(filename);
  if(!ret->pixels)
    test_load_tga(icns, &ret->pixels, &ret->w, &ret->h, filename);

  ASSERTEQ(ret->w, width,
    "'%s': %u != %zu", filename, ret->w, width);
  ASSERTEQ(ret->h, height,
    "'%s': %u != %zu", filename, ret->h, height);
  return ret;
}

void test_load_cached_cleanup(void)
{
  size_t i;
  for(i = 0; i < num_loaded; i++)
  {
    free(loaded_files[i].which);
    free(loaded_files[i].data);
    free(loaded_files[i].pixels);
  }
  num_loaded = 0;
}
