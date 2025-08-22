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
  compare->magic = ICNS_DATA_MAGIC;
  compare->compat_version = ICNSCVT_COMPILED_VERSION;
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


struct check_error_priv
{
  const char *title;
  const char * const *compare;
  size_t num_compare;
  size_t pos;
};

void check_error_fn(const char *message, void *priv)
{
  struct check_error_priv *data = (struct check_error_priv *)priv;
  ASSERT(priv, "");

  ASSERT(data->compare,
    "%s: should not call this function if there are no messages", data->title);

  ASSERT(data->pos < data->num_compare, "%s: message %zu past end of expected",
    data->title, data->pos);

  ASSERTCMP(message, data->compare[data->pos], "%s: %zu", data->title, data->pos);
  data->pos++;
}

UNITTEST(icns_set_error_level)
{
  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  icns_set_error_level(&icns, ICNS_ERROR_SUMMARY);
  ASSERTEQ(icns.error_level, ICNS_ERROR_SUMMARY, "");
  icns_set_error_level(&icns, ICNS_NO_ERRORS);
  ASSERTEQ(icns.error_level, ICNS_NO_ERRORS, "");
  icns_set_error_level(&icns, ICNS_ERROR_DETAILS);
  ASSERTEQ(icns.error_level, ICNS_ERROR_DETAILS, "");
  icns_set_error_level(&icns, ICNS_WARNING_DETAILS);
  ASSERTEQ(icns.error_level, ICNS_WARNING_DETAILS, "");

  /* Can't set <ICNS_NO_ERRORS */
  icns_set_error_level(&icns, ICNS_ERROR_SUMMARY);
  icns_set_error_level(&icns, ICNS_NO_ERRORS - 1);
  ASSERTEQ(icns.error_level, ICNS_ERROR_SUMMARY, "");

  /* Can't set >ICNS_WARNING_DETAILS */
  icns_set_error_level(&icns, ICNS_NO_ERRORS);
  icns_set_error_level(&icns, ICNS_WARNING_DETAILS + 1);
  ASSERTEQ(icns.error_level, ICNS_NO_ERRORS, "");

  icns_clear_state_data(&icns);
}

UNITTEST(icns_set_error_function)
{
  struct check_error_priv priv;

  struct icns_data icns;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  /* Any combination of the two works, though NULL function
   * with non-NULL priv is pointless. */
  icns_set_error_function(&icns, &priv, check_error_fn);
  ASSERTEQ(icns.err_priv, &priv, "");
  ASSERTEQ(icns.err_fn, check_error_fn, "");

  icns_set_error_function(&icns, NULL, NULL);
  ASSERTEQ(icns.err_priv, NULL, "");
  ASSERTEQ(icns.err_fn, NULL, "");

  icns_set_error_function(&icns, &priv, NULL);
  ASSERTEQ(icns.err_priv, &priv, "");
  ASSERTEQ(icns.err_fn, NULL, "");

  icns_set_error_function(&icns, NULL, check_error_fn);
  ASSERTEQ(icns.err_priv, NULL, "");
  ASSERTEQ(icns.err_fn, check_error_fn, "");

  icns_clear_state_data(&icns);
}

static void copy_error(struct icns_data *icns, const char * const *messages,
  size_t num_messages, bool is_warning)
{
  size_t i;
  for(i = 1; i < num_messages; i++)
    snprintf(icns->error_stack[i - 1], ICNS_ERROR_SIZE, "%s", messages[i]);

  if(num_messages >= 1)
    icns->num_errors = num_messages - 1;
  else
    icns->num_errors = 0;

  if(is_warning)
    icns->is_warning = true;
  else
    icns->is_error = true;
}

