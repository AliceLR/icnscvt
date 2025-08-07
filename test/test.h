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

/* Stripped down, C-only version of MegaZeux Unit test system.
 * Also significantly inspired by Claudio Matsuoka's test system for libxmp. */

#ifndef ICNSCVT_TEST_H
#define ICNSCVT_TEST_H

#include "../src/common.h"

#include <stdio.h>

ICNS_BEGIN_DECLS

#define DATA_DIR      "test/data"
#define TEMP_DIR      "test/tmp"

#define DIRENT_DIR    DATA_DIR "/dirent"
#define INTERNAL_DIR  DATA_DIR "/internal"
#define PNG_DIR       DATA_DIR "/png"

#define STR2(x)     #x
#define STR(x)      STR2(x)

#define OUT(...)    do { \
                      fprintf(stderr, "" __VA_ARGS__); \
                      fflush(stderr); \
                    } while(0)

typedef             void (*test_func)(void);
#define UNITPRO(x)  void icnscvt_unit_ ## x (void)
#define UNITDECL(x) UNITPRO(x);
#define UNITTEST(x) UNITPRO(x); UNITPRO(x)

#define FAIL(...) \
  do { \
    OUT("\n at %s:%d (%s):", \
      __FILE__, __LINE__, __func__); \
    OUT("\n  " __VA_ARGS__);\
    exit(1); \
  } while(0)

#define UNIMPLEMENTED() \
  do { \
    FAIL("test is not implemented"); \
  } while(0)

#define ASSERT_FAIL(a_str, b_str, file, func, line, ...) \
  do { \
    OUT("\n  at %s:%d (%s):", file, line, func); \
    OUT("\n    '%s' != '%s'", a_str, b_str); \
    OUT(" (" __VA_ARGS__); \
    OUT(")"); \
    exit(1); \
  } while(0)

#define ASSERT(x, ...) \
  do { if(!(x)) \
    ASSERT_FAIL(STR(x), "", __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while(0)

#define ASSERTEQ(a, b, ...) \
  do { if((a) != (b)) \
    ASSERT_FAIL(STR(a), STR(b), __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while(0)

#define ASSERTCMP(a, b, ...) \
  do { if(strcmp((a), (b))) \
    ASSERT_FAIL((a), (b), __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while(0)

#define ASSERTMEM(a, b, n, ...) \
  do { if(memcmp((a), (b), (n))) \
    ASSERT_FAIL(STR(a) "[" STR(n) "]", \
                STR(b) "[" STR(n) "]", \
                __FILE__, __func__, __LINE__, __VA_ARGS__); \
  } while(0)

static inline void clear_error(struct icns_data *icns)
{
  icns->num_errors = 0;
  icns->is_error = false;
  icns->is_warning = false;
}

#define check_init(icns) \
  do { \
    ASSERTEQ((icns)->io.type, IO_NONE, "%d != %d", (icns)->io.type, IO_NONE); \
    ASSERTEQ((icns)->read_priv, NULL, ""); \
    ASSERTEQ((icns)->read_fn, NULL, ""); \
    ASSERTEQ((icns)->bytes_in, 0, ""); \
    ASSERTEQ((icns)->write_priv, NULL, ""); \
    ASSERTEQ((icns)->write_fn, NULL, ""); \
    ASSERTEQ((icns)->bytes_out, 0, ""); \
  } while(0)

#define check_ok(icns, ret) \
  do { \
    if((icns)->num_errors) \
    { \
      size_t _i; \
      for(_i = 0; _i < (icns)->num_errors; _i++) \
        OUT("\n    %s", (icns)->error_stack[_i]); \
    } \
    ASSERTEQ(ret, ICNS_OK, "%d != %d", ret, ICNS_OK); \
    ASSERT((icns)->num_errors == 0, "error messages are set"); \
    ASSERT(!(icns)->is_error, "error state is set"); \
    ASSERT(!(icns)->is_warning, "warning state is set"); \
  } while(0)

#define check_ok_var(icns, ret, expected) \
  do { \
    ASSERTEQ(ret, expected, "%d != %d", ret, expected); \
    ASSERT((icns)->num_errors == 0, "error messages are set"); \
    ASSERT(!(icns)->is_error, "error state is set"); \
    ASSERT(!(icns)->is_warning, "warning state is set"); \
  } while(0)

#define check_warning(icns) \
  do { \
    ASSERT((icns)->num_errors > 0, "no error messages set"); \
    ASSERT((icns)->is_warning, "warning flag not set"); \
    ASSERT(!(icns)->is_error, "error flag is set"); \
    clear_error(icns); \
  } while(0)

#define check_error(icns, ret, expected) \
  do { \
    ASSERTEQ(ret, expected, "%d != %d", ret, expected); \
    ASSERT((icns)->num_errors > 0, "no error messages set"); \
    ASSERT((icns)->is_error, "error flag not set"); \
    clear_error(icns); \
  } while (0)

ICNS_END_DECLS

#endif /* ICNSCVT_TEST_H */
