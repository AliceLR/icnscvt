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


static const char *icns_strerror(enum icns_error err)
{
  switch(err)
  {
    case ICNS_OK:
      return "ok";
    case ICNS_NO_IMAGE:
      return "image does not exist";
    case ICNS_IMAGE_EXISTS_FOR_FORMAT:
      return "image already exists";
    case ICNS_NOT_1_BIT_COLOR:
      return "image not 1-bit color";
    case ICNS_NOT_1_BIT_COLOR_MASK:
      return "image not 1-bit color with 1-bit mask";
    case ICNS_NOT_IN_PALETTE:
      return "image contains colors not in palette";
    case ICNS_NULL_POINTER:
      return "null pointer to API call";
    case ICNS_INVALID_PARAMETER:
      return "invalid parameter to API call";
    case ICNS_INTERNAL_ERROR:
      return "internal error";
    case ICNS_READ_OPEN_ERROR:
      return "error opening read stream";
    case ICNS_READ_ERROR:
      return "error reading from stream";
    case ICNS_WRITE_OPEN_ERROR:
      return "error opening write stream";
    case ICNS_WRITE_ERROR:
      return "error writing to stream";
    case ICNS_FILESYSTEM_ERROR:
      return "filesystem error";
    case ICNS_ALLOC_ERROR:
      return "allocation error";
    case ICNS_DATA_ERROR:
      return "error in input data";
    case ICNS_INVALID_DIMENSIONS:
      return "invalid image dimensions for format";
    case ICNS_UNKNOWN_CHUNK:
      return "unknown chunk in ICNS data";
    case ICNS_UNKNOWN_FORMAT:
      return "unknown image format";
    case ICNS_UNIMPLEMENTED_FORMAT:
      return "feature is unimplemented for image format";
    case ICNS_PNG_INIT_ERROR:
      return "error initializing PNG context";
    case ICNS_PNG_READ_ERROR:
      return "error reading PNG";
    case ICNS_PNG_WRITE_ERROR:
      return "error writing PNG";
    case ICNS_PNG_NOT_A_PNG:
      return "input is not a PNG";
    case ICNS_JP2_NOT_A_JP2:
      return "input is not a JPEG 2000 codestream or part 1";
    case ICNS_JP2_DATA_ERROR:
      return "error reading JPEG 2000 codestream or part 1";
  }
  return "unknown error";
}

/**
 * Set the error reporting level for the current state.
 *
 * @param icns      current state data.
 * @param level     error reporting level. Out-of-range values are ignored.
 */
void icns_set_error_level(struct icns_data *icns, enum icns_error_level level)
{
  if(level >= ICNS_NO_ERRORS && level <= ICNS_WARNING_DETAILS)
    icns->error_level = level;
}

/**
 * Set the error reporting callback for the current state.
 * This callback may be called multiple times during an API function call.
 * The first call corresponds to a general error message, and the following
 * calls provide call stack information (if allowed by the current error level).
 * Typically, this callback should perform something equivalent to:
 *
 * void fn(const char *message, void *priv)
 * {
 *   fprintf(stderr, "%s\n", message);
 * }
 *
 * @param icns      current state data.
 * @param priv      private data pointer for error message callback.
 * @param err_func  error message callback.
 */
void icns_set_error_function(struct icns_data * RESTRICT icns,
  void * RESTRICT priv, void (*err_func)(const char *message, void *priv))
{
  icns->err_priv = priv;
  icns->err_fn = err_func;
}

/**
 * Flush error data to the error stream at the requested detail level, then
 * return an integer error value. This function resets the context error state.
 * This function should be called once every time an API function returns.
 *
 * @param icns    current state data.
 * @param err     error value.
 * @return        integer equivalent of the provided error value.
 */
int icns_flush_error(struct icns_data *icns, enum icns_error err)
{
  if(icns->err_fn && icns->error_level >= ICNS_ERROR_SUMMARY)
  {
    if(icns->is_error || icns->error_level >= ICNS_WARNING_DETAILS)
      icns->err_fn(icns_strerror(err), icns->err_priv);

    if((icns->is_error && icns->error_level >= ICNS_ERROR_DETAILS) ||
       (icns->is_warning && icns->error_level >= ICNS_WARNING_DETAILS))
    {
      unsigned i;
      for(i = 0; i < icns->num_errors; i++)
        icns->err_fn(icns->error_stack[i], icns->err_priv);
    }
  }
  icns->is_error = false;
  icns->is_warning = false;
  icns->num_errors = 0;
  return -(int)err;
}
