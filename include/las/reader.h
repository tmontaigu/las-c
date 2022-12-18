#ifndef LAS_C_READER_H
#define LAS_C_READER_H

#include <las/error.h>
#include <stdint.h>

struct las_header_t;
typedef struct las_header_t las_header_t;

struct las_raw_point_t;
typedef struct las_raw_point_t las_raw_point_t;

struct las_point_t;
typedef struct las_point_t las_point_t;

struct las_reader_t;
typedef struct las_reader_t las_reader_t;

las_error_t las_reader_open_file_path(const char *file_path, las_reader_t **out_reader);

las_error_t las_reader_open_buffer(const uint8_t *buffer, uint64_t size, las_reader_t **out_reader);

void las_reader_destroy(las_reader_t *reader);

/// Returns a reference to the reader's header
///
/// This header returned correspond the one of the currently
/// opened LAS/LAZ source.
///
/// The reader owns the header.
const las_header_t *las_reader_header(const las_reader_t *reader);

las_error_t las_reader_read_next_raw(las_reader_t *self, las_raw_point_t *point);

las_error_t las_reader_read_next(las_reader_t *self, las_point_t *point);

#endif // LAS_C_READER_H
