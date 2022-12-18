#include <las/las.h>

#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

int main(int argc, const char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILE_PATH\n", argv[0]);
        return 1;
    }

    const char *path = argv[1];

    las_error_t las_err;
    las_reader_t *reader = NULL;
    las_raw_point_t point = {0};
    las_writer_t *writer = NULL;

    las_err = las_reader_open_file_path(path, &reader);
    if (las_error_is_failure(&las_err))
    {
        goto main_exit;
    }

    const las_header_t *header = las_reader_header(reader);

    uint64_t point_count = header->point_count;
    printf("Point count:  %" PRIu64 "\n", point_count);
    printf("Point format: %" PRIu8 "\n", header->point_format.id);
    printf("Num extra bytes:   %" PRIu16 "\n", header->point_format.num_extra_bytes);

    if (point_count == 0)
    {
        printf("Empty file\n");
        goto main_exit;
    }

    las_raw_point_prepare(&point, header->point_format);
    las_header_t *writer_header;
    // TODO handle error
    las_header_clone(header, &writer_header);
    las_err = las_writer_open_file_path("lol.las", writer_header, &writer);

    if (las_error_is_failure(&las_err))
    {
        goto main_exit;
    }

    for (uint64_t i = 0; i < point_count; i++)
    {
        las_err = las_reader_read_next_raw(reader, &point);
        if (las_error_is_failure(&las_err))
        {
            goto main_exit;
        }

        las_err = las_writer_write_raw_point(writer, &point);
        if (las_error_is_failure(&las_err))
        {
            goto main_exit;
        }
    }

main_exit:
    if (las_error_is_failure(&las_err))
    {
        las_error_fprintf(&las_err, stderr);
    }
    las_writer_delete(writer);
    las_raw_point_deinit(&point);
    las_reader_destroy(reader);
    return las_error_is_failure(&las_err);
}