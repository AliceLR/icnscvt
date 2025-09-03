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

#ifndef ICNSCVT_ICNS_TARGET_EXTERNAL_H
#define ICNSCVT_ICNS_TARGET_EXTERNAL_H

#include "common.h"

ICNS_BEGIN_DECLS

enum icns_error icns_load_image_from_external_memory(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const uint8_t *src, size_t src_size) NOT_NULL;
enum icns_error icns_save_image_to_external_memory(
  struct icns_data * RESTRICT icns, uint32_t format_id, size_t *total_out,
  uint8_t * RESTRICT dest, size_t dest_size) NOT_NULL;

enum icns_error icns_load_image_from_external_callback(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  void *priv, size_t (*read_func)(void *dest, size_t sz, void *priv))
  NOT_NULL_2(1, 4);
enum icns_error icns_save_image_to_external_callback(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  void *priv, size_t (*write_func)(const void *src, size_t sz, void *priv))
  NOT_NULL_2(1, 4);

enum icns_error icns_load_image_from_external_file(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const char *filename) NOT_NULL;
enum icns_error icns_save_image_to_external_file(
  struct icns_data * RESTRICT icns, uint32_t format_id,
  const char *filename) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_ICNS_TARGET_EXTERNAL_H */
