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

#include <zlib.h>

/* Validate internal test functions for TGA images and compressed files.
 * The TGA loader only accepts one variant of TGA (32-bit truecolor RLE with
 * mandatory footer), so the test file requires extra ImageMagick options:
 *
 * magick -background none -density 12 ../../../assets/icnscvt.svg
 *   -define tga:write-footer=true -compress rle test.tga
 *
 * magick test.tga rgba:test.raw
 */

static const uint32_t text_crc = 0xe3017c40ul;
static const uint32_t tga_crc = 0x0ca71c97ul;
static const uint32_t raw_crc = 0x9c238a75ul;

UNITTEST(test_load)
{
  uint8_t *data;
  size_t data_size;
  uint32_t crc;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &data, &data_size, DATA_DIR "/internal/text.txt");

  crc = crc32(0u, data, data_size);
  ASSERTEQ(crc, text_crc, "%08" PRIx32 " != %08" PRIx32, crc, text_crc);
  free(data);

  test_load(&icns, &data, &data_size, DATA_DIR "/internal/test.tga");

  crc = crc32(0u, data, data_size);
  ASSERTEQ(crc, tga_crc, "%08" PRIx32 " != %08" PRIx32, crc, tga_crc);
  free(data);
}

UNITTEST(test_load_compressed)
{
  uint8_t *orig;
  size_t orig_size;
  uint8_t *gzip;
  size_t gzip_size;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/text.txt");
  test_load_compressed(&icns, &gzip, &gzip_size, DATA_DIR "/internal/text.txt.gz");
  ASSERTEQ(orig_size, gzip_size, "size mismatch");
  ASSERTMEM(orig, gzip, gzip_size, "data mismatch");
  free(orig);
  free(gzip);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/test.tga");
  test_load_compressed(&icns, &gzip, &gzip_size, DATA_DIR "/internal/test.tga.gz");
  ASSERTEQ(orig_size, gzip_size, "size mismatch");
  ASSERTMEM(orig, gzip, gzip_size, "data mismatch");
  free(orig);
  free(gzip);
}

UNITTEST(test_save)
{
  uint8_t *orig;
  size_t orig_size;
  uint8_t *save;
  size_t save_size;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/text.txt");
  test_save(&icns, orig, orig_size, TEMP_DIR "/text.txt");
  test_load(&icns, &save, &save_size, TEMP_DIR "/text.txt");
  ASSERTEQ(orig_size, save_size, "size mismatch");
  ASSERTMEM(orig, save, save_size, "data mismatch");
  free(orig);
  free(save);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/test.tga");
  test_save(&icns, orig, orig_size, TEMP_DIR "/test.tga");
  test_load(&icns, &save, &save_size, TEMP_DIR "/test.tga");
  ASSERTEQ(orig_size, save_size, "size mismatch");
  ASSERTMEM(orig, save, save_size, "data mismatch");
  free(orig);
  free(save);
}

UNITTEST(test_save_compressed)
{
  uint8_t *orig;
  size_t orig_size;
  uint8_t *save;
  size_t save_size;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/text.txt");
  test_save_compressed(&icns, orig, orig_size, TEMP_DIR "/text.txt.gz");
  test_load_compressed(&icns, &save, &save_size, TEMP_DIR "/text.txt.gz");
  ASSERTEQ(orig_size, save_size, "size mismatch");
  ASSERTMEM(orig, save, save_size, "data mismatch");
  free(orig);
  free(save);

  test_load(&icns, &orig, &orig_size, DATA_DIR "/internal/test.tga");
  test_save_compressed(&icns, orig, orig_size, TEMP_DIR "/test.tga.gz");
  test_load_compressed(&icns, &save, &save_size, TEMP_DIR "/test.tga.gz");
  ASSERTEQ(orig_size, save_size, "size mismatch");
  ASSERTMEM(orig, save, save_size, "data mismatch");
  free(orig);
  free(save);
}

UNITTEST(test_load_tga)
{
  uint8_t *tmp;
  struct rgba_color *raw;
  struct rgba_color *tga;
  struct rgba_color *tga_gz;
  size_t raw_size;
  unsigned w;
  unsigned h;
  unsigned w_gz;
  unsigned h_gz;
  uint32_t crc;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &tmp, &raw_size, DATA_DIR "/internal/test.raw");
  crc = crc32(0u, tmp, raw_size);
  ASSERTEQ(crc, raw_crc, "%08" PRIx32 " != %08" PRIx32, crc, raw_crc);

  raw = (struct rgba_color *)tmp;

  test_load_tga(&icns, &tga, &w, &h, DATA_DIR "/internal/test.tga");
  ASSERTEQ(w * h * 4, raw_size, "loaded tga size doesn't match raw");
  ASSERTMEM(raw, tga, raw_size, "tga pixel data mismatch");

  test_load_tga(&icns, &tga_gz, &w_gz, &h_gz, DATA_DIR "/internal/test.tga.gz");
  ASSERTEQ(w_gz * h_gz * 4, raw_size, "loaded tga.gz size doesn't match raw");
  ASSERTMEM(raw, tga_gz, raw_size, "tga.gz pixel data mismatch");

  free(tmp);
  free(tga);
  free(tga_gz);
}

UNITTEST(test_save_tga)
{
  uint8_t *tmp;
  struct rgba_color *raw;
  struct rgba_color *tga;
  struct rgba_color *tga_gz;
  size_t raw_size;
  unsigned w;
  unsigned h;
  unsigned w_gz;
  unsigned h_gz;
  uint32_t crc;

  struct icns_data icns;
  memset(&icns, 0, sizeof(icns));
  check_init(&icns);

  test_load(&icns, &tmp, &raw_size, DATA_DIR "/internal/test.raw");
  crc = crc32(0u, tmp, raw_size);
  ASSERTEQ(crc, raw_crc, "%08" PRIx32 " != %08" PRIx32, crc, raw_crc);

  raw = (struct rgba_color *)tmp;

  test_save_tga(&icns, raw, 16, 16, TEMP_DIR "/test.tga");
  test_load_tga(&icns, &tga, &w, &h, TEMP_DIR "/test.tga");
  ASSERTEQ(w * h * 4, raw_size, "loaded tga size doesn't match raw");
  ASSERTMEM(raw, tga, raw_size, "tga pixel data mismatch");

  test_save_tga(&icns, raw, 16, 16, TEMP_DIR "/test.tga.gz");
  test_load_tga(&icns, &tga_gz, &w_gz, &h_gz, TEMP_DIR "/test.tga.gz");
  ASSERTEQ(w_gz * h_gz * 4, raw_size, "loaded tga.gz size doesn't match raw");
  ASSERTMEM(raw, tga_gz, raw_size, "tga.gz pixel data mismatch");

  free(tmp);
  free(tga);
  free(tga_gz);
}
