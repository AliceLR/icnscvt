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

#include "icns.h"
#include "icns_image.h"

/**
 * Allocate and initialize the icnscvt state data.
 *
 * @return    an initialized `icns_data` on success, otherwise `NULL`.
 */
struct icns_data *icns_allocate_state_data(void)
{
  struct icns_data *icns = (struct icns_data *)malloc(sizeof(struct icns_data));
  if(icns)
    icns_initialize_state_data(icns);

  return icns;
}

/**
 * Initialize a pre-existing icnscvt state data on the stack.
 * This should NOT be used on an `icns_data` that is already initialized,
 * and should be called directly only for static linked regression tests.
 *
 * @param icns    uninitialized state data structure to initialize.
 */
void icns_initialize_state_data(struct icns_data *icns)
{
  memset(icns, 0, sizeof(struct icns_data));
}

static void icns_free_all(struct icns_data *icns)
{
  icns_delete_all_images(icns);
}

/**
 * Reset an icnscvt state data structure back to its initial state, freeing
 * all data as-needed.
 *
 * @param icns    current state data structure to reset.
 */
void icns_clear_state_data(struct icns_data *icns)
{
  icns_free_all(icns);
  icns_initialize_state_data(icns);
}

/**
 * Free an icnscvt state data structure and all internal data.
 * The pointer provided will be invalid upon the return of this function.
 *
 * @param icns    current state data structure to free.
 */
void icns_delete_state_data(struct icns_data *icns)
{
  icns_free_all(icns);
  free(icns);
}
