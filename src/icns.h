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

#ifndef ICNSCVT_ICNS_H
#define ICNSCVT_ICNS_H

#include "common.h"

ICNS_BEGIN_DECLS

struct icns_data *icns_allocate_state_data(void);
void icns_initialize_state_data(struct icns_data *icns) NOT_NULL;
void icns_clear_state_data(struct icns_data *icns) NOT_NULL;
void icns_delete_state_data(struct icns_data *icns) NOT_NULL;

ICNS_END_DECLS

#endif /* ICNSCVT_ICNS_H */
