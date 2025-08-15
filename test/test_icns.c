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
#include "../src/icns.h"
#include "../src/icns_image.h"

NOT_NULL
static void test_init_compare(struct icns_data *compare)
{
  memset(compare, 0, sizeof(struct icns_data));
}

UNITTEST(icns_initialize_state_data)
{
  struct icns_data icns;
  struct icns_data compare;
  icns_initialize_state_data(&icns);

  test_init_compare(&compare);
  ASSERTMEM(&icns, &compare, sizeof(compare), "should be initialized to 0");
}

UNITTEST(icns_allocate_state_data)
{
  struct icns_data *icns;
  struct icns_data compare;

  icns = icns_allocate_state_data();
  ASSERT(icns, "failed to allocate state data");

  test_init_compare(&compare);
  ASSERTMEM(icns, &compare, sizeof(compare), "should be initialized to 0");

  icns_delete_state_data(icns);
}

UNITTEST(icns_delete_state_data)
{
  struct icns_data *icns;

  icns = icns_allocate_state_data();
  ASSERT(icns, "failed to allocate state data");

  /* Ensure most data is not freed. */
  memset(icns, 0xff, sizeof(*icns));

  /* Should not leak allocated image. */
  icns->images.head = (struct icns_image *)calloc(1, sizeof(struct icns_image));
  icns->images.tail = icns->images.head;
  icns->images.num_images = 1;
  ASSERT(icns->images.head, "failed to allocate image");

  icns_delete_state_data(icns);
}

UNITTEST(icns_clear_state_data)
{
  struct icns_data *icns;
  struct icns_data icns_st;
  struct icns_data compare;

  icns_initialize_state_data(&icns_st);
  icns = icns_allocate_state_data();
  ASSERT(icns, "failed to allocate state data");
  test_init_compare(&compare);

  /* Should be able to clear either with no issue. */
  icns_clear_state_data(icns);
  icns_clear_state_data(&icns_st);
  ASSERTMEM(icns, &compare, sizeof(compare), "should be identical");
  ASSERTMEM(&icns_st, &compare, sizeof(compare), "should be identical");

  /* Should free from either with no issue. */
  memset(icns, 0xff, sizeof(*icns));
  memset(&icns_st, 0xff, sizeof(icns_st));

  icns->images.head = (struct icns_image *)calloc(1, sizeof(struct icns_image));
  icns->images.tail = icns->images.head;
  icns->images.num_images = 1;
  ASSERT(icns->images.head, "failed to allocate image");

  icns_st.images.head = (struct icns_image *)calloc(1, sizeof(struct icns_image));
  icns_st.images.tail = icns->images.head;
  icns_st.images.num_images = 1;
  ASSERT(icns_st.images.head, "failed to allocate image");

  icns_clear_state_data(icns);
  icns_clear_state_data(&icns_st);
  ASSERTMEM(icns, &compare, sizeof(compare), "should be identical");
  ASSERTMEM(&icns_st, &compare, sizeof(compare), "should be identical");

  icns_delete_state_data(icns);
}
