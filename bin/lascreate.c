#include <stdlib.h>
#include <string.h>

#include <las/las.h>

// Example of how to create a new LAS/LAZ from scratch
int main(void)
{
    const char out_filename[] = "output.las";

    las_header_t *header = malloc(sizeof(*header));
    las_vlr_t *vlrs = malloc(sizeof(las_vlr_t));

    if (header == NULL || vlrs == NULL)
    {
        fprintf(stderr, "Not enough memory, buy more RAM\n");
        return EXIT_FAILURE;
    }

    // Create 1 VLR
    char *data = strdup("Hello World");
    vlrs[0] = (las_vlr_t) {
        .record_id = 17,
        // Beware that for user_id and description,
        // there is maximum length.
        .user_id = "User",
        .description = "A Description",
        .data_size = strlen(data),
        .data = (uint8_t*) data,
    };

    // Initialize the header
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
        },
        // Put our VLR
        .number_of_vlrs = 1,
        .vlrs = vlrs
    };

    las_error_t las_error;
    las_raw_point_t point;
    las_writer_t *writer;

    // The raw point has to be prepared
    las_raw_point_prepare(&point, header->point_format);

    las_error = las_writer_open_file_path(out_filename, header, &writer);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        // open_file_path deleted the header
        return EXIT_FAILURE;
    }

    // Write a first point (yes, with only zeros)
    las_error = las_writer_write_raw_point(writer, &point);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        las_raw_point_deinit(&point);
        las_writer_delete(writer);
        return EXIT_FAILURE;
    }

    // Set the integer x,y,z and write the second point
    point.point10.x = 100;
    point.point10.y = 100;
    point.point10.z = 100;
    las_error = las_writer_write_raw_point(writer, &point);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        las_raw_point_deinit(&point);
        las_writer_delete(writer);
        return EXIT_FAILURE;
    }

    // Set the x,y,z from double values and write the third point
    // This uses the scaling defined in the header to compute the integer x,y,z
    point.point10.x = las_scaling_unapply_x(header->scaling, 2.0);
    point.point10.y = las_scaling_unapply_y(header->scaling, 2.0);
    point.point10.z = las_scaling_unapply_z(header->scaling, 2.0);
    las_error = las_writer_write_raw_point(writer, &point);
    if (las_error_is_failure(&las_error))
    {
        las_error_fprintf(&las_error, stderr);
        las_raw_point_deinit(&point);
        las_writer_delete(writer);
        return EXIT_FAILURE;
    }

    las_raw_point_deinit(&point);
    las_writer_delete(writer);
    return EXIT_SUCCESS;
}