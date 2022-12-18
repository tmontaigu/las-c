#include "private/source.h"
#include "private/macro.h"

#include <limits.h>

// Macro this ?
uint64_t uint64_max(uint64_t a, uint64_t b)
{
    return ((a > b) ? a : b);
}

uint64_t uint64_min(uint64_t a, uint64_t b)
{
    return ((a < b) ? a : b);
}

struct las_memory_source_t
{
    const uint8_t *buffer;
    uint64_t size;
    uint64_t pos;
};

typedef struct las_memory_source_t las_memory_source_t;

uint64_t las_memory_source_read(void *vself, uint64_t n, uint8_t *out_buffer)
{
    LAS_DEBUG_ASSERT(vself != NULL);
    LAS_DEBUG_ASSERT(out_buffer != NULL);

    las_memory_source_t *self = (las_memory_source_t *)vself;

    uint64_t bytes_left = self->size - self->pos;
    uint64_t num_to_read = uint64_min(n, bytes_left);

    memcpy(out_buffer, &self->buffer[self->pos], num_to_read);

    self->pos += num_to_read;
    return num_to_read;
}

int las_memory_source_seek(void *vself, int64_t pos, las_seek_from_t from)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_memory_source_t *self = (las_memory_source_t *)vself;

    uint64_t new_pos = self->size;
    switch (from)
    {
    case LAS_SEEK_FROM_START:
        LAS_DEBUG_ASSERT(pos >= 0);
        new_pos = (uint64_t)pos;
        break;
    case LAS_SEEK_FROM_CURRENT:
        LAS_ASSERT_M(0, "todo");
        break;
    case LAS_SEEK_FROM_END:
        if (pos > 0)
        {
            new_pos = self->size;
        }
        else
        {
            LAS_ASSERT_M(0, "todo");
        }
        break;
    default:
        LAS_DEBUG_ASSERT_M(0, "invalid seek from value");
        break;
    }

    if (new_pos >= self->size)
    {
        self->pos = self->size;
    }
    else
    {
        self->pos = new_pos;
    }

    return 0;
}

uint64_t las_memory_source_tell(void *vself)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_memory_source_t *self = (las_memory_source_t *)vself;
    return self->pos;
}

int las_memory_source_eof(void *vself)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_memory_source_t *self = (las_memory_source_t *)vself;
    return self->pos == self->size;
}

struct las_source_file_t
{
    FILE *file;
};

typedef struct las_source_file_t las_source_file_t;

uint64_t las_file_source_read(void *vself, uint64_t n, uint8_t *out_buffer)
{
    LAS_DEBUG_ASSERT(vself != NULL);
    LAS_DEBUG_ASSERT(out_buffer != NULL);

    las_source_file_t *self = (las_source_file_t *)vself;

    size_t read = fread(out_buffer, sizeof(uint8_t), n, self->file);
    return (uint64_t)read;
}

int las_file_source_seek(void *vself, int64_t pos, las_seek_from_t from)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_source_file_t *self = (las_source_file_t *)vself;
    LAS_DEBUG_ASSERT(self->file != NULL);

    LAS_ASSERT(pos <= LONG_MAX);

    return fseek(self->file, (long int)pos, (int)from);
}

uint64_t las_file_source_tell(void *vself)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_source_file_t *self = (las_source_file_t *)vself;
    LAS_DEBUG_ASSERT(self->file != NULL);

    // TODO handle error
    return (uint64_t)ftell(self->file);
}

int las_file_source_eof(void *vself)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_source_file_t *self = (las_source_file_t *)vself;
    LAS_DEBUG_ASSERT(self->file != NULL);

    return feof(self->file);
}

int las_file_source_close(void *vself)
{
    LAS_DEBUG_ASSERT(vself != NULL);

    las_source_file_t *self = (las_source_file_t *)vself;
    LAS_DEBUG_ASSERT(self->file != NULL);

    return fclose(self->file);
}

int las_source_new_file(const char *filename, las_source_t *source)
{
    LAS_DEBUG_ASSERT(filename != NULL);
    LAS_DEBUG_ASSERT(source != NULL);

    //    *source = malloc(sizeof(las_source_t));
    //    LAS_ASSERT_M(source != NULL, "out of memory");

    las_source_file_t *inner = malloc(sizeof(las_source_file_t));
    LAS_ASSERT_M(inner != NULL, "out of memory");

    inner->file = fopen(filename, "rb");

    memset(source, 0, sizeof(las_source_t));
    source->inner = (void *)inner;
    source->read_fn = las_file_source_read;
    source->seek_fn = las_file_source_seek;
    source->tell_fn = las_file_source_tell;
    source->eof_fn = las_file_source_eof;
    source->close_fn = las_file_source_close;

    return inner->file == NULL;
}

las_source_t las_source_new_memory(const uint8_t *buffer, uint64_t size)
{
    LAS_DEBUG_ASSERT(buffer != NULL);
    las_memory_source_t *inner = malloc(sizeof(las_memory_source_t));
    LAS_ASSERT_M(inner != NULL, "out of memory");
    inner->buffer = buffer;
    inner->size = size;
    inner->pos = 0;

    las_source_t source;
    memset(&source, 0, sizeof(las_source_t));
    source.inner = (void *)inner;
    source.read_fn = las_memory_source_read;
    source.seek_fn = las_memory_source_seek;
    source.tell_fn = las_memory_source_tell;
    source.eof_fn = las_memory_source_eof;
    source.close_fn = NULL;

    return source;
}

uint64_t las_source_read(las_source_t *self, uint64_t n, uint8_t *out_buffer)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->read_fn != NULL);

    return (*self->read_fn)(self->inner, n, out_buffer);
}

int las_source_seek(las_source_t *self, int64_t n, las_seek_from_t from)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->seek_fn != NULL);
    return (*self->seek_fn)(self->inner, n, from);
}

int las_source_eof(las_source_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->eof_fn != NULL);

    return (*self->eof_fn)(self->inner);
}

uint64_t las_source_tell(las_source_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(self->tell_fn != NULL);

    return (*self->tell_fn)(self->inner);
}

int las_source_close(las_source_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    if (self->close_fn != NULL)
    {
        return (*self->close_fn)(self->inner);
    }
    return 0;
}

void las_source_deinit(las_source_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    free(self->inner);
    self->inner = NULL;
}