UNITTEST(icns_flush_error)
{
  static const char * const messages[] =
  {
    "invalid image dimensions for format",
    "testing",
    "",
    "sdfjldskjflsdkfjdslkmfds",
    "hewwo OwO;;;"
  };
  static const size_t num_messages = sizeof(messages) / sizeof(messages[0]);

  static const char * const messages2[] =
  {
    "internal error",
    "oWo"
  };
  static const size_t num_messages2 = sizeof(messages2) / sizeof(messages2[0]);

  struct check_error_priv priv;
  struct icns_data icns;
  int ret;
  icns_initialize_state_data(&icns);
  check_init(&icns);

  icns_set_error_level(&icns, ICNS_WARNING_DETAILS);
  icns_set_error_function(&icns, &priv, check_error_fn);

  priv.compare = messages;
  priv.num_compare = num_messages;

  /* Error: print messages at full detail level. */
  priv.title = "set 1 error";
  priv.pos = 0;
  copy_error(&icns, messages, num_messages, false);
  ret = icns_flush_error(&icns, ICNS_INVALID_DIMENSIONS);
  ASSERTEQ(ret, -(int)ICNS_INVALID_DIMENSIONS, "");
  check_ok(&icns, ICNS_OK);

  /* Warning: print messages at full detail level. */
  priv.title = "set 1 warning";
  priv.pos = 0;
  copy_error(&icns, messages, num_messages, true);
  icns_flush_error(&icns, ICNS_INVALID_DIMENSIONS);
  ASSERTEQ(ret, -(int)ICNS_INVALID_DIMENSIONS);
  check_ok(&icns, ICNS_OK);

  priv.compare = messages2;
  priv.num_compare = num_messages2;

  priv.title = "set 2 error";
  priv.pos = 0;
  copy_error(&icns, messages2, num_messages2, false);
  ret = icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR, "");
  check_ok(&icns, ICNS_OK);

  priv.title = "set 2 warning";
  priv.pos = 0;
  copy_error(&icns, messages2, num_messages2, true);
  icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR);
  check_ok(&icns, ICNS_OK);

  /* ICNS_ERROR_DETAILS: print at full detail level for errors, ignore warnings */
  icns_set_error_level(&icns, ICNS_ERROR_DETAILS);

  priv.title = "set 2 error (again)";
  priv.pos = 0;
  copy_error(&icns, messages2, num_messages2, false);
  ret = icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR, "");
  check_ok(&icns, ICNS_OK);

  priv.title = "set 2 warning (do not print)";
  priv.pos = 0;
  copy_error(&icns, messages2, 0, true);
  icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR);
  ASSERTEQ(priv.pos, 0, "");
  check_ok(&icns, ICNS_OK);

  /* ICNS_ERROR_SUMMARY: summary for errors, ignore warnings */
  icns_set_error_level(&icns, ICNS_ERROR_SUMMARY);

  priv.title = "set 2 error (summary only)";
  priv.pos = 0;
  copy_error(&icns, messages2, 1, false);
  ret = icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR, "");
  check_ok(&icns, ICNS_OK);

  priv.title = "set 2 warning (do not print again)";
  priv.pos = 0;
  copy_error(&icns, messages2, 0, true);
  icns_flush_error(&icns, ICNS_INTERNAL_ERROR);
  ASSERTEQ(ret, -(int)ICNS_INTERNAL_ERROR);
  ASSERTEQ(priv.pos, 0, "");
  check_ok(&icns, ICNS_OK);

  /* ICNS_NO_ERRORS: never call the callback. */
  icns_set_error_level(&icns, ICNS_NO_ERRORS);
  priv.title = "ICNS_NO_ERRORS";
  priv.pos = 0;
  copy_error(&icns, messages, num_messages, false);
  icns.is_warning = true;
  icns.num_errors = 102312;
  ret = icns_flush_error(&icns, ICNS_JP2_DATA_ERROR);
  ASSERTEQ(ret, -(int)ICNS_JP2_DATA_ERROR);
  ASSERTEQ(priv.pos, 0, "");
  check_ok(&icns, ICNS_OK);

  /* Callback not set: never call the callback */
  icns_set_error_level(&icns, ICNS_WARNING_DETAILS);
  icns_set_error_function(&icns, &priv, NULL);
  priv.title = "no error function";
  priv.pos = 0;
  icns.is_error = true;
  icns.num_errors = 52;
  ret = icns_flush_error(&icns, ICNS_PNG_NOT_A_PNG);
  ASSERTEQ(ret, -(int)ICNS_PNG_NOT_A_PNG, "");
  ASSERTEQ(priv.pos, 0, "");
  check_ok(&icns, ICNS_OK);

  icns_clear_state_data(&icns);
}
