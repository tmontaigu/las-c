#include "private/point.h"
#include "private/macro.h"
#include "private/source.h"
#include "private/utils.h"

#include "las/header.h"

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#define BASE_POINT10_SIZE 20
// remove the gps for `las_point_standard_size` to work
#define BASE_POINT14_SIZE (30 - sizeof(double))

static int has_gps_time(uint8_t point_format_id)
{
    return point_format_id == 3 || point_format_id == 1 || point_format_id >= 6;
}

static int has_rgb(uint8_t point_format_id)
{
    return point_format_id == 3 || point_format_id == 2 || point_format_id == 5 ||
           point_format_id == 7 || point_format_id == 8 || point_format_id == 10;
}

static int has_nir(uint8_t point_format_id)
{
    return point_format_id == 8 || point_format_id == 10;
}

static int has_waveform(uint8_t point_format_id)
{
    return point_format_id == 4 || point_format_id == 5 || point_format_id == 9 ||
           point_format_id == 10;
}

uint16_t las_point_standard_size(uint8_t format_id)
{
    LAS_ASSERT(format_id <= 10);

    uint16_t size;
    if (format_id <= 5)
    {
        size = BASE_POINT10_SIZE;
    }
    else
    {
        size = BASE_POINT14_SIZE;
    }

    if (has_rgb(format_id))
    {
        size += sizeof(uint16_t) * 3;
    }

    if (has_gps_time(format_id))
    {
        size += sizeof(double);
    }

    if (has_nir(format_id))
    {
        size += sizeof(uint16_t);
    }

    if (has_waveform(format_id))
    {
        size += LAS_WAVE_PACKET_SIZE;
    }

    return size;
}

bool las_wave_packet_eq(const las_wave_packet_t *lhs, const las_wave_packet_t *rhs)
{
    return lhs->descriptor_index == rhs->descriptor_index &&
           lhs->byte_offset_to_data == rhs->byte_offset_to_data &&
           lhs->size_in_bytes == rhs->size_in_bytes &&
           lhs->return_point_waveform_location == rhs->return_point_waveform_location &&
           lhs->xt == rhs->xt && lhs->yt == rhs->yt && lhs->zt == rhs->zt;
}

void las_raw_point_prepare(las_raw_point_t *point, const las_point_format_t point_format)
{
    LAS_DEBUG_ASSERT(point != NULL);

    // TODO the way this is currently implemented will
    //  create memory leak of the extra bytes array of point already has one

    memset(point, 0, sizeof(las_raw_point_t));

    uint8_t *extra_bytes = NULL;
    if (point_format.num_extra_bytes != 0)
    {
        extra_bytes = malloc(sizeof(uint8_t) * point_format.num_extra_bytes);
        LAS_ASSERT_M(extra_bytes != NULL, "out of memory");
        memset(extra_bytes, 0, sizeof(uint8_t) * point_format.num_extra_bytes);
    }
    point->point_format_id = point_format.id;

    if (point_format.id <= 5)
    {
        point->point10.num_extra_bytes = point_format.num_extra_bytes;
        point->point10.extra_bytes = extra_bytes;
    }
    else
    {
        point->point14.num_extra_bytes = point_format.num_extra_bytes;
        point->point14.extra_bytes = extra_bytes;
    }
}

void las_raw_point_10_deinit(las_raw_point_10_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    if (self->extra_bytes != NULL)
    {
        free(self->extra_bytes);
        self->extra_bytes = NULL;
        self->num_extra_bytes = 0;
    }
}

void las_raw_point_14_deinit(las_raw_point_14_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    if (self->extra_bytes != NULL)
    {
        free(self->extra_bytes);
        self->extra_bytes = NULL;
        self->num_extra_bytes = 0;
    }
}

void las_raw_point_deinit(las_raw_point_t *point)
{
    LAS_DEBUG_ASSERT(point != NULL);

    if (point->point_format_id <= 5)
    {
        las_raw_point_10_deinit(&point->point10);
    }
    else
    {
        las_raw_point_14_deinit(&point->point14);
    }
}

