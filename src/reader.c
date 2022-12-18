#include <las/reader.h>

#ifdef WITH_LAZRS
#include <lazrs/lazrs.h>
#endif

#include "private/macro.h"
#include "private/point.h"
#include "private/source.h"

struct las_reader_t
{
    /// Source from where we get the LAS/LAZ data
    las_source_t source;
    /// The header of the LAS/LAZ data we are trying to read
    las_header_t header;
    /// Holds the raw bytes of a point.
    /// Its size is point_size
    uint8_t *point_buffer;
    uint16_t point_size;

#ifdef WITH_LAZRS
    /// Is not null when the input data is LAZ
    /// meaning we should get bytes from the
    /// decompressor and not the source
    Lazrs_SeqLasZipDecompressor *decompressor;
#endif
};

typedef struct las_reader_t las_reader_t;

static inline las_error_t las_reader_fill_point_buffer_from_source(las_reader_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;

    uint64_t n = las_source_read(&self->source, (uint64_t)self->point_size, self->point_buffer);
    if (n < self->point_size)
    {
        if (las_source_eof(&self->source))
        {
            las_err.kind = LAS_ERROR_UNEXPECTED_EOF;
        }
        else
        {
            las_err.kind = LAS_ERROR_ERRNO;
        }
        return las_err;
    }

    return las_err;
}

#ifdef WITH_LAZRS
/// Fills the reader's point buffer with the bytes of a decompressed point
static inline las_error_t las_reader_fill_point_buffer_from_decompressor(las_reader_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->decompressor != NULL);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;
    Lazrs_Result laz_err;

    laz_err = lazrs_seq_laszip_decompressor_decompress_one(
        self->decompressor, self->point_buffer, self->point_size);

    if (laz_err != LAZRS_OK)
    {
        las_err.kind = LAS_ERROR_LAZRS;
        las_err.lazrs = laz_err;
        return las_err;
    }
    return las_err;
}

/// Creates the decompressor that correspond the input data.
static inline las_error_t las_reader_create_decompressor(las_reader_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;
    Lazrs_Result laz_err;
    Lazrs_SeqLasZipDecompressor *decompressor;

    const las_vlr_t *laszip_vlr = las_header_find_laszip_vlr(&self->header);
    if (laszip_vlr == NULL)
    {
        las_err.kind = LAS_ERROR_MISSING_LASZIP_VLR;
        return las_err;
    }

    Lazrs_DecompressorParams params;
    params.source_type = LAZRS_SOURCE_CUSTOM;
    params.source.custom.user_data = &self->source;
    // We cast to change the las_source_t* to void*
    params.source.custom.read_fn = (uint64_t(*)(void *, uint64_t, uint8_t *))las_source_read;
    // We cast to change the las_source_t* to void*
    // _AND_ the las_seek_from_t to an int which should be fine
    // as enums are ints, and our enum uses the same values
    params.source.custom.seek_fn = (int (*)(void *, int64_t, int))las_source_seek;
    // We cast to change the las_source_t* to void*
    params.source.custom.tell_fn = (uint64_t(*)(void *))las_source_tell;
    params.source_offset = self->header.offset_to_point_data;
    params.laszip_vlr.data = laszip_vlr->data;
    params.laszip_vlr.len = (uintptr_t)laszip_vlr->data_size;

    laz_err = lazrs_seq_laszip_decompressor_new(params, &decompressor);
    if (laz_err != LAZRS_OK)
    {
        las_err.kind = LAS_ERROR_LAZRS;
        las_err.lazrs = laz_err;
        return las_err;
    }

    // We are removing the laszip vlr
    // as it is an implementation detail,
    las_vlr_t *new_vlrs = malloc(sizeof(las_vlr_t) * (self->header.number_of_vlrs - 1));
    if (new_vlrs == NULL)
    {
        las_err.kind = LAS_ERROR_MEMORY;
        return las_err;
    }

    // Move the vlrs into a new array
    // removing the laszip one
    uint32_t j = 0;
    for (uint32_t i = 0; i < self->header.number_of_vlrs; ++i)
    {
        const las_vlr_t *source_vlr = &self->header.vlrs[i];
        las_vlr_t *dest_vlr = &new_vlrs[j];
        if (source_vlr == laszip_vlr)
        {
            continue;
        }
        *dest_vlr = *source_vlr;
        j++;
    }
    free(laszip_vlr->data);
    free(self->header.vlrs);
    self->header.vlrs = new_vlrs;
    self->header.number_of_vlrs--;

    self->decompressor = decompressor;
    return las_err;
}
#endif

