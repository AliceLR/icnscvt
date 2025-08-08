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

#ifndef ICNSCVT_FORMAT_MASK_H
#define ICNSCVT_FORMAT_MASK_H

#include "common.h"
#include "icns_format.h"

ICNS_BEGIN_DECLS

#define icns_magic_s8mk MAGIC('s','8','m','k')
#define icns_magic_l8mk MAGIC('l','8','m','k')
#define icns_magic_h8mk MAGIC('h','8','m','k')
#define icns_magic_t8mk MAGIC('t','8','m','k')

extern const struct icns_format icns_format_s8mk;
extern const struct icns_format icns_format_l8mk;
extern const struct icns_format icns_format_h8mk;
extern const struct icns_format icns_format_t8mk;

enum icns_error icns_add_alpha_from_8_bit_mask(struct icns_data * RESTRICT icns,
 struct icns_image * RESTRICT rgb, const struct icns_image *mask) NOT_NULL;
enum icns_error icns_split_alpha_to_8_bit_mask(struct icns_data * RESTRICT icns,
 struct icns_image * RESTRICT mask, const struct icns_image *rgb) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_FORMAT_MASK_H */
