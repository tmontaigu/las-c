#ifndef LAS_C_SOURCE_H
#define LAS_C_SOURCE_H

#include <stdint.h>
#include <stdio.h>

typedef enum las_seek_from
{
    LAS_SEEK_FROM_START = SEEK_SET,
    LAS_SEEK_FROM_CURRENT = SEEK_CUR,
    LAS_SEEK_FROM_END = SEEK_END,
} las_seek_from_t;

typedef uint64_t (*las_source_read_fn)(void *self, uint64_t n, uint8_t *out_buffer);

// TODO allow seek to return error
typedef int (*las_source_seek_fn)(void *self, int64_t pos, las_seek_from_t from);

// TODO allow tell to return error
typedef uint64_t (*las_source_tell_fn)(void *self);

typedef int (*las_source_eof_fn)(void *self);

typedef int (*las_source_close_fn)(void *self);

typedef struct las_source
{
    void *inner;
    las_source_read_fn read_fn;
    las_source_seek_fn seek_fn;
    las_source_eof_fn eof_fn;
    las_source_tell_fn tell_fn;
    las_source_close_fn close_fn;
} las_source_t;

// TODO delete function for las_source

las_source_t las_source_new_memory(const uint8_t *buffer, uint64_t size);

int las_source_new_file(const char *filename, las_source_t *source);

uint64_t las_source_read(las_source_t *self, uint64_t n, uint8_t *out_buffer);

int las_source_seek(las_source_t *self, int64_t n, las_seek_from_t from);

int las_source_eof(las_source_t *self);

uint64_t las_source_tell(las_source_t *self);

int las_source_close(las_source_t *self);

void las_source_deinit(las_source_t *self);

#endif // LAS_C_SOURCE_H
