#include <inttypes.h>
#include <limits.h>
#include <stdio.h>

#include "private/header.h"
#include "private/macro.h"

void las_error_fprintf(const las_error_t *self, FILE *stream)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(stream != NULL);

    fprintf(stream, "las_error: ");
    switch (self->kind)
    {
    case LAS_ERROR_OK:
        fprintf(stream, "no error\n");
        break;
    case LAS_ERROR_MEMORY:
        fprintf(stream, "out of memory\n");
        break;
    case LAS_ERROR_ERRNO:
        perror("");
        break;
    case LAS_ERROR_UNEXPECTED_EOF:
        fprintf(stream, "Unexpected end of file/input\n");
        break;
    case LAS_ERROR_INVALID_SIGNATURE:
        fprintf(stream, "Invalid file signature '%s'\n", self->signature);
        break;
    case LAS_ERROR_INVALID_VERSION:
        fprintf(stream,
                "The version `%d.%d` is invalid or not supported\n",
                (int)self->version.major,
                (int)self->version.minor);
        break;
    case LAS_ERROR_INVALID_POINT_FORMAT:
        fprintf(stream,
                "The point format `%I32u` is invalid or not supported\n",
                (uint32_t)self->point_format_id);
        break;
    case LAS_ERROR_INVALID_POINT_SIZE:
        fprintf(stream,
                "The file has an invalid point_size `%d`, for the given point "
                "format `%d`.\n",
                (int)self->point_size.point_size,
                (int)self->point_size.point_format_id);
        fprintf(
            stream, "           At least `%d` bytes are expected\n", (int)self->point_size.minimum);
        break;
    case LAS_ERROR_MISSING_LASZIP_VLR:
        fprintf(stream, "laszip vlr not found, cannot decompress points\n");
        break;
    case LAS_ERROR_INCOMPATIBLE_VERSION_AND_FORMAT:
        fprintf(stream,
                "point format `%d` is incompatible with version `%d.%d`\n",
                (int)self->incompatible.point_format_id,
                (int)self->incompatible.version.major,
                (int)self->incompatible.version.minor
                );
        break;
    case LAS_ERROR_INCOMPATIBLE_POINT_FORMAT:
        fprintf(stream, "The point format of does not match");
        break;
    case LAS_ERROR_POINT_COUNT_TOO_HIGH:
        fprintf(stream,
                "The point_count `%" PRIu64 "` exceeds the maximum",
                self->point_count);
        break;

#ifdef WITH_LAZRS
    case LAS_ERROR_LAZRS:
        lazrs_fprint_result(self->lazrs, stream);
        break;
#endif
#ifndef WITH_LAZRS
    case LAS_ERROR_NO_LAZ_SUPPORT:
        fprintf(stream,
                "Cannot read or write LAZ, the library wasn't "
                "compiled with LAZ support\n");
        break;
#endif

    default:
        LAS_DEBUG_ASSERT_M(0, "Unhandled error kind");
        break;
    }
}
