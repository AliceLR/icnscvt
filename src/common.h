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

#ifndef ICNSCVT_COMMON_H
#define ICNSCVT_COMMON_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// #include "../include/libicnscvt.h"

#ifdef __cplusplus
#define ICNS_BEGIN_DECLS extern "C" {
#define ICNS_END_DECLS }
#else
#define ICNS_BEGIN_DECLS
#define ICNS_END_DECLS
#endif

#ifndef RESTRICT
#define RESTRICT restrict
#endif
#define NOT_NULL            __attribute__((nonnull))
#define NOT_NULL_1(a)       __attribute__((nonnull((a))))
#define NOT_NULL_2(a,b)     __attribute__((nonnull((a),(b))))
#define NOT_NULL_3(a,b,c)   __attribute__((nonnull((a),(b),(c))))
#define NOT_NULL_4(a,b,c,d) __attribute__((nonnull((a),(b),(c),(d))))

ICNS_BEGIN_DECLS

#define MAGIC(a,b,c,d) ((((uint32_t)a) << 24u) | ((b) << 16) | ((c) << 8) | (d))

enum icns_error
{
  ICNS_OK,
  // Acceptable in some situations.
  ICNS_NO_IMAGE,
  ICNS_IMAGE_EXISTS_FOR_FORMAT,
  // Always an error.
  ICNS_INTERNAL_ERROR,
  ICNS_READ_OPEN_ERROR,
  ICNS_READ_ERROR,
  ICNS_WRITE_OPEN_ERROR,
  ICNS_WRITE_ERROR,
  ICNS_FILESYSTEM_ERROR,
  ICNS_ALLOC_ERROR,
  ICNS_DATA_ERROR,
  ICNS_INVALID_DIMENSIONS,
  ICNS_UNKNOWN_CHUNK,
  ICNS_UNIMPLEMENTED_FORMAT,
  ICNS_PNG_INIT_ERROR,
  ICNS_PNG_READ_ERROR,
  ICNS_PNG_WRITE_ERROR,
  ICNS_PNG_NOT_A_PNG,
  ICNS_PNG_NOT_1_BIT_COLOR,
  ICNS_PNG_NOT_1_BIT_COLOR_MASK,
  ICNS_PNG_NOT_IN_PALETTE,
  ICNS_JP2_NOT_A_JP2,
  ICNS_JP2_DATA_ERROR,
};

enum icns_state
{
  ICNS_STATE_INIT,
  ICNS_STATE_LOADING,
  ICNS_STATE_PREPARED_FOR_EXTERNAL,
  ICNS_STATE_PREPARED_FOR_ICNS,
  ICNS_STATE_HOLDING_ICNS_SIZE,
  ICNS_STATE_HOLDING_ICONSET_SIZE,
  ICNS_STATE_HOLDING_EXTERNAL_SIZE,
};

enum icns_target_type
{
  ICNS_TARGET_EXTERNAL,
  ICNS_TARGET_ICONSET,
  ICNS_TARGET_ICNS
};

struct icns_data;
struct icns_format;
struct icns_image;

struct icns_chunk_header
{
  uint32_t magic;
  uint32_t length;
};

#define ICNS_MAX_TOC 256
struct icns_image_set
{
  struct icns_image *head;
  struct icns_image *tail;
  unsigned num_images;

  struct icns_chunk_header toc[ICNS_MAX_TOC];
  unsigned num_toc;
};

#define ICNS_ERROR_SIZE 256
struct icns_data
{
  struct icns_image_set images;

  int compat_version;
  enum icns_state state;
  enum icns_target_type input_target;
  enum icns_target_type output_target;
  bool force_recoding;
  bool force_raw_if_available;

  struct
  {
    union
    {
      FILE *f;
      const uint8_t *src;
      uint8_t *dest;
    } ptr;
    enum
    {
      IO_NONE,
      IO_CALLBACK,
      IO_FILE,
      IO_MEMORY
    } type;
    size_t pos;
    size_t size;
  } io;

  void *read_priv;
  size_t (*read_fn)(void *, size_t, void *);
  size_t bytes_in;

  void *write_priv;
  size_t (*write_fn)(const void *, size_t, void *);
  size_t bytes_out;

  void *err_priv;
  void (*err_fn)(const char *);

  uint32_t requested_inputs[32];
  unsigned num_requested;

  char error_stack[64][ICNS_ERROR_SIZE];
  unsigned num_errors;
  bool is_warning;
  bool is_error;
};

#define W_(...) do { \
  unsigned i = icns->num_errors; \
  int pos = snprintf(icns->error_stack[i], ICNS_ERROR_SIZE, "%s:%s:%d: ", \
   __FILE__, __func__, __LINE__); \
  snprintf(icns->error_stack[i] + pos, ICNS_ERROR_SIZE - pos, "" __VA_ARGS__); \
  icns->num_errors++; \
  icns->is_warning = true; \
} while(0)

#define E_(...) do { \
  unsigned i = icns->num_errors; \
  int pos = snprintf(icns->error_stack[i], ICNS_ERROR_SIZE, "%s:%s:%d: ", \
   __FILE__, __func__, __LINE__); \
  snprintf(icns->error_stack[i] + pos, ICNS_ERROR_SIZE - pos, "" __VA_ARGS__); \
  icns->num_errors++; \
  icns->is_error = true; \
} while(0)

ICNS_END_DECLS

#endif /* ICNSCVT_COMMON_H */
