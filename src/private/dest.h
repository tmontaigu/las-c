#ifndef LAS_C_DEST_H
#define LAS_C_DEST_H

#include "source.h"
#include <las/error.h>

typedef uint64_t (*las_dest_write_fn)(void *self, const uint8_t *buffer, uint64_t n);

// TODO allow seek to return error
typedef int (*las_dest_seek_fn)(void *self, int64_t pos, las_seek_from_t from);

// TODO allow tell to return error
typedef uint64_t (*las_dest_tell_fn)(void *self);

typedef int (*las_dest_close_fn)(void *self);

typedef int (*las_dest_flush_fn)(void *self);

typedef las_error_t (*las_dest_err_fn)(void *self);

struct las_dest_t
{
    void *inner;
    las_dest_write_fn write_fn;
    las_dest_seek_fn seek_fn;
    las_dest_tell_fn tell_fn;
    las_dest_close_fn close_fn;
    las_dest_flush_fn flush_fn;
    las_dest_err_fn err_fn;
};

typedef struct las_dest_t las_dest_t;

// TODO delete function for las_dest

int las_dest_new_file(const char *filename, las_dest_t *dest);

uint64_t las_dest_write(las_dest_t *self, const uint8_t *buffer, uint64_t n);

int las_dest_seek(las_dest_t *self, int64_t n, las_seek_from_t from);

uint64_t las_dest_tell(las_dest_t *self);

int las_dest_flush(las_dest_t *self);

las_error_t las_dest_err(las_dest_t *self);

void las_dest_deinit(las_dest_t *self);

int las_dest_close(las_dest_t *self);
#endif // LAS_C_DEST_H
