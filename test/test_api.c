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

 #include <limits.h>

#include "test.h"

UNITTEST(icnscvt_get_linked_version)
{
  unsigned ver = icnscvt_get_linked_version();
  ASSERTEQ(ver, ICNSCVT_COMPILED_VERSION, "%u != %lu",
    ver, ICNSCVT_COMPILED_VERSION);
}

UNITTEST(icnscvt_create_context)
{
  struct icns_data *icns;
  struct icns_data compare;
  icnscvt context;

  memset(&compare, 0, sizeof(compare));
  compare.magic = ICNS_DATA_MAGIC;
  compare.compat_version = ICNSCVT_COMPILED_VERSION;

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  icns = (struct icns_data *)context;

  ASSERTEQ(icns->magic, ICNS_DATA_MAGIC, "%" PRIx32 " != %" PRIx32,
    icns->magic, ICNS_DATA_MAGIC);
  ASSERTEQ(icns->compat_version, ICNSCVT_COMPILED_VERSION,
    "%" PRIx32 " != %" PRIx32,
    icns->compat_version,
    (uint32_t)ICNSCVT_COMPILED_VERSION);
  ASSERTMEM(icns, &compare, sizeof(compare), "");

  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_destroy_context)
{
  icnscvt context = NULL;
  struct icns_data compare;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_destroy_context(context);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  /* Error on junk context. */
  ret = icnscvt_destroy_context((icnscvt)&compare);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  ret = icnscvt_destroy_context(context);
  ASSERTEQ(ret, 0, "");
}


UNITTEST(icnscvt_allocate)
{
  icnscvt context = NULL;
  struct icns_data compare;
  uint8_t *ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_allocate(context, 1234);
  ASSERTEQ(ret, NULL, "");
  /* Error on junk context. */
  ret = icnscvt_allocate((icnscvt)&compare, 1234);
  ASSERTEQ(ret, NULL, "");

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  ret = (uint8_t *)icnscvt_allocate(context, 1234);
  ASSERT(ret, "");
  ret[1233] = '\0'; /* ASan */
  icnscvt_free(context, ret);

  ret = icnscvt_allocate(context, 0);
  ASSERT(ret, "");
  ret[0] = '\0'; /* ASan */
  icnscvt_free(context, ret);

  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_free)
{
  icnscvt context = NULL;
  struct icns_data compare;
  uint8_t *buf = NULL;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_free(context, buf);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);
  /* Error on junk context. */
  ret = icnscvt_free((icnscvt)&compare, buf);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  ret = icnscvt_free(context, NULL);
  ASSERTEQ(ret, 0, "%d", ret);

  buf = icnscvt_allocate(context, 1234);
  ASSERT(buf, "");
  ret = icnscvt_free(context, buf);
  ASSERTEQ(ret, 0, "%d", ret);

  buf = icnscvt_allocate(context, 0);
  ASSERT(buf, "");
  ret = icnscvt_free(context, buf);
  ASSERTEQ(ret, 0, "%d", ret);

  /* Still error on junk context, even with a real buffer. */
  buf = icnscvt_allocate(context, 256);
  ASSERT(buf, "");
  ret = icnscvt_free((icnscvt)&compare, buf);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);
  ret = icnscvt_free(context, buf);
  ASSERTEQ(ret, 0, "%d", ret);

  icnscvt_destroy_context(context);
}


static void suppress_errors(const char *message, void *priv)
{
  (void)message;
  (void)priv;
}

UNITTEST(icnscvt_set_error_level)
{
  struct icns_data *icns;
  struct icns_data compare;
  icnscvt context = NULL;
  int ret;

  memset(&compare, 0, sizeof(compare));

  /* Error on null context. */
  ret = icnscvt_set_error_level(context, 1);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);
  /* Error on junk context. */
  ret = icnscvt_set_error_level((icnscvt)&compare, 1);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");

  icns = (struct icns_data *)context;
  icns->error_level = ICNS_WARNING_DETAILS;
  icns->err_priv = NULL;
  icns->err_fn = suppress_errors;

  /* Error if level is invalid. */
  ret = icnscvt_set_error_level(context, -1);
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  ret = icnscvt_set_error_level(context, -32768);
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  ret = icnscvt_set_error_level(context, INT_MIN);
  ASSERTEQ(ret, -ICNS_INVALID_PARAMETER, "%d != %d", ret, -ICNS_INVALID_PARAMETER);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  /* Success for valid levels. */
  ret = icnscvt_set_error_level(context, 0);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_NO_ERRORS,
    "%d != %d", icns->error_level, ICNS_NO_ERRORS);

  ret = icnscvt_set_error_level(context, 1);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_ERROR_SUMMARY,
    "%d != %d", icns->error_level, ICNS_ERROR_SUMMARY);

  ret = icnscvt_set_error_level(context, 2);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_ERROR_DETAILS,
    "%d != %d", icns->error_level, ICNS_ERROR_DETAILS);

  ret = icnscvt_set_error_level(context, 3);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  /* Values above maximum are clamped down. */
  ret = icnscvt_set_error_level(context, 4);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  ret = icnscvt_set_error_level(context, 32768);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  ret = icnscvt_set_error_level(context, INT_MAX);
  ASSERTEQ(ret, 0, "%d != 0", ret);
  ASSERTEQ(icns->error_level, ICNS_WARNING_DETAILS,
    "%d != %d", icns->error_level, ICNS_WARNING_DETAILS);

  icnscvt_destroy_context(context);
}

UNITTEST(icnscvt_set_error_function)
{
  struct icns_data *icns;
  icnscvt context = NULL;
  struct err_priv
  {
    int a;
    int b;
  } priv;
  int ret;

  priv.a = 12345;
  priv.b = -54321;

  /* Error on null context. */
  ret = icnscvt_set_error_function(context, &priv, suppress_errors);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);
  /* Error on junk context. */
  ret = icnscvt_set_error_function((icnscvt)&priv, NULL, NULL);
  ASSERTEQ(ret, -ICNS_NULL_POINTER, "%d != %d", ret, -ICNS_NULL_POINTER);

  context = icnscvt_create_context(ICNSCVT_COMPILED_VERSION);
  ASSERT(context, "");
  icns = (struct icns_data *)context;

  /* Any combination of the two works, though NULL function
   * with non-NULL priv is pointless. */
  icnscvt_set_error_function(context, &priv, suppress_errors);
  ASSERTEQ(icns->err_priv, &priv, "");
  ASSERTEQ(icns->err_fn, suppress_errors, "");

  icnscvt_set_error_function(context, NULL, NULL);
  ASSERTEQ(icns->err_priv, NULL, "");
  ASSERTEQ(icns->err_fn, NULL, "");

  icnscvt_set_error_function(context, &priv, NULL);
  ASSERTEQ(icns->err_priv, &priv, "");
  ASSERTEQ(icns->err_fn, NULL, "");

  icnscvt_set_error_function(context, NULL, suppress_errors);
  ASSERTEQ(icns->err_priv, NULL, "");
  ASSERTEQ(icns->err_fn, suppress_errors, "");

  icnscvt_destroy_context(context);
}