void las_wave_packet_from_buffer(const uint8_t *buffer, las_wave_packet_t *wave_packet)
{
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT(wave_packet != NULL);

    buffer_reader_t rdr = {buffer};

    read_intog(&rdr, &wave_packet->descriptor_index);
    read_intog(&rdr, &wave_packet->byte_offset_to_data);
    read_intog(&rdr, &wave_packet->size_in_bytes);
    read_intog(&rdr, &wave_packet->return_point_waveform_location);
    read_intog(&rdr, &wave_packet->xt);
    read_intog(&rdr, &wave_packet->yt);
    read_intog(&rdr, &wave_packet->zt);
}

void las_wave_packet_to_buffer(const las_wave_packet_t *wave_packet, uint8_t *buffer)
{
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT(wave_packet != NULL);

    buffer_writer_t wtr = {buffer};

    write_intog(&wtr, &wave_packet->descriptor_index);
    write_intog(&wtr, &wave_packet->byte_offset_to_data);
    write_intog(&wtr, &wave_packet->size_in_bytes);
    write_intog(&wtr, &wave_packet->return_point_waveform_location);
    write_intog(&wtr, &wave_packet->xt);
    write_intog(&wtr, &wave_packet->yt);
    write_intog(&wtr, &wave_packet->zt);
}

void las_raw_point_10_from_buffer(const uint8_t *buffer,
                                  const las_point_format_t point_format,
                                  las_raw_point_10_t *point10)
{
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT(point10 != NULL);
    LAS_DEBUG_ASSERT(point_format.id <= 5);

    buffer_reader_t rdr = {buffer};

    read_intog(&rdr, &point10->x);
    read_intog(&rdr, &point10->y);
    read_intog(&rdr, &point10->z);

    read_intog(&rdr, &point10->intensity);

    // clang-format off
    uint8_t packed;
    read_intog(&rdr, &packed);
    point10->return_number       = packed  & 0b00000111;
    point10->number_of_returns   = (packed & 0b00111000) >> 3;
    point10->scan_angle_rank     = (packed & 0b01000000) >> 6;
    point10->edge_of_flight_line = (packed & 0b10000000) >> 7;

    read_intog(&rdr, &packed);
    point10->classification = packed  & 0b00011111;
    point10->synthetic      = (packed & 0b00100000) >> 5;
    point10->key_point      = (packed & 0b01000000) >> 6;
    point10->withheld       = (packed & 0b10000000) >> 7;
    // clang-format on

    read_intog(&rdr, &point10->scan_angle_rank);
    read_intog(&rdr, &point10->user_data);
    read_intog(&rdr, &point10->point_source_id);

    if (has_gps_time(point_format.id))
    {
        read_intog(&rdr, &point10->gps_time);
    }

    if (has_rgb(point_format.id))
    {
        read_intog(&rdr, &point10->red);
        read_intog(&rdr, &point10->green);
        read_intog(&rdr, &point10->blue);
    }

    if (has_waveform(point_format.id))
    {
        las_wave_packet_from_buffer(rdr.ptr, &point10->wave_packet);
        rdr.ptr += LAS_WAVE_PACKET_SIZE;
    }

    LAS_DEBUG_ASSERT((rdr.ptr - buffer) == las_point_standard_size(point_format.id));
    if (point10->extra_bytes != NULL)
    {
        LAS_DEBUG_ASSERT(point10->num_extra_bytes == point_format.num_extra_bytes);
        read_into(&rdr, point10->extra_bytes, point10->num_extra_bytes);
    }

    // LAS_DEBUG_ASSERT((rdr.ptr - buffer) == (uint64_t)header->point_size);
}

