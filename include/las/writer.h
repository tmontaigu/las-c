#ifndef LAS_C_WRITER_H
#define LAS_C_WRITER_H

struct las_writer_t;
typedef struct las_writer_t las_writer_t;

struct las_raw_point_t;
typedef struct las_raw_point_t las_raw_point_t;

las_error_t
las_writer_open_file_path(const char *file_path, las_header_t *header, las_writer_t **out_writer);

void las_writer_delete(las_writer_t *self);

las_error_t las_writer_write_raw_point(las_writer_t *self, const las_raw_point_t *point);

#endif // LAS_C_WRITER_H
