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

#include <assert.h>
#include <limits.h>

#include "../include/libicnscvt.h"

#include "common.h"
#include "icns.h"
#include "icns_format.h"
//#include "icns_target_external.h"
//#include "icns_target_icns.h"
//#include "icns_target_iconset.h"

#define str(p) #p

#define base_check() do { \
  if(!(context) || ((struct icns_data *)context)->magic != ICNS_DATA_MAGIC) \
    return -(int)ICNS_NULL_POINTER; \
} while(0)

#define base_check_zero() do { \
  if(!(context) || ((struct icns_data *)context)->magic != ICNS_DATA_MAGIC) \
    return 0; \
} while(0)

#define base_check_ptr() do { \
  if(!(context) || ((struct icns_data *)context)->magic != ICNS_DATA_MAGIC) \
    return NULL; \
} while(0)

#define null_check(ptr) do { \
  if(!(ptr)) \
  { \
    E_("provided null pointer to function parameter '" str(ptr) "'"); \
    return icns_flush_error(icns, ICNS_NULL_POINTER); \
  } \
} while(0)

#define null_check_zero(ptr) do { \
  if(!(ptr)) \
  { \
    E_("provided null pointer to function parameter '" str(ptr) "'"); \
    icns_flush_error(icns, ICNS_NULL_POINTER); \
    return 0; \
  } \
} while(0)

#if ULONG_MAX > UINT32_MAX
#define format_id_check(format_id, id) do { \
  if((id) > UINT32_MAX) \
  { \
    E_("provided format ID '" str(id) "' is out-of-range for uint32_t: %lx", (id)); \
    return icns_flush_error(icns, ICNS_INVALID_PARAMETER); \
  } \
  *(format_id) = (id); \
} while(0)
#else
#define format_id_check(id)
#endif


unsigned icnscvt_get_linked_version(void)
{
  return ICNSCVT_COMPILED_VERSION;
}


icnscvt icnscvt_create_context(unsigned compiled_version)
{
  struct icns_data *icns = icns_allocate_state_data();
  if(!icns)
    return NULL;

  icns->compat_version = compiled_version;
  return (icnscvt)icns;
}

int icnscvt_destroy_context(icnscvt context)
{
  struct icns_data *icns = (struct icns_data *)context;
  base_check();

  icns_delete_state_data(icns);
  return 0;
}


void *icnscvt_allocate(icnscvt context, size_t size)
{
  base_check_ptr();

  /* Always allocate, even if the size is 0. */
  return malloc(size ? size : 1);
}

int icnscvt_free(icnscvt context, void *buf)
{
  base_check();
  free(buf);
  return 0;
}


int icnscvt_set_error_level(icnscvt context, int level)
{
  struct icns_data *icns = (struct icns_data *)context;
  base_check();

  if(level > ICNS_WARNING_DETAILS)
    level = ICNS_WARNING_DETAILS;

  if(level < ICNS_NO_ERRORS)
  {
    E_("error reporting level %d out-of-range", level);
    return icns_flush_error(icns, ICNS_INVALID_PARAMETER);
  }

  icns_set_error_level(icns, level);
  return icns_flush_error(icns, ICNS_OK);
}

int icnscvt_set_error_function(icnscvt context, void *priv, icnscvt_error_func fn)
{
  struct icns_data *icns = (struct icns_data *)context;
  base_check();

  icns_set_error_function(icns, priv, fn);
  return icns_flush_error(icns, ICNS_OK);
}


unsigned icnscvt_get_formats_list(icnscvt context, icns_format_id *dest,
  unsigned dest_count)
{
  const struct icns_format *list[ICNSCVT_MAX_IMAGES];
  size_t num;
  size_t i;
  base_check_zero();

  num = icns_get_format_list(NULL, 0);
  if(dest && dest_count)
  {
    /* Should never happen, as this would fail the regression tests. */
    assert(num <= ICNSCVT_MAX_IMAGES);
    icns_get_format_list(list, num);

    if(dest_count > num)
      dest_count = num;

    for(i = 0; i < dest_count; i++)
      dest[i] = list[i]->magic;
  }
  return num;
}

icns_format_id icnscvt_get_format_id_by_name(icnscvt context, const char *name)
{
  struct icns_data *icns = (struct icns_data *)context;
  const struct icns_format *format;
  base_check_zero();
  null_check_zero(name);

  format = icns_get_format_by_name(name);
  if(!format)
  {
    E_("magic string does not represent an ICNS image format");
    icns_flush_error(icns, ICNS_INVALID_PARAMETER);
    return 0;
  }
  return format->magic;
}

unsigned icnscvt_get_format_string(icnscvt context,
  char *dest, unsigned dest_count, icns_format_id which)
{
  struct icns_data *icns = (struct icns_data *)context;
  const struct icns_format *format;
  base_check_zero();

  format = icns_get_format_by_magic(which);
  if(!format)
  {
    E_("magic string does not represent an ICNS image format");
    icns_flush_error(icns, ICNS_INVALID_PARAMETER);
    return 0;
  }
  return icns_get_format_string(dest, dest_count, format);
}
