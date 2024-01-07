#include "private/dest.h"
#include "private/macro.h"

#include <errno.h>
#include <limits.h>

struct las_file_dest_t
{
    FILE *file;
};

typedef struct las_file_dest_t las_file_dest_t;

uint64_t las_file_dest_write(las_file_dest_t *self, const uint8_t *buffer, const uint64_t n)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->file != NULL);
    LAS_DEBUG_ASSERT(buffer != NULL);

    LAS_ASSERT(n <= SIZE_MAX);

    return (uint64_t)fwrite(buffer, sizeof(uint8_t), (size_t)n, self->file);
}

int las_file_dest_seek(las_file_dest_t *self, int64_t pos, const las_seek_from_t from)
{
    LAS_DEBUG_ASSERT(self->file != NULL);

    LAS_ASSERT(pos <= LONG_MAX);
    return fseek(self->file, (long int)pos, (int)from);
}

uint64_t las_file_dest_tell(las_file_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->file != NULL);

    // TODO handle error
    return (uint64_t)ftell(self->file);
}

uint64_t las_file_dest_flush(las_file_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->file != NULL);

    return fflush(self->file) != 0;
}

int las_file_dest_close(las_file_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->file != NULL);

    return fclose(self->file);
}

las_error_t las_file_dest_err_fn(las_file_dest_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(self->file);

    las_error_t las_err = {LAS_ERROR_OK};
    const int is_error = ferror(self->file);

    if (is_error)
    {
        las_err.kind = LAS_ERROR_ERRNO;
        las_err.errno_ = errno;
    }

    return las_err;
}

//=============================================================================

uint64_t las_dest_write(las_dest_t *self, const uint8_t *buffer, const uint64_t n)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->inner != NULL);
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT_NOT_NULL(self->write_fn);

    return self->write_fn(self->inner, buffer, n);
}

int las_dest_seek(las_dest_t *self, int64_t n, const las_seek_from_t from)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->inner != NULL);
    LAS_DEBUG_ASSERT_NOT_NULL(self->seek_fn);

    return self->seek_fn(self->inner, n, from);
}

uint64_t las_dest_tell(las_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->inner != NULL);
    LAS_DEBUG_ASSERT_NOT_NULL(self->tell_fn);

    return self->tell_fn(self->inner);
}

int las_dest_flush(las_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->inner != NULL);
    LAS_DEBUG_ASSERT_NOT_NULL(self->flush_fn);

    return self->flush_fn(self->inner);
}

int las_dest_close(las_dest_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->inner != NULL);
    LAS_DEBUG_ASSERT_NOT_NULL(self->close_fn);

    return self->close_fn(self->inner);
}

las_error_t las_dest_err(las_dest_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(self->inner);
    LAS_DEBUG_ASSERT_NOT_NULL(self->err_fn);

    return self->err_fn(self->inner);
}

void las_dest_deinit(las_dest_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    free(self->inner);
    self->inner = NULL;
}

int las_dest_new_file(const char *filename, las_dest_t *dest)
{
    LAS_DEBUG_ASSERT(filename != NULL);
    LAS_DEBUG_ASSERT(dest != NULL);

    las_file_dest_t *inner = malloc(sizeof(las_file_dest_t));
    if (inner == NULL)
    {
        return 1;
    }

    FILE *file = fopen(filename, "wb");
    if (file == NULL)
    {
        free(inner);
        return 1;
    }

    inner->file = file;
    dest->inner = (void *)inner;
    // The casts are to cast the first param of the fn a void*
    dest->write_fn = (las_dest_write_fn)las_file_dest_write;
    dest->seek_fn = (las_dest_seek_fn)las_file_dest_seek;
    dest->tell_fn = (las_dest_tell_fn)las_file_dest_tell;
    dest->close_fn = (las_dest_close_fn)las_file_dest_close;
    dest->flush_fn = (las_dest_flush_fn)las_file_dest_flush;
    dest->err_fn = (las_dest_err_fn)las_file_dest_err_fn;

    return 0;
}