void las_raw_point_10_to_buffer(const las_raw_point_10_t *point10,
                                const las_point_format_t point_format,
                                uint8_t *buffer)
{
    LAS_DEBUG_ASSERT_NOT_NULL(point10);
    LAS_DEBUG_ASSERT_NOT_NULL(buffer);

    buffer_writer_t wtr_s = {buffer};
    buffer_writer_t *wtr = &wtr_s;

    write_intog(wtr, &point10->x);
    write_intog(wtr, &point10->y);
    write_intog(wtr, &point10->z);

    write_intog(wtr, &point10->intensity);

    // clang-format off
    uint8_t packed = 0;
    packed |= point10->return_number;
    packed |= point10->number_of_returns << 3;
    packed |= point10->scan_angle_rank << 6;
    packed |= point10->edge_of_flight_line << 7;
    write_intog(wtr, (const uint8_t*)&packed);

    packed = 0;
    packed |= point10->classification;
    packed |= point10->synthetic << 5;
    packed |= point10->key_point << 6;
    packed |= point10->withheld << 7;
    write_intog(wtr, (const uint8_t*)&packed);
    // clang-format on

    write_intog(wtr, &point10->scan_angle_rank);
    write_intog(wtr, &point10->user_data);
    write_intog(wtr, &point10->point_source_id);

    if (has_gps_time(point_format.id))
    {
        write_intog(wtr, &point10->gps_time);
    }

    if (has_rgb(point_format.id))
    {
        write_intog(wtr, &point10->red);
        write_intog(wtr, &point10->green);
        write_intog(wtr, &point10->blue);
    }

    if (has_waveform(point_format.id))
    {
        las_wave_packet_to_buffer(&point10->wave_packet, wtr->ptr);
        wtr->ptr += LAS_WAVE_PACKET_SIZE;
    }

    LAS_DEBUG_ASSERT((wtr->ptr - buffer) == las_point_standard_size(point_format.id));
    if (point10->extra_bytes != NULL)
    {
        LAS_DEBUG_ASSERT(point10->num_extra_bytes == point_format.num_extra_bytes);
        write_into(wtr, point10->extra_bytes, point10->num_extra_bytes);
    }

    LAS_DEBUG_ASSERT((wtr->ptr - buffer) == (uint64_t)las_point_format_point_size(point_format));
}

void las_raw_point_14_from_buffer(const uint8_t *buffer,
                                  const las_point_format_t point_format,
                                  las_raw_point_14_t *point14)
{
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT(point14 != NULL);
    LAS_DEBUG_ASSERT(point_format.id >= 6);

    buffer_reader_t rdr = {buffer};

    read_intog(&rdr, &point14->x);
    read_intog(&rdr, &point14->y);
    read_intog(&rdr, &point14->z);

    read_intog(&rdr, &point14->intensity);

    // clang-format off
    uint8_t packed;
    read_intog(&rdr, &packed);
    point14->return_number       = packed & 0b00001111;
    point14->number_of_returns   = (packed & 0b11110000) >> 4;

    read_intog(&rdr, &packed);
    point14->synthetic           = packed & 0b00000001;
    point14->key_point           = (packed & 0b00000010) >> 1;
    point14->withheld            = (packed & 0b00000100) >> 2;
    point14->overlap             = (packed & 0b00001000) >> 3;
    point14->scanner_channel     = (packed & 0b00110000) >> 4;
    point14->scan_direction_flag = (packed & 0b01000000) >> 6;
    point14->edge_of_flight_line = (packed & 0b10000000) >> 7;
    // clang-format on

    read_intog(&rdr, &point14->classification);
    read_intog(&rdr, &point14->user_data);
    read_intog(&rdr, &point14->scan_angle);
    read_intog(&rdr, &point14->point_source_id);

    if (has_gps_time(point_format.id))
    {
        read_intog(&rdr, &point14->gps_time);
    }

    if (has_rgb(point_format.id))
    {
        read_intog(&rdr, &point14->red);
        read_intog(&rdr, &point14->green);
        read_intog(&rdr, &point14->blue);
    }

    if (has_nir(point_format.id))
    {
        read_intog(&rdr, &point14->nir);
    }

    if (has_waveform(point_format.id))
    {
        las_wave_packet_from_buffer(rdr.ptr, &point14->wave_packet);
        rdr.ptr += LAS_WAVE_PACKET_SIZE;
    }

    LAS_DEBUG_ASSERT((rdr.ptr - buffer) == las_point_standard_size(point_format.id));
    if (point14->extra_bytes != NULL)
    {
        LAS_DEBUG_ASSERT(point14->num_extra_bytes == point_format.num_extra_bytes);
        read_into(&rdr, point14->extra_bytes, point14->num_extra_bytes);
    }

    LAS_DEBUG_ASSERT((rdr.ptr - buffer) == (uint64_t)las_point_format_point_size(point_format));
}

