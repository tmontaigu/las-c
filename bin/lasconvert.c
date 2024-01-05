#include <las/las.h>

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

const char *point_format_arg_str = "--point-format";
const char *version_arg_str = "--version";

typedef struct
{
    const char *input_file;
    const char *output_file;

    uint8_t target_format_id;
    las_version_t target_version;
} argument_t;

argument_t parse_arguments(const int argc, char *argv[])
{
    argument_t args = {
        .input_file = NULL,
        .output_file = NULL,
        .target_format_id = 255,
    };

    for (int i = 1; i < argc; ++i)
    {
        const char *arg = argv[i];

        if (strcmp(arg, point_format_arg_str) == 0)
        {
            ++i;
            if (i >= argc)
            {
                fprintf(stderr, "Expected number after '%s'\n", point_format_arg_str);
                exit(1);
            }
            char *end;
            unsigned long id = strtoul(argv[i], &end, 10);
            if ((id == 0 || id == ULONG_MAX) && errno != 0)
            {
                fprintf(stderr, "Failed to convert %s to a point format id\n", argv[i]);
                perror("");
                exit(1);
            }

            if (id >= (unsigned long)UINT8_MAX)
            {
                fprintf(stderr, "'%lu' is not a valid point format id\n", id);
                exit(1);
            }

            args.target_format_id = (uint8_t)id;
        }
        else if (strcmp(arg, version_arg_str) == 0)
        {
            ++i;
            if (i >= argc)
            {
                fprintf(stderr, "Expected version (eg '1.2') after '%s'\n", version_arg_str);
                exit(1);
            }

            const char *version_str = argv[i];

            const char *dot_pos = strchr(version_str, '.');
            if (dot_pos == NULL)
            {
                fprintf(stderr, "Invalid version missing '.'\n");
                exit(1);
            }

            char *end;
            const unsigned long major = strtoul(version_str, &end, 10);
            if ((major == 0 || major == ULONG_MAX) && errno != 0)
            {
                fprintf(stderr, "Failed to convert %s to a version\n", version_str);
                perror("");
                exit(1);
            }

            if (major >= (unsigned long)UINT8_MAX)
            {
                fprintf(stderr, "Failed to convert %s to a version\n", version_str);
                exit(1);
            }

            const unsigned long minor = strtoul(dot_pos + 1, &end, 10);
            if ((minor == 0 || minor == ULONG_MAX) && errno != 0)
            {
                fprintf(stderr, "Failed to convert %s to a version\n", version_str);
                perror("");
                exit(1);
            }

            if (minor >= (unsigned long)UINT8_MAX)
            {
                fprintf(stderr, "Failed to convert %s to a version\n", version_str);
                exit(1);
            }

            args.target_version.major = (uint8_t)major;
            args.target_version.minor = (uint8_t)minor;
        }
        else if (args.input_file == NULL)
        {
            args.input_file = arg;
        }
        else if (args.output_file == NULL)
        {
            args.output_file = arg;
        }
        else
        {
            fprintf(stderr, "Unexpected argument '%s'\n", arg);
            exit(1);
        }
    }

    if (args.input_file == NULL || args.output_file == NULL || args.target_format_id == 255)
    {
        fprintf(stderr, "Not enough arguments\n");
        fprintf(stderr, "Usage: %s [--point_format] [--version] INPUT_FILE OUTPUT_FILE\n", argv[0]);
        exit(1);
    }

    return args;
}

int main(int argc, char *argv[])
{
    argument_t args = parse_arguments(argc, argv);

    las_error_t las_err = {.kind = LAS_ERROR_OK};

    las_reader_t *reader = NULL;
    las_writer_t *writer = NULL;
    las_raw_point_t source_point = {0};
    las_raw_point_t dest_point = {0};

    las_err = las_reader_open_file_path(args.input_file, &reader);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    const las_header_t *reader_header = las_reader_header(reader);
    las_header_t *header;
    if (las_header_clone(reader_header, &header) != 0)
    {
        goto out;
    }

    header->point_format.id = args.target_format_id;
    if (args.target_version.major != 0)
    {
        header->version = args.target_version;
    }

    las_raw_point_prepare(&source_point, reader_header->point_format);
    las_raw_point_prepare(&dest_point, header->point_format);

    las_err = las_writer_open_file_path(args.output_file, header, &writer);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    for (uint64_t i = 0; i < reader_header->point_count; ++i)
    {
        las_err = las_reader_read_next_raw(reader, &source_point);
        if (las_error_is_failure(&las_err))
        {
            goto out;
        }

        las_raw_point_copy_from_raw(&dest_point, &source_point);

        las_err = las_writer_write_raw_point(writer, &dest_point);
        if (las_error_is_failure(&las_err))
        {
            goto out;
        }
    }

out:
    las_raw_point_deinit(&source_point);
    las_raw_point_deinit(&dest_point);
    las_reader_destroy(reader);
    las_writer_delete(writer);

    if (las_error_is_failure(&las_err))
    {
        las_error_fprintf(&las_err, stderr);
    }

    return las_error_is_failure(&las_err);
}
