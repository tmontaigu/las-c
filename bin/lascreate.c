#include <stdlib.h>

#include <las/las.h>

int main(void)
{
    const char out_filename[] = "lol.las";

    las_header_t *header = malloc(sizeof(*header));
    if (header == NULL)
    {
        fprintf(stderr, "Not enough memory, buy more RAM\n");
        return EXIT_FAILURE;
    }
    *header = (las_header_t){
        .version = {
            .major = 1,
            .minor = 2,
        },
        .point_format =  {
            .id = 3,
            .num_extra_bytes = 0,
        },
        .scaling = {
            .scales = {
                .x = 0.01,
                .y = 0.01,
                .z = 0.01,
            }
        }
    };
    las_writer_t *writer;
    las_error_t las_error;
    las_raw_point_t point;

    las_raw_point_prepare(&point, header->point_format);

    las_error = las_writer_open_file_path(out_filename, header, &writer);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        // open_file_path deleted the header
        return EXIT_FAILURE;
    }

    las_error = las_writer_write_raw_point(writer, &point);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        las_writer_delete(writer);
        return EXIT_FAILURE;
    }

    point.point10.x = 100;
    point.point10.y = 100;
    point.point10.z = 100;
    las_error = las_writer_write_raw_point(writer, &point);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        las_writer_delete(writer);
        return EXIT_FAILURE;
    }

    las_raw_point_deinit(&point);
    las_writer_delete(writer);
    return EXIT_SUCCESS;
}