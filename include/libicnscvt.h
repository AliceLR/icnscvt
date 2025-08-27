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

/**
 * Library for checking, importing files into, and exporting files out of
 * ICNS and .iconset bundles. This library performs NO file IO of its own;
 * file data is loaded via a buffer or by callbacks, and is exported via
 * a pre-allocated buffer or by callbacks.
 */

#ifndef LIBICNSCVT_H
#define LIBICNSCVT_H

#include <stddef.h> /* size_t, ptrdiff_t */

#ifdef __cplusplus
extern "C" {
#endif

/* Configured variables. */
/* #define ICNSCVT_NO_FILESYSTEM */
/* End configured variables. */

#ifndef ICNSCVT_EXPORT
#ifdef ICNSCVT_STATIC
#define ICNSCVT_EXPORT
#else
#define ICNSCVT_EXPORT __attribute__((visibility("default")))
#endif
#endif

/* This macro expands to a 32-bit unsigned integer representing the version
 * of libicnscvt the including software was compiled against. To get the
 * version of the currently linked copy of libicnscvt, use the function
 * `icnscvt_get_linked_version` instead.
 */
#define ICNSCVT_COMPILED_VERSION  ICNSCVT_VERSION(0,1,0)

#define ICNSCVT_VERSION(ma,mi,r) ((((ma) & 0xfful) << 24ul) | \
                                  (((mi) & 0xffful) << 12ul) | \
                                  ((r) & 0xffful))
#define ICNSCVT_MAJOR_VER(v)      (((v) >> 24ul) & 0xfful)
#define ICNSCVT_MINOR_VER(v)      (((v) >> 12ul) & 0xffful)
#define ICNSCVT_RELEASE_VER(v)    ((v) & 0xffful)

/* An icns_image_id array of this size should fit all possible images. */
#define ICNSCVT_MAX_IMAGES        64

/* Subset numbers for icons that use e.g. dark mode. */
#define ICNSCVT_SUBSET_MAIN       0
#define ICNSCVT_SUBSET_DARK_MODE  1

typedef struct libicnscvt_opaque *icnscvt;
typedef unsigned long icns_format_id;
typedef ptrdiff_t icns_ssize_t;

typedef void   (*icnscvt_error_func)(const char *message, void *priv);
typedef size_t (*icnscvt_read_func) (void *dest, size_t sz, void *priv);
typedef size_t (*icnscvt_write_func)(const void *src, size_t sz, void *priv);

/**
 * Get the 32-bit unsigned integer corresponding to the version of libicnscvt
 * that the caller is currently linked with.
 *
 * @return                  32-bit unsigned version of the current library.
 */
ICNSCVT_EXPORT unsigned icnscvt_get_linked_version(void);

/**
 * Create a new context for icnscvt. This context corresponds to one loaded
 * ICNS file or active ICNS conversion. All functions that accept this context
 * as an argument should be accessed under a caller-managed lock in situations
 * where one context must be accessed by multiple threads, but this library
 * is fully reentrant for multiple separate contexts.
 *
 * @param compiled_version  provide ICNSCVT_COMPILED_VERSION to this argument.
 *                          This argument currently does nothing, but in the
 *                          future may be important for version locking old
 *                          behavior and/or default settings.
 * @return                  newly allocated context/state data.
 */
ICNSCVT_EXPORT icnscvt icnscvt_create_context(
  unsigned compiled_version
);

/**
 * Destroy an existing context for icnscvt. The pointer provided to this
 * function and all internal data will be freed during this call and should
 * not be used afterward.
 *
 * @param context           context/state data.
 * @return                  0 on success or a negative value on failure.
 *                          The only failure state for this function is if
 *                          the provided context pointer is invalid.
 */
ICNSCVT_EXPORT int icnscvt_destroy_context(
  icnscvt context
);

/**
 * Allocate a buffer in memory using icnscvt's internal allocation functions.
 * This is intended for environments where internal memory can not readily be
 * allocated by the caller, e.g. Emscripten. All other usages can disregard
 * this function.
 *
 * The memory allocated by this function is NOT freed by
 * `icnscvt_destroy_context`, and should instead be freed using
 * `icnscvt_free` prior to calling that function.
 *
 * @param context           context/state data.
 * @param size              size of the new memory buffer to allocate.
 *                          A valid allocation will be returned if size is 0.
 * @return                  the newly allocated memory pointer on success,
 *                          or NULL if allocation fails or if the provided
 *                          context pointer is NULL.
 */
ICNSCVT_EXPORT void *icnscvt_allocate(
  icnscvt context,
  size_t size
);

