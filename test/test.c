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

#ifdef __unix__
#include <sys/wait.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <process.h> /* _spawnl, _P_WAIT */
#endif

#include "test.h"
#include "testdecls.h"

struct test_entry
{
  test_func func;
  const char *name;
};

static const struct test_entry tests[] =
{
#undef UNITDECL
#define UNITDECL(x) { icnscvt_unit_ ## x, STR(x) },
#include "testdecls.h"
};
static size_t num_tests = sizeof(tests) / sizeof(tests[0]);

static int total = 0;
static int failed = 0;
static int passed = 0;

/* Run an individual test in a child process to allow
 * crash recovery and pseudo-exception handling. */
static bool run_test(const struct test_entry *test, const char *argv0, size_t i)
{
#ifdef __unix__

  int status;
  int pid;

  (void)argv0;
  (void)i;

  pid = fork();
  if(pid == 0)
  {
    test->func();
    exit(0);
  }
  waitpid(pid, &status, 0);
  return status == 0;

#elif defined(_WIN32)

  char args[32];
  snprintf(args, sizeof(args), "%zu", i);

  (void)test;

  if(_spawnl(_P_WAIT, argv0, argv0, args, NULL))
    return false;

  return true;

#else

  /* Run with system() (bad...). */
  char argvs[512];
  snprintf(args, sizeof(args), "%s %zu", argv0, i);

  (void)test;

  if(system(args) != 0)
    return false;

  return true;

#endif
}

int main(int argc, char *argv[])
{
  const struct test_entry *test;
  int ret = 0;
  size_t i;

  /* Run individual test (usually via _spawnl or system). */
  if(argc > 1)
  {
    size_t which = strtoul(argv[1], NULL, 10);
    if(which >= 1 && which <= num_tests)
    {
      tests[which - 1].func();
      return 0;
    }
    OUT("invalid test number %zu", which);
    return 2;
  }

  /* Run all tests. */
  for(i = 0; i < num_tests; i++)
  {
    test = &tests[i];
    OUT("test %zu: %s... ", i, test->name);

    if(run_test(test, argv[0], i))
    {
      OUT("ok\n");
      passed++;
    }
    else
    {
      OUT("\n  %s failed\n", test->name);
      failed++;
      ret = 1;
    }
    total++;
  }
  OUT("\ntotal:  %d\npassed: %d\nfailed: %d\n", total, passed, failed);
  return ret;
}