static void las_reader_deinit(las_reader_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    if (self->source.inner != NULL)
    {
        las_source_close(&self->source);
        las_source_deinit(&self->source);
    }

    if (self->point_buffer != NULL)
    {
        free(self->point_buffer);
        self->point_buffer = NULL;
    }

#ifdef WITH_LAZRS
    if (self->decompressor != NULL)
    {
        lazrs_seq_laszip_decompressor_delete(self->decompressor);
        self->decompressor = NULL;
    }
#endif

    las_header_deinit(&self->header);
}

las_error_t las_reader_read_next_raw(las_reader_t *self, las_raw_point_t *point)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(point != NULL);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;

#ifdef WITH_LAZRS
    if (self->decompressor)
    {
        las_err = las_reader_fill_point_buffer_from_decompressor(self);
    }
    else
    {
        las_err = las_reader_fill_point_buffer_from_source(self);
    }
#else
    las_err = las_reader_fill_point_buffer_from_source(self);
#endif

    if (self->header.point_format.id <= 5)
    {
        las_raw_point_10_from_buffer(
            self->point_buffer, self->header.point_format, &point->point10);
    }
    else
    {
        las_raw_point_14_from_buffer(
            self->point_buffer, self->header.point_format, &point->point14);
    }

    return las_err;
}

const las_header_t *las_reader_header(const las_reader_t *reader)
{
    LAS_DEBUG_ASSERT(reader != NULL);
    return &reader->header;
}

las_error_t las_reader_from_source(las_source_t source, las_reader_t **out_reader)
{
    LAS_DEBUG_ASSERT(out_reader != NULL);

    int r;
    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;

    las_reader_t *reader = malloc(sizeof(las_reader_t));
    if (reader == NULL)
    {
        las_err.kind = LAS_ERROR_MEMORY;
        goto out;
    }
    memset(reader, 0, sizeof(las_reader_t));

    reader->source = source;

    las_err = las_header_read_from(&reader->source, &reader->header);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    las_err = las_header_validate(&reader->header);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    int is_compressed = reader->header.point_format.is_compressed;
    reader->point_size = las_point_format_point_size(reader->header.point_format);

    r = las_source_seek(
        &reader->source, (int64_t)reader->header.offset_to_point_data, LAS_SEEK_FROM_START);
    if (r != 0)
    {
        las_err.kind = LAS_ERROR_ERRNO;
        goto out;
    }

    if (is_compressed)
    {
#ifndef WITH_LAZRS
        las_err.kind = LAS_ERROR_NO_LAZ_SUPPORT;
#else
        las_err = las_reader_create_decompressor(reader);
#endif
    }

    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    reader->point_buffer = malloc(sizeof(uint8_t) * reader->point_size);
    LAS_ASSERT(reader->point_buffer != NULL);
    memset(reader->point_buffer, 0, sizeof(uint8_t) * reader->point_size);

out:
    if (las_error_is_failure(&las_err))
    {
        if (reader)
        {
            las_reader_deinit(reader);
            free(reader);
        }
        *out_reader = NULL;
    }
    else
    {
        *out_reader = reader;
    }
    return las_err;
}

las_error_t las_reader_open_buffer(const uint8_t *buffer, uint64_t size, las_reader_t **out_reader)
{
    las_source_t source = las_source_new_memory(buffer, size);
    return las_reader_from_source(source, out_reader);
}

las_error_t las_reader_open_file_path(const char *file_path, las_reader_t **out_reader)
{
    LAS_DEBUG_ASSERT(out_reader != NULL);
    LAS_DEBUG_ASSERT(file_path != NULL);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;
    *out_reader = NULL;

    las_source_t source;
    int r = las_source_new_file(file_path, &source);

    if (r != 0)
    {
        las_err.kind = LAS_ERROR_ERRNO;
        return las_err;
    }

    return las_reader_from_source(source, out_reader);
}

void las_reader_destroy(las_reader_t *self)
{
    if (self == NULL)
    {
        return;
    }

    las_reader_deinit(self);

    free(self);
}