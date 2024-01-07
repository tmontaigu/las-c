#include <stdio.h>
#include <string.h>

#include <las/las.h>
#include <malloc.h>

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "USAGE: %s INPUT_FILE OUTPUT_FILE", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    const char *dest_filename = argv[2];
    const uint64_t chunk_size = 10000000; // TODO make this an argument


    if (strcmp(filename, dest_filename) == 0)
    {
        fprintf(stderr, "The output filename must be different than the input filename");
        return 1;
    }

    las_error_t err = {.kind = LAS_ERROR_OK};
    las_reader_t *reader = NULL;
    las_writer_t *writer = NULL;
    las_raw_point_t *raw_points = NULL;

    err = las_reader_open_file_path(filename, &reader);
    if (las_error_is_failure(&err))
    {
        goto main_exit;
    }

    const las_header_t *reader_header = las_reader_header(reader);
    las_header_t *writer_header;
    if (las_header_clone(reader_header, &writer_header) == 1)
    {
        fprintf(stderr, "Failed to clone LAS header\n");
        goto main_exit;
    }

    // Prepare the point buffer for "chunked" reading
    raw_points = malloc(sizeof(las_raw_point_t) * chunk_size);
    if (raw_points == NULL)
    {
        fprintf(stderr, "Not enough memory\n");
        goto main_exit;
    }
    las_raw_point_prepare_many(raw_points, chunk_size, reader_header->point_format);

    err = las_writer_open_file_path(dest_filename, writer_header, &writer);
    if (las_error_is_failure(&err))
    {
        goto main_exit;
    }

    uint64_t points_left = reader_header->point_count;
    while (points_left != 0)
    {
        const uint64_t num_points_to_read = (chunk_size > points_left) ? points_left : chunk_size;
        err = las_reader_read_many_next_raw(reader, raw_points, num_points_to_read);
        if (las_error_is_failure(&err))
        {
            goto main_exit;
        }

        err = las_writer_write_many_raw_points(writer, raw_points, num_points_to_read);
        if (las_error_is_failure(&err))
        {
            goto main_exit;
        }

        points_left -= num_points_to_read;
    }

main_exit:
    las_reader_destroy(reader);
    las_writer_delete(writer);

    if (raw_points != NULL)
    {
        las_raw_point_deinit_many(raw_points, chunk_size);
        free(raw_points);
    }

    if (las_error_is_failure(&err))
    {
        las_error_fprintf(&err, stderr);
    }

    return las_error_is_failure(&err);
}