void las_raw_point_14_to_buffer(const las_raw_point_14_t *point14,
                                const las_point_format_t point_format,
                                uint8_t *buffer)

{
    LAS_DEBUG_ASSERT(buffer != NULL);
    LAS_DEBUG_ASSERT(point14 != NULL);
    LAS_DEBUG_ASSERT(point_format.id >= 6);

    buffer_writer_t wtr_s = {buffer};
    buffer_writer_t *wtr = &wtr_s;

    write_intog(wtr, &point14->x);
    write_intog(wtr, &point14->y);
    write_intog(wtr, &point14->z);

    write_intog(wtr, &point14->intensity);

    // clang-format off
    uint8_t packed = 0;
    packed |= point14->return_number;
    packed |= point14->number_of_returns << 4;
    write_intog(wtr, (const uint8_t*)&packed);

    packed = 0;
    packed |= point14->synthetic;
    packed |= point14->key_point << 1;
    packed |= point14->withheld << 2;
    packed |= point14->overlap << 3;
    packed |= point14->scanner_channel << 4;
    packed |= point14->scan_direction_flag << 6;
    packed |= point14->edge_of_flight_line << 7;
    write_intog(wtr, (const uint8_t*)&packed);
    // clang-format on

    write_intog(wtr, &point14->classification);
    write_intog(wtr, &point14->user_data);
    write_intog(wtr, &point14->scan_angle);
    write_intog(wtr, &point14->point_source_id);

    if (has_gps_time(point_format.id))
    {
        write_intog(wtr, &point14->gps_time);
    }

    if (has_rgb(point_format.id))
    {
        write_intog(wtr, &point14->red);
        write_intog(wtr, &point14->green);
        write_intog(wtr, &point14->blue);
    }

    if (has_nir(point_format.id))
    {
        write_intog(wtr, &point14->nir);
    }

    if (has_waveform(point_format.id))
    {
        las_wave_packet_to_buffer(&point14->wave_packet, wtr->ptr);
        wtr->ptr += LAS_WAVE_PACKET_SIZE;
    }

    LAS_DEBUG_ASSERT((wtr->ptr - buffer) == las_point_standard_size(point_format.id));
    if (point14->extra_bytes != NULL)
    {
        LAS_DEBUG_ASSERT(point14->num_extra_bytes == point_format.num_extra_bytes);
        write_into(wtr, point14->extra_bytes, point14->num_extra_bytes);
    }

    LAS_DEBUG_ASSERT((wtr->ptr - buffer) == (uint64_t)las_point_format_point_size(point_format));
}

