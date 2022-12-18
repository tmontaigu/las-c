#ifndef LAS_C_ERROR_H
#define LAS_C_ERROR_H

#include <stdint.h>
#include <stdio.h>

#ifdef WITH_LAZRS
#include <lazrs/lazrs.h>
#endif

#include <las/header.h>

enum las_error_kind_t
{
    LAS_ERROR_OK = 0,
    LAS_ERROR_MEMORY,
    LAS_ERROR_ERRNO,
    LAS_ERROR_UNEXPECTED_EOF,
    LAS_ERROR_INVALID_SIGNATURE,
    LAS_ERROR_INVALID_VERSION,
    LAS_ERROR_INVALID_POINT_FORMAT,
    LAS_ERROR_INVALID_POINT_SIZE,
    LAS_ERROR_MISSING_LASZIP_VLR,
    LAS_ERROR_INCOMPATIBLE_VERSION_AND_FORMAT,
    LAS_ERROR_POINT_COUNT_TOO_HIGH,
    LAS_ERROR_INCOMPATIBLE_POINT_FORMAT,
#ifdef WITH_LAZRS
    LAS_ERROR_LAZRS,
#else
    LAS_ERROR_NO_LAZ_SUPPORT,
#endif
};

typedef enum las_error_kind_t las_error_kind_t;

struct las_error_t
{
    las_error_kind_t kind;
    union
    {
        /// Active for LAS_ERROR_INVALID_POINT_SIZE
        struct
        {
            /// The point size written in the header
            uint16_t point_size;
            /// The point format written in the header
            uint8_t point_format_id;
            /// The minimum point size we expect for the point format
            uint16_t minimum;
        } point_size;
        /// Active for LAS_ERROR_INVALID_VERSION
        las_version_t version;
        /// Active for LAS_ERROR_INVALID_POINT_FORMAT
        uint8_t point_format_id;
        /// Active for LAS_ERROR_INCOMPATIBLE_VERSION_AND_FORMAT
        struct
        {
            las_version_t version;
            uint8_t point_format_id;
        } incompatible;
        /// Active for LAS_ERROR_ERRNO
        int errno_;
        /// Active for LAS_ERROR_POINT_COUNT_TOO_HIGH
        uint64_t point_count;
        /// Active for LAS_ERROR_INVALID_SIGNATURE
        // +1 for the null terminator
        char signature[LAS_SIGNATURE_SIZE + 1];
#ifdef WITH_LAZRS
        Lazrs_Result lazrs;
#endif
    };
};

typedef struct las_error_t las_error_t;

int las_error_is_ok(const las_error_t *self);

int las_error_is_failure(const las_error_t *self);

void las_error_fprintf(const las_error_t *self, FILE *stream);

#endif // LAS_C_ERROR_H
