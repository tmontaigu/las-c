#include "private/dest.h"
#include "private/header.h"
#include "private/macro.h"
#include "private/point.h"

#ifdef WITH_LAZRS
#include <lazrs/lazrs.h>
#endif

typedef struct las_writer
{
    las_header_t *header;
    las_dest_t *dest;
    /// Holds the raw bytes of a point.
    /// Its size is point_size * num_points_in_buffer
    uint8_t *point_buffer;
    uint64_t num_points_in_buffer;
    uint16_t point_size;

#ifdef WITH_LAZRS
    /// Is not null when we are writing points as compressed
    /// meaning we should write the bytes into the `compressor`
    /// and not the `dest`
    Lazrs_LasZipCompressor *compressor;
#endif
} las_writer_t;

las_error_t
las_writer_open_file_path(const char *file_path, las_header_t *header, las_writer_t **out_writer)
{
    LAS_DEBUG_ASSERT(file_path != NULL);
    LAS_DEBUG_ASSERT(header != NULL);
    LAS_DEBUG_ASSERT(out_writer != NULL);

    las_error_t las_err = {LAS_ERROR_OK};
    las_writer_t *writer = NULL;
    uint8_t *point_buffer = NULL;
    las_dest_t *dest = NULL;
#ifdef WITH_LAZRS
    Lazrs_LasZipCompressor *compressor = NULL;
#endif

    las_err = las_header_validate_for_writing(header);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    writer = calloc(1, sizeof(las_writer_t));
    if (writer == NULL)
    {
        las_err.kind = LAS_ERROR_MEMORY;
        goto out;
    }

    const uint16_t point_size = las_point_format_point_size(header->point_format);
    point_buffer = malloc(sizeof(uint8_t) * point_size);
    if (point_buffer == NULL)
    {
        las_err.kind = LAS_ERROR_MEMORY;
        goto out;
    }
    writer->num_points_in_buffer = 1;

    dest = calloc(1, sizeof(las_dest_t));
    if (dest == NULL)
    {
        las_err.kind = LAS_ERROR_MEMORY;
        goto out;
    }

    if (las_dest_new_file(file_path, dest) != 0)
    {
        las_err.kind = LAS_ERROR_ERRNO;
        goto out;
    }

    int should_compress = 0;
    const char *dot_pos = strrchr(file_path, '.');
    if (dot_pos != NULL && (strcmp(dot_pos, ".laz") == 0 || strcmp(dot_pos, ".LAZ") == 0))
    {
        should_compress = 1;
    }
    if (should_compress)
    {
#ifdef WITH_LAZRS
        Lazrs_CompressorParams params;
        params.dest_type = LAZRS_DEST_CUSTOM;
        params.dest.custom.user_data = (void *)dest;
        params.dest.custom.write_fn =
            (uint64_t(*)(void *, const uint8_t *, uint64_t))las_dest_write;
        params.dest.custom.flush_fn = (int (*)(void *))las_dest_flush;
        params.dest.custom.seek_fn = (int (*)(void *, int64_t, int))las_dest_seek;
        params.dest.custom.tell_fn = (uint64_t(*)(void *))las_dest_tell;
        params.point_format_id = header->point_format.id;
        uint16_t num_extra_bytes = header->point_format.num_extra_bytes;
        params.num_extra_bytes = num_extra_bytes;

        Lazrs_Result r = lazrs_compressor_new_for_point_format(params, true /* prefer_parallel */, &compressor);
        if (r != LAZRS_OK)
        {
            las_err.kind = LAS_ERROR_LAZRS;
            las_err.lazrs = r;
            goto out;
        }

        uint16_t len = lazrs_compressor_laszip_vlr_size(compressor);
        LAS_ASSERT(len != 0);
        uint8_t *laszip_vlr_data = malloc(sizeof(uint8_t) * len);
        if (laszip_vlr_data == NULL)
        {
            las_err.kind = LAS_ERROR_MEMORY;
            goto out;
        }
        memset(laszip_vlr_data, 0, sizeof(uint8_t) * len);

        r = lazrs_compressor_laszip_vlr_data(compressor, laszip_vlr_data, len);
        if (r != LAZRS_OK)
        {
            las_err.kind = LAS_ERROR_LAZRS;
            las_err.lazrs = r;
            goto out;
        }

        uint32_t new_num_vlrs = header->number_of_vlrs + 1;
        las_vlr_t *new_vlrs = malloc(sizeof(las_vlr_t) * new_num_vlrs);
        if (new_vlrs == NULL)
        {
            free(laszip_vlr_data);
            las_err.kind = LAS_ERROR_MEMORY;
            goto out;
        }

        for (uint32_t i = 0; i < header->number_of_vlrs; ++i)
        {
            new_vlrs[i] = header->vlrs[i];
        }

        las_vlr_t *laszip_vlr = &new_vlrs[new_num_vlrs - 1];
        laszip_vlr->data_size = len;
        laszip_vlr->data = laszip_vlr_data;
        laszip_vlr->record_id = 22204;
        strncpy(laszip_vlr->user_id, "laszip encoded", LAS_VLR_USER_ID_SIZE);
        strncpy(laszip_vlr->description, "https://laszip.org", LAS_VLR_DESCRIPTION_SIZE);

        // we can simply free the array, we moved/stole the data ptrs
        if (header->vlrs != NULL)
        {
            free(header->vlrs);
        }
        header->vlrs = new_vlrs;
        header->number_of_vlrs = new_num_vlrs;
#else
        las_err.kind = LAS_ERROR_NO_LAZ_SUPPORT;
        goto out;
#endif
    }

    // Reset some header fields
    header->point_count = 0;
    memset(header->number_of_points_by_return,
           0,
           sizeof(uint64_t) * LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE);

    las_err = las_header_write_to(header, dest);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

out:
    if (las_error_is_failure(&las_err) || writer == NULL)
    {
        las_header_delete(header);
        if (point_buffer != NULL)
        {
            free(point_buffer);
        }
        if (dest != NULL)
        {
            // Opening was successful
            las_dest_close(dest);
            free(dest);
        }
        if (writer != NULL)
        {
            free(writer);
        }
    }
    else
    {
        writer->point_size = point_size;
        writer->point_buffer = point_buffer;
        writer->dest = dest;
        writer->header = header;
#ifdef WITH_LAZRS
        writer->compressor = compressor;
#endif
        *out_writer = writer;
    }
    return las_err;
}