void las_raw_point_copy_from_point(las_raw_point_t *self,
                                   const las_point_t *point,
                                   const las_scaling_t scaling)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(point);

    if (self->point_format_id <= 5)
    {
        las_raw_point_10_t *rp = &self->point10;
        LAS_DEBUG_ASSERT(rp->num_extra_bytes == rp->num_extra_bytes);

        rp->x = las_scaling_unapply_x(scaling, point->x);
        rp->y = las_scaling_unapply_y(scaling, point->x);
        rp->z = las_scaling_unapply_z(scaling, point->z);

        rp->intensity = point->intensity;

        rp->return_number = point->return_number & 0b00000111;
        rp->number_of_returns = point->number_of_returns & 0b00000111;

        rp->scan_direction_flag = point->scan_direction_flag;
        rp->edge_of_flight_line = point->edge_of_flight_line;

        rp->classification = point->classification & UINT8_C(0b00011111);
        rp->synthetic = point->synthetic;
        rp->key_point = point->key_point;
        rp->withheld = point->withheld;

        rp->scan_angle_rank = (uint8_t)(point->scan_angle & 0b11111111);
        rp->user_data = point->user_data;
        rp->point_source_id = point->point_source_id;

        rp->gps_time = point->gps_time;

        rp->red = point->red;
        rp->green = point->green;
        rp->blue = point->blue;

        rp->wave_packet = point->wave_packet;

        if (rp->extra_bytes != NULL)
        {
            LAS_DEBUG_ASSERT(rp->extra_bytes != NULL)
            memcpy(rp->extra_bytes, point->extra_bytes, point->num_extra_bytes);
        }
    }
    else
    {
        las_raw_point_14_t *rp = &self->point14;
        LAS_DEBUG_ASSERT(rp->num_extra_bytes == point->num_extra_bytes);

        rp->x = las_scaling_unapply_x(scaling, point->x);
        rp->y = las_scaling_unapply_y(scaling, point->x);
        rp->z = las_scaling_unapply_z(scaling, point->z);

        rp->intensity = point->intensity;

        rp->return_number = point->return_number & 0b00001111;
        rp->number_of_returns = point->number_of_returns & 0b00001111;

        rp->synthetic = point->synthetic;
        rp->key_point = point->key_point;
        rp->withheld = point->withheld;
        rp->overlap = point->overlap;
        rp->scanner_channel = point->scanner_channel;
        rp->scan_direction_flag = point->scan_direction_flag;
        rp->edge_of_flight_line = point->edge_of_flight_line;

        rp->classification = point->classification;
        rp->user_data = point->user_data;
        rp->classification = point->classification;
        rp->scan_angle = point->scan_angle;
        rp->point_source_id = point->point_source_id;

        rp->gps_time = point->gps_time;

        rp->red = point->red;
        rp->green = point->green;
        rp->blue = point->blue;

        rp->nir = point->nir;

        rp->wave_packet = point->wave_packet;

        if (rp->extra_bytes != NULL)
        {
            LAS_DEBUG_ASSERT(rp->extra_bytes != NULL)
            memcpy(rp->extra_bytes, point->extra_bytes, point->num_extra_bytes);
        }
    }
}