/**
 * Free a memory buffer allocated by `icnscvt_allocate`.
 * This should be performed prior to `icnscvt_destroy_context`.
 *
 * @param context           context/state data.
 * @param buf               buffer allocated by `icnscvt_allocate` to free.
 * @return                  0 on success or a negative value on failure.
 *                          The only failue state for this function is if
 *                          the provided context pointer is invalid.
 */
ICNSCVT_EXPORT int icnscvt_free(
  icnscvt context,
  void *buf
);

/**
 * Set the error reporting level. Errors will be reported by the callback
 * provided to `icnscvt_set_error_function`; otherwise, they will be printed
 * to stderr.
 *
 * Levels:
 *  0 (default) - report no information.
 *  1           - report error summaries (will print only once per call).
 *  2           - report error summaries and stack trace details.
 *  3           - report error summaries and stack trace details and warnings.
 *  4+          - clamped down to the highest available reporting level.
 *
 * @param context           context/state data.
 * @param level             error reporting level.
 * @return                  0 on success or a negative value on failure.
 */
ICNSCVT_EXPORT int icnscvt_set_error_level(
  icnscvt context,
  int level
);

/**
 * Set the error reporting function. This function will be called at the end
 * of a failed libicnscvt API function call once for every line of error
 * data to be reported, as determined by `icnscvt_set_error_level`. By default,
 * no error reporting function is specified, and libicnscvt will print error
 * information to stderr.
 *
 * The strings provided to this callback do not contain newlines or other
 * formatting, and should not be expected to have a lifetime beyond the scope
 * of the callback function call. If multiple lines are sent by one API call,
 * the first line will contain the error/warning summary, and any following
 * lines will contain details.
 *
 * NULL may be provided for either the private data pointer or callback.
 * A NULL calback is treated as no user-defined callback, which will force
 * the error reporting behavior to revert back to default (stderr).
 *
 * @param context           context/state data.
 * @param priv              private caller-defined data for the callback.
 * @param fn                caller-defined error reporting callback.
 * @return                  0 on success or a negative value on failure.
 */
ICNSCVT_EXPORT int icnscvt_set_error_function(
  icnscvt context,
  void *priv,
  icnscvt_error_func fn
);

/**
 * Get the full list of ICNS image formats supported by this libicnscvt.
 *
 * @param context           context/state data.
 * @param dest              buffer to write the list of supported format IDs to.
 *                          If this pointer is NULL, no IDs will be written.
 * @param dest_count        number of `icns_format_id` entries in `dest`.
 * @return                  the total number of image formats supported by
 *                          this libicnscvt. If this number is greated than
 *                          `dest_count`, this function will only write
 *                          `dest_count` IDs to `dest` (if non-NULL).
 *                          Returns 0 if `context` is NULL or invalid.
 */
ICNSCVT_EXPORT unsigned icnscvt_get_formats_list(
  icnscvt context,
  icns_format_id *dest,
  unsigned dest_count
);

/**
 * Gets the libicnscvt image format ID for a four letter ICNS image format
 * name. These names are case-sensitive and identical to the IFF magic used
 * in the ICNS file format for a given image format.
 *
 * @param context           context/state data.
 * @param name              four letter ICNS image format name (nul-terminated).
 * @return                  corresponding libicnscvt image format ID, or 0
 *                          if the magic string does not exist, does not
 *                          represent an image format, or if `context` or
 *                          `name` is invalid.
 */
ICNSCVT_EXPORT icns_format_id icnscvt_get_format_id_by_name(
  icnscvt context,
  const char *name
);

/**
 * Get a descriptive string for a given libicnscvt image format ID.
 * This string includes the four letter image format name (IFF magic),
 * logical dimensions, real dimensions, image type, and minimum macOS version.
 *
 * @param context           context/state data.
 * @param dest              buffer to write the descriptive string to. If this
 *                          pointer is NULL, no characters will be written.
 * @param dest_count        Maximum number of characters that can be written
 *                          to `dest`, including a nul-terminator.
 * @param which             libicnscvt image format ID to get the string of.
 * @return                  the number of characters in the full descriptive
 *                          string, not including the nul-terminator. If this
 *                          number is greater than or equal to `dest_count`,
 *                          only (`dest_count` - 1) characters of the string
 *                          plus a nul-terminator will be written to `dest`
 *                          (if `dest` is non-NULL and `dest_count` > 0).
 *                          Returns 0 if `which` or `context` is invalid.
 */
ICNSCVT_EXPORT unsigned icnscvt_get_format_string(
  icnscvt context,
  char *dest,
  unsigned dest_count,
  icns_format_id which
);


#ifdef __cplusplus
}
#endif

#endif /* LIBICNSCVT_H */
