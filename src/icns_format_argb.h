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

#ifndef ICNSCVT_FORMAT_ARGB_H
#define ICNSCVT_FORMAT_ARGB_H

#include "common.h"
#include "icns_format.h"

ICNS_BEGIN_DECLS

#define icns_magic_is32 MAGIC('i','s','3','2')
#define icns_magic_il32 MAGIC('i','l','3','2')
#define icns_magic_ih32 MAGIC('i','h','3','2')
#define icns_magic_it32 MAGIC('i','t','3','2')

#define icns_magic_icp4 MAGIC('i','c','p','4')
#define icns_magic_icp5 MAGIC('i','c','p','5')
#define icns_magic_ic04 MAGIC('i','c','0','4')
#define icns_magic_ic05 MAGIC('i','c','0','5')
#define icns_magic_icsb MAGIC('i','c','s','b')

extern const struct icns_format icns_format_is32;
extern const struct icns_format icns_format_il32;
extern const struct icns_format icns_format_ih32;
extern const struct icns_format icns_format_it32;

extern const struct icns_format icns_format_icp4;
extern const struct icns_format icns_format_icp5;
extern const struct icns_format icns_format_ic04;
extern const struct icns_format icns_format_ic05;
extern const struct icns_format icns_format_icsb;

ICNS_END_DECLS

#endif /* ICNSCVT_FORMAT_ARGB_H */