void las_raw_point_copy_from_raw(
    las_raw_point_t *restrict dest,
    const las_raw_point_t *restrict source)
{
    LAS_DEBUG_ASSERT_NOT_NULL(dest);
    LAS_DEBUG_ASSERT_NOT_NULL(source);

    if (dest->point_format_id <= 5 && source->point_format_id <= 5)
    {
        // Simple case where we can simply copy data
        las_raw_point_10_t *d = &dest->point10;
        const las_raw_point_10_t *s = &source->point10;
        LAS_DEBUG_ASSERT(d->num_extra_bytes == s->num_extra_bytes);

        *d = *s;

        if (s->num_extra_bytes)
        {
            LAS_DEBUG_ASSERT_NOT_NULL(d->extra_bytes);
            LAS_DEBUG_ASSERT_NOT_NULL(s->extra_bytes);
            memcpy(d->extra_bytes, s->extra_bytes, sizeof(uint8_t) * s->num_extra_bytes);
            uint8_t *eb = d->extra_bytes;
            d->extra_bytes = eb;
        }
    } else if (dest->point_format_id >= 6 && source->point_format_id >= 6)
    {
        // Simple case where we can simply copy data
        las_raw_point_14_t *d = &dest->point14;
        const las_raw_point_14_t *s = &source->point14;
        LAS_DEBUG_ASSERT(d->num_extra_bytes == s->num_extra_bytes);

        *d = *s;

        if (s->num_extra_bytes)
        {
            LAS_DEBUG_ASSERT_NOT_NULL(d->extra_bytes);
            LAS_DEBUG_ASSERT_NOT_NULL(s->extra_bytes);
            memcpy(d->extra_bytes, s->extra_bytes, sizeof(uint8_t) * s->num_extra_bytes);
            uint8_t *eb = d->extra_bytes;
            d->extra_bytes = eb;
        }
    } else if (dest->point_format_id <= 5 && source->point_format_id >= 6)
    {
        // Here the underlying data is not totally the same, so we manually copy
        las_raw_point_10_t *d = &dest->point10;
        const las_raw_point_14_t *s = &source->point14;
        LAS_DEBUG_ASSERT(d->num_extra_bytes == s->num_extra_bytes);

        d->x = s->x;
        d->y = s->y;
        d->z = s->z;

        d->intensity = s->intensity;
        d->return_number = s->return_number & (uint8_t) 0b0000111;
        d->number_of_returns = s->number_of_returns & 0b0000111;

        d->scan_direction_flag = s->scan_direction_flag;
        d->edge_of_flight_line = s->edge_of_flight_line;

        d->classification = s->classification & (uint8_t) 0b00011111;
        d->synthetic = s->synthetic;
        d->key_point = s->key_point;
        d->withheld = s->withheld;

        d->scan_angle_rank = (uint8_t)s->scan_angle;
        d->user_data = s->user_data;
        d->point_source_id = s->point_source_id;

        d->gps_time = s->gps_time;

        d->red = s->red;
        d->green = s->green;
        d->blue = s->blue;

        d->wave_packet = s->wave_packet;

        if (s->num_extra_bytes)
        {
            LAS_DEBUG_ASSERT_NOT_NULL(d->extra_bytes);
            LAS_DEBUG_ASSERT_NOT_NULL(s->extra_bytes);
            memcpy(d->extra_bytes, s->extra_bytes, s->num_extra_bytes);
        }
    } else if (dest->point_format_id >= 6 && source->point_format_id <= 5) {
        // Here the underlying data is not totally the same, so we manually copy
        las_raw_point_14_t *d = &dest->point14;
        const las_raw_point_10_t *s = &source->point10;
        LAS_DEBUG_ASSERT(d->num_extra_bytes == s->num_extra_bytes);

        d->x = s->x;
        d->y = s->y;
        d->z = s->z;

        d->intensity = s->intensity;

        d->return_number = s->return_number;
        d->number_of_returns = s->number_of_returns;

        d->synthetic = s->synthetic;
        d->key_point = s->key_point;
        d->withheld = s->withheld;
        d->overlap = 0;
        d->scanner_channel = 0;
        d->scan_direction_flag = s->scan_direction_flag;
        d->edge_of_flight_line = s->edge_of_flight_line;

        d->classification = s->classification;
        d->user_data = s->user_data;
        d->scan_angle = (uint16_t)s->scan_angle_rank;
        d->point_source_id = s->point_source_id;

        d->gps_time = s->gps_time;

        d->red = s->red;
        d->green = s->green;
        d->blue = s->blue;

        d->wave_packet = s->wave_packet;

        if (s->num_extra_bytes)
        {
            LAS_DEBUG_ASSERT_NOT_NULL(d->extra_bytes);
            LAS_DEBUG_ASSERT_NOT_NULL(s->extra_bytes);
            memcpy(d->extra_bytes, s->extra_bytes, s->num_extra_bytes);
        }
    }

}

void las_point_copy_from_raw(las_point_t *self,
                             const las_raw_point_t *raw_point,
                             const las_scaling_t scaling)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(raw_point);

    if (raw_point->point_format_id <= 5)
    {
        const las_raw_point_10_t *rp = &raw_point->point10;
        LAS_DEBUG_ASSERT(self->num_extra_bytes == rp->num_extra_bytes);

        self->x = las_scaling_apply_x(scaling, rp->x);
        self->y = las_scaling_apply_y(scaling, rp->y);
        self->z = las_scaling_apply_z(scaling, rp->z);

        self->intensity = rp->intensity;

        self->return_number = rp->return_number;
        self->number_of_returns = rp->number_of_returns;

        self->scan_direction_flag = rp->scan_direction_flag;
        self->edge_of_flight_line = rp->edge_of_flight_line;

        self->classification = rp->classification;
        self->synthetic = rp->synthetic;
        self->key_point = rp->key_point;
        self->withheld = rp->withheld;

        self->scan_angle = rp->scan_angle_rank;
        self->user_data = rp->user_data;
        self->point_source_id = rp->point_source_id;

        self->gps_time = rp->gps_time;

        self->red = rp->red;
        self->green = rp->green;
        self->blue = rp->blue;

        self->wave_packet = rp->wave_packet;

        if (rp->extra_bytes != NULL)
        {
            LAS_DEBUG_ASSERT(self->extra_bytes != NULL);
            memcpy(self->extra_bytes, rp->extra_bytes, rp->num_extra_bytes);
        }
    }
    else
    {
        const las_raw_point_14_t *rp = &raw_point->point14;
        LAS_DEBUG_ASSERT(self->num_extra_bytes == raw_point->point14.num_extra_bytes);

        self->x = las_scaling_apply_x(scaling, rp->x);
        self->y = las_scaling_apply_y(scaling, rp->y);
        self->z = las_scaling_apply_z(scaling, rp->z);

        self->intensity = rp->intensity;

        self->return_number = rp->return_number;
        self->number_of_returns = rp->number_of_returns;

        self->synthetic = rp->synthetic;
        self->key_point = rp->key_point;
        self->withheld = rp->withheld;
        self->overlap = rp->overlap;
        self->scanner_channel = rp->scanner_channel;
        self->scan_direction_flag = rp->scan_direction_flag;
        self->edge_of_flight_line = rp->edge_of_flight_line;

        self->classification = rp->classification;
        self->user_data = rp->user_data;
        self->classification = rp->classification;
        self->scan_angle = rp->scan_angle;
        self->point_source_id = rp->point_source_id;

        self->gps_time = rp->gps_time;

        self->red = rp->red;
        self->green = rp->green;
        self->blue = rp->blue;

        self->nir = rp->nir;

        self->wave_packet = rp->wave_packet;

        if (rp->extra_bytes != NULL)
        {
            LAS_DEBUG_ASSERT(self->extra_bytes != NULL);
            memcpy(self->extra_bytes, rp->extra_bytes, rp->num_extra_bytes);
        }
    }
}

