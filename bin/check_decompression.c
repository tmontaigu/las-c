#include <las/las.h>

#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s FILE_PATH_1, FILE_PATH_2\n", argv[0]);
        return 1;
    }

    const char *path_1 = argv[1];
    const char *path_2 = argv[2];

    las_error_t las_err;
    las_reader_t *reader_1 = NULL;
    las_raw_point_t point_1 = {0};
    las_reader_t *reader_2 = NULL;
    las_raw_point_t point_2 = {0};

    las_err = las_reader_open_file_path(path_1, &reader_1);
    if (las_error_is_failure(&las_err))
    {
        goto main_exit;
    }

    las_err = las_reader_open_file_path(path_2, &reader_2);
    if (las_error_is_failure(&las_err))
    {
        goto main_exit;
    }

    const las_header_t *header_1 = las_reader_header(reader_1);
    const las_header_t *header_2 = las_reader_header(reader_2);

    uint64_t point_count_1 = header_1->point_count;
    uint64_t point_count_2 = header_2->point_count;

    if (point_count_1 != point_count_2)
    {
        printf("not same point count\n");
        goto main_exit;
    }

    las_raw_point_prepare(&point_1, header_1->point_format);
    las_raw_point_prepare(&point_2, header_2->point_format);

    for (uint64_t i = 0; i < point_count_1; i++)
    {
        las_err = las_reader_read_next_raw(reader_1, &point_1);
        if (las_error_is_failure(&las_err))
        {
            fprintf(stderr,"error for reader & (file: %s)\n", path_1);
            goto main_exit;
        }

        las_err = las_reader_read_next_raw(reader_2, &point_2);
        if (las_error_is_failure(&las_err))
        {
            fprintf(stderr, "error for reader 2 (file: %s)\n", path_2);
            goto main_exit;
        }

        if (las_raw_point_eq(&point_1, &point_2) != 1)
        {
            printf("Point %" PRIu64  "not equal\n", i);
            goto main_exit;
        }
    }

main_exit:
    if (las_error_is_failure(&las_err))
    {
        las_error_fprintf(&las_err, stderr);
    }
    las_raw_point_deinit(&point_1);
    las_raw_point_deinit(&point_2);
    las_reader_destroy(reader_1);
    las_reader_destroy(reader_2);
    return las_error_is_failure(&las_err);
}