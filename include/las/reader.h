#ifndef LAS_C_READER_H
#define LAS_C_READER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <las/error.h>
#include <stdint.h>

typedef struct las_header_t las_header_t;

typedef struct las_raw_point_t las_raw_point_t;

typedef struct las_point_t las_point_t;

typedef struct las_reader_t las_reader_t;

/// Creates a reader that reads from a file
///
/// Imediatly reads header and vlrs
las_error_t las_reader_open_file_path(const char *file_path, las_reader_t **out_reader);

/// Creates a reader that reads from an in memory buffer of bytes
las_error_t las_reader_open_buffer(const uint8_t *buffer, uint64_t size, las_reader_t **out_reader);

/// Destroy the reader
///
/// reader can be NULL
void las_reader_destroy(las_reader_t *reader);

/// Returns a reference to the reader's header
///
/// This header returned correspond the one of the currently
/// opened LAS/LAZ source.
///
/// The reader still owns the header.
const las_header_t *las_reader_header(const las_reader_t *reader);

/// Reads the next point into a raw point struct
las_error_t las_reader_read_next_raw(las_reader_t *self, las_raw_point_t *point);

/// Reads the newt point into a point struct
las_error_t las_reader_read_next(las_reader_t *self, las_point_t *point);

#ifdef __cplusplus
}
#endif

#endif // LAS_C_READER_H
