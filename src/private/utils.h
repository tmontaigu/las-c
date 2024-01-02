#ifndef LAS_C_PRIV_UTILS_H
#define LAS_C_PRIV_UTILS_H

#include <stdint.h>
#include <string.h>

/// Simple in memory byte buffer reader
///
/// Simple "reader" to read data from
/// an in memory byte buffer.
///
/// It does not do any size checking
/// so it has to be used correctly in order
/// no cause out of bound reads.
typedef struct buffer_reader_t
{
    const uint8_t *ptr;
} buffer_reader_t;

static inline void read_into(buffer_reader_t *self, void *dst, uint64_t count)
{
    memcpy(dst, self->ptr, count);
    self->ptr += count;
}

/// Defines a specialized read_into function for the given type
///
/// The function name will be suffixed with the type name
/// eg: define_read_into(uint8_t) -> `read_into_uint8_t`
///
/// The defined function is only meant to read one element.
#define define_read_into(type)                                                                     \
    static inline void read_into_##type(buffer_reader_t *self, type *dst)                          \
    {                                                                                              \
        read_into(self, dst, sizeof(type));                                                        \
    }

define_read_into(int32_t)
define_read_into(uint8_t)
define_read_into(uint16_t)
define_read_into(uint32_t)
define_read_into(uint64_t)
define_read_into(float)
define_read_into(double)

/// Definition of a generic read_into.
///
/// This helps avoiding too much typing and potential errors.
///
/// # eg
///
/// We go from
//// ```C
//// uint32_t a;
//// read_into(&rdr, &a, sizeof(uint32_t);
/// ```
/// to
//// ```C
//// uint32_t a;
//// read_intog(&rdr, &a);
/// ```
#define read_intog(reader, dst)                                                                    \
    _Generic((dst),                        \
        int32_t* : read_into_int32_t,    \
        uint8_t* : read_into_uint8_t,    \
        uint16_t* : read_into_uint16_t,  \
        uint32_t* : read_into_uint32_t,  \
        uint64_t* : read_into_uint64_t,  \
        float* : read_into_float,        \
        double* : read_into_double       \
    )(reader, dst)

typedef struct buffer_writer_t
{
    uint8_t *ptr;
} buffer_writer_t;

static inline void write_into(buffer_writer_t *self, const void *src, uint64_t count)
{
    memcpy(self->ptr, src, count);
    self->ptr += count;
}

/// Defines a specialized write_into function for the given type
///
/// The function name will be suffixed with the type name
/// eg: define_write_into(uint8_t) -> `write_into_uint8_t`
///
/// The defined function is only meant to read one element.
#define define_write_into(type)                                                                    \
    static inline void write_into_##type(buffer_writer_t *self, const type *src)                   \
    {                                                                                              \
        write_into(self, src, sizeof(type));                                                       \
    }

define_write_into(int32_t)
define_write_into(uint8_t)
define_write_into(uint16_t)
define_write_into(uint32_t)
define_write_into(uint64_t)
define_write_into(float)
define_write_into(double)

/// Definition of a generic write_into.
///
/// This helps avoiding too much typing and potential errors.
///
/// # eg
///
/// We go from
//// ```C
//// uint32_t a;
//// write_intog(&wrt, (uint32_t) 13, sizeof(uint32_t);
/// ```
/// to
//// ```C
//// uint32_t a;
//// write_intog(&rdr, (uint32_t) 13);
/// ```
#define write_intog(writer, src)                                                                   \
    _Generic((src),                       \
        const int32_t* : write_into_int32_t,    \
        const uint8_t* : write_into_uint8_t,    \
        const uint16_t* : write_into_uint16_t,  \
        const uint32_t* : write_into_uint32_t,  \
        const uint64_t* : write_into_uint64_t,  \
        const float* : write_into_float,        \
        const double* : write_into_double       \
    )(writer, src)

#endif // LAS_C_PRIV_UTILS_H