int las_raw_point_eq(const las_raw_point_t *lhs, const las_raw_point_t *rhs)
{
    LAS_DEBUG_ASSERT_NOT_NULL(lhs);
    LAS_DEBUG_ASSERT_NOT_NULL(rhs);

    if (lhs->point_format_id != rhs->point_format_id)
    {
        return 0;
    }

    if (lhs->point_format_id <= 5)
    {
        const las_raw_point_10_t *rpl = &rhs->point10;
        const las_raw_point_10_t *rph = &rhs->point10;

        return rpl->x == rph->x &&
               rpl->y == rph->y &&
               rpl->z == rph->z &&
               rpl->intensity == rph->intensity &&
               rpl->return_number == rph->return_number &&
               rpl->number_of_returns == rph->number_of_returns &&
               rpl->scan_direction_flag == rph->scan_direction_flag &&
               rpl->edge_of_flight_line == rph->edge_of_flight_line &&
               rpl->classification == rph->classification &&
               rpl->synthetic == rph->synthetic &&
               rpl->key_point == rph->key_point &&
               rpl->withheld == rph->withheld &&
               rpl->scan_angle_rank == rph->scan_angle_rank &&
               rpl->user_data == rph->user_data &&
               rpl->point_source_id == rph->point_source_id &&
               rpl->gps_time == rph->gps_time &&
               rpl->red == rph->red &&
               rpl->green == rph->green &&
               rpl->blue == rph->blue &&
               las_wave_packet_eq(&rpl->wave_packet, &rph->wave_packet) &&
               rpl->num_extra_bytes == rph->num_extra_bytes &&
               (memcmp(rpl->extra_bytes, rph->extra_bytes, rpl->num_extra_bytes) == 0);
    }
    else
    {
        LAS_ASSERT_M(false, "todo");
    }
}
void las_point_prepare(las_point_t *self, const las_point_format_t point_format)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    memset(self, 0, sizeof(las_point_t));

    uint8_t *extra_bytes = NULL;
    if (point_format.num_extra_bytes != 0)
    {
        extra_bytes = malloc(sizeof(uint8_t) * point_format.num_extra_bytes);
        LAS_ASSERT_M(extra_bytes != NULL, "out of memory");
        memset(extra_bytes, 0, sizeof(uint8_t) * point_format.num_extra_bytes);
    }
    self->extra_bytes = extra_bytes;
    self->num_extra_bytes = point_format.num_extra_bytes;
}

void las_point_deinit(las_point_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    if (self->extra_bytes != NULL)
    {
        free(self->extra_bytes);
        self->num_extra_bytes = 0;
    }
}
