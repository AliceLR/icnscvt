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

#ifndef ICNSCVT_TARGET_H
#define ICNSCVT_TARGET_H

#include "test.h"

ICNS_BEGIN_DECLS

struct target_test
{
  const char *file;
  uint32_t format;
  uint32_t format_loaded;

  enum icns_error expected;
  icns_ssize_t api_expected;
};

extern const struct target_test external_load_sequence[];
extern const size_t num_external_load_sequence;

extern const struct target_test external_save_sequence[];
extern const size_t num_external_save_sequence;

struct icns_image *get_image_by_magic(struct icns_data *icns, uint32_t magic);

ICNS_END_DECLS

#endif /* ICNSCVT_TARGET_H */
