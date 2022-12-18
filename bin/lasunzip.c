#include <stdio.h>
#include <string.h>

#include <las/las.h>
#include <malloc.h>

int main(int argc, char * argv[]) {
    if (argc < 2) {
        fprintf(stderr, "USAGE: %s INPUT_FILE [OUTPUT_FILE]", argv[0]);
        return 1;
    }

    const char *filename = argv[1];

    char *dest_filename;
    if (argc == 3)
    {
        if (strcmp(filename, argv[2]) == 0)
        {
            fprintf(stderr, "The output filename must be different than the input filename");
            return 1;
        }

        const char *last_dot = strrchr(argv[2], '.');
        if (last_dot == NULL || (strcmp(last_dot, ".las") != 0))
        {
            fprintf(stderr, "The output file name must end with '.las'");
            return 1;
        }
        // duplicate to make memory handling easier
        dest_filename = malloc(sizeof(char) * (strlen(argv[2]) + 1));
        strcpy(dest_filename, argv[2]);
    } else {
        // strdup will be standard in C23
        unsigned long long len = strlen(filename);
        dest_filename= malloc(sizeof(char) * (len + 1));
        strcpy(dest_filename, filename);
        dest_filename[len - 1] = 's';
    }

    las_error_t err = { LAS_ERROR_OK };
    las_reader_t *reader = NULL;
    las_writer_t *writer = NULL;
    las_raw_point_t raw_point = {0};

    err = las_reader_open_file_path(filename, &reader);
    if (las_error_is_failure(&err))
    {
        goto main_exit;
    }

    const las_header_t *reader_header = las_reader_header(reader);
    las_header_t *writer_header;
    // TODO handle error
    las_header_clone(reader_header, &writer_header);

    las_raw_point_prepare(&raw_point, writer_header->point_format);


    err = las_writer_open_file_path(dest_filename, writer_header, &writer);
    if (las_error_is_failure(&err))
    {
        goto main_exit;
    }

    uint64_t point_count = reader_header->point_count;

    for (uint64_t i = 0; i < point_count; i++)
    {
        err = las_reader_read_next_raw(reader, &raw_point);
        if (las_error_is_failure(&err))
        {
            goto main_exit;
        }

        err = las_writer_write_raw_point(writer, &raw_point);
        if (las_error_is_failure(&err))
        {
            goto main_exit;
        }
    }

main_exit:
    las_reader_destroy(reader);
    las_writer_delete(writer);
    las_raw_point_deinit(&raw_point);

    if (dest_filename != NULL)
    {
        free(dest_filename);
    }

    if (las_error_is_failure(&err))
    {
        las_error_fprintf(&err, stderr);
    }

    return las_error_is_failure(&err);
}