las_error_t las_writer_write_raw_point(las_writer_t *self, const las_raw_point_t *point)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(point);

    las_error_t las_err = {LAS_ERROR_OK};

    if (point->point_format_id != self->header->point_format.id)
    {
        las_err.kind = LAS_ERROR_INCOMPATIBLE_POINT_FORMAT;
        return las_err;
    }

    const uint64_t max_point_count_allowed =
        (self->header->version.major < 4) ? UINT32_MAX : UINT64_MAX;
    if (self->header->point_count == max_point_count_allowed)
    {
        las_err.kind = LAS_ERROR_POINT_COUNT_TOO_HIGH;
        las_err.point_count = self->header->point_count;
        return las_err;
    }

    if (self->header->point_format.id <= 5)
    {
        las_raw_point_10_to_buffer(&point->point10, self->header->point_format, self->point_buffer);
        const int max_value = LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE - 1;
        self->header->number_of_points_by_return[point->point10.return_number & max_value]++;
    }
    else
    {
        las_raw_point_14_to_buffer(&point->point14, self->header->point_format, self->point_buffer);
        const int max_value = LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE - 1;
        self->header->number_of_points_by_return[point->point14.return_number & max_value]++;
    }

#ifdef WITH_LAZRS
    if (self->compressor != NULL)
    {
        Lazrs_Result r =
            lazrs_compressor_compress_one(self->compressor, self->point_buffer, self->point_size);
        if (r != LAZRS_OK)
        {
            las_err.kind = LAS_ERROR_LAZRS;
            las_err.lazrs = r;
        }
    }
    else
    {
        uint64_t n = las_dest_write(self->dest, self->point_buffer, self->point_size);
        if (n < self->point_size)
        {
            las_err = las_dest_err(self->dest);
        }
    }
#else
    uint64_t n = las_dest_write(self->dest, self->point_buffer, self->point_size);
    if (n < self->point_size)
    {
        las_err = las_dest_err(self->dest);
    }
#endif

    self->header->point_count++;

    return las_err;
}

las_error_t las_writer_write_many_raw_points(las_writer_t *self,
                                             const las_raw_point_t *points,
                                             const uint64_t num_points)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(points);

    las_error_t las_err = {LAS_ERROR_OK};

    for (uint64_t i = 0; i < num_points; ++i)
    {
        if (points[i].point_format_id != self->header->point_format.id)
        {
            las_err.kind = LAS_ERROR_INCOMPATIBLE_POINT_FORMAT;
            return las_err;
        }
    }

    const uint64_t max_point_count_allowed =
        (self->header->version.major < 4) ? UINT32_MAX : UINT64_MAX;
    if (self->header->point_count + num_points == max_point_count_allowed)
    {
        las_err.kind = LAS_ERROR_POINT_COUNT_TOO_HIGH;
        las_err.point_count = self->header->point_count;
        return las_err;
    }

    // Resize internal buffer if needed
    if (self->num_points_in_buffer < num_points)
    {
        uint8_t *buffer = realloc(self->point_buffer, self->point_size * num_points);
        if (buffer == NULL)
        {
            las_err.kind = LAS_ERROR_MEMORY;
            return las_err;
        }
        self->point_buffer = buffer;
        self->num_points_in_buffer = num_points;
    }

    // Write the points to internal buffer
    uint8_t *buffer = self->point_buffer;
    if (self->header->point_format.id <= 5)
    {
        for (uint64_t i = 0; i < num_points; ++i)
        {
            las_raw_point_10_to_buffer(&points[i].point10, self->header->point_format, buffer);
            const int max_value = LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE - 1;
            self->header->number_of_points_by_return[points[i].point10.return_number % max_value]++;
            buffer += self->point_size;
            LAS_DEBUG_ASSERT(buffer <=
                             self->point_buffer + (self->point_size * self->num_points_in_buffer));
        }
    }
    else
    {
        for (uint64_t i = 0; i < num_points; ++i)
        {
            las_raw_point_14_to_buffer(&points[i].point14, self->header->point_format, buffer);
            const int max_value = LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE - 1;
            self->header->number_of_points_by_return[points[i].point14.return_number % max_value]++;
            buffer += self->point_size;
            LAS_DEBUG_ASSERT(buffer <=
                             self->point_buffer + (self->point_size * self->num_points_in_buffer));
        }
    }

#ifdef WITH_LAZRS
    if (self->compressor != NULL)
    {
        const Lazrs_Result r = lazrs_compressor_compress_many(
            self->compressor, self->point_buffer, self->point_size * num_points);
        if (r != LAZRS_OK)
        {
            las_err.kind = LAS_ERROR_LAZRS;
            las_err.lazrs = r;
        }
    }
    else
    {
        const uint64_t n =
            las_dest_write(self->dest, self->point_buffer, self->point_size * num_points);
        if (n < self->point_size * num_points)
        {
            las_err = las_dest_err(self->dest);
        }
    }
#else  // WITH_LAZRS
    const uint64_t n =
        las_dest_write(self->dest, self->point_buffer, self->point_size * num_points);
    if (n < self->point_size * num_points)
    {
        las_err = las_dest_err(self->dest);
    }
#endif // WITH_LAZRS

    self->header->point_count += num_points;

    return las_err;
}

static las_error_t las_writer_close(las_writer_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    las_error_t err = {LAS_ERROR_OK};

#ifdef WITH_LAZRS
    if (self->compressor != NULL)
    {
        Lazrs_Result r = lazrs_compressor_done(self->compressor);
        if (r != LAZRS_OK)
        {
            las_error_t err;
            err.kind = LAS_ERROR_LAZRS;
            err.lazrs = r;
            return err;
        }
    }
    self->header->point_format.id |= (2 << 7);
#endif

    if (las_dest_seek(self->dest, 0, LAS_SEEK_FROM_START))
    {
        return las_dest_err(self->dest);
    }

    err = las_header_write_to(self->header, self->dest);
    if (las_error_is_failure(&err))
    {
        return err;
    }

#ifdef WITH_LAZRS
    self->header->point_format.id &= ~(2 << 7);
#endif

    return err;
}

void las_writer_deinit(las_writer_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    if (self->header != NULL)
    {
        las_writer_close(self);
        las_header_delete(self->header);
        self->header = NULL;
    }

    if (self->dest != NULL)
    {
        las_dest_close(self->dest);
        las_dest_deinit(self->dest);
        free(self->dest);
        self->dest = NULL;
    }

    if (self->point_buffer != NULL)
    {
        free(self->point_buffer);
        self->point_buffer = NULL;
    }

#ifdef WITH_LAZRS
    if (self->compressor != NULL)
    {
        lazrs_compressor_delete(self->compressor);
        self->compressor = NULL;
    }
#endif
}

void las_writer_delete(las_writer_t *self)
{
    if (self == NULL)
    {
        return;
    }

    las_writer_deinit(self);
    free(self);
}
