#ifndef LAS_C_WRITER_H
#define LAS_C_WRITER_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct las_writer las_writer_t;

typedef struct las_raw_point_t las_raw_point_t;

/// Creates a LAS/LAZ file for writing
///
/// - `file_path`: path where the file will be created
/// - `header`: header with metadata of the LAS to be written.
///             The writer takes ownership of the header, It must be allocated with malloc.
///             (Even if the function fail, the header will be freed)
las_error_t
las_writer_open_file_path(const char *file_path, las_header_t *header, las_writer_t **out_writer);


/// Closes the file, and deletes the writer
void las_writer_delete(las_writer_t *self);

/// Write a raw point
///
/// Writes the `point` into the writer's output
las_error_t las_writer_write_raw_point(las_writer_t *self, const las_raw_point_t *point);

las_error_t las_writer_write_many_raw_points(
    las_writer_t *self, const las_raw_point_t *points, const uint64_t num_points);


#ifdef __cplusplus
}
#endif

#endif // LAS_C_WRITER_H
