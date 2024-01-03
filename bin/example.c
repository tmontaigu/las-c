#include <las/las.h>

#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdio.h>

static double fmaxd(double a, double b)
{
    return (a > b) ? a : b;
}

static double fmind(double a, double b)
{
    return (a < b) ? a : b;
}

// Example of how to read a file
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

    las_err = las_reader_open_file_path(path, &reader);
    if (las_error_is_failure(&las_err))
    {
        goto main_exit;
    }

    const las_header_t *header = las_reader_header(reader);

    uint64_t point_count = header->point_count;
    printf("Point count:  %" PRIu64 "\n", point_count);
    printf("Point format: %" PRIu8 "\n", header->point_format.id);
    printf("Point size:   %" PRIu16 "\n", las_point_format_point_size(header->point_format));

    if (point_count == 0)
    {
        printf("Empty file\n");
        goto main_exit;
    }

    las_raw_point_prepare(&point, header->point_format);

    double x_min = DBL_MAX;
    double y_min = DBL_MAX;

    double x_max = DBL_MIN;
    double y_max = DBL_MIN;

    for (uint64_t i = 0; i < point_count; i++)
    {
        las_err = las_reader_read_next_raw(reader, &point);
        if (las_error_is_failure(&las_err))
        {
            goto main_exit;
        }

        double x, y;

        if (header->point_format.id <= 5)
        {
            x = point.point10.x;
            y = point.point10.y;
        }
        else
        {
            x = point.point14.x;
            y = point.point14.y;
        }
        x_min = fmind(x_min, x);
        y_min = fmind(y_min, y);

        x_max = fmaxd(x_max, x);
        y_max = fmaxd(y_max, y);
    }

    double x_extent = x_max - x_min;
    double y_extent = y_max - y_min;

    printf("x extent: %f\n", x_extent);
    printf("y extent: %f\n", y_extent);

main_exit:
    if (las_error_is_failure(&las_err))
    {
        las_error_fprintf(&las_err, stderr);
    }
    las_raw_point_deinit(&point);
    las_reader_destroy(reader);
    return las_error_is_failure(&las_err);
}