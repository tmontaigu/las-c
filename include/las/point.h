#ifndef LAS_C_POINT_H
#define LAS_C_POINT_H

#include <las/header.h>
#include <stdint.h>

/// Wave-packet information
///
/// For point formats 4, 5, 9, 10
struct las_wave_packet_t
{
    uint8_t descriptor_index;
    uint64_t byte_offset_to_data;
    uint32_t size_in_bytes;
    float return_point_waveform_location;
    float xt;
    float yt;
    float zt;
};

typedef struct las_wave_packet_t las_wave_packet_t;

int las_wave_packet_eq(const las_wave_packet_t *lhs, const las_wave_packet_t *rhs);

/// Raw representation of point 1.
///
/// This struct covers members for point format [0, 5]
/// which were introduced in LAS 1.0 and 1.3
struct las_raw_point_10_t
{
    int32_t x;
    int32_t y;
    int32_t z;
    uint16_t intensity;

    uint8_t return_number : 3;
    uint8_t number_of_returns : 3;
    uint8_t scan_direction_flag : 1;
    uint8_t edge_of_flight_line : 1;

    uint8_t classification : 5;
    uint8_t synthetic : 1;
    uint8_t key_point : 1;
    uint8_t withheld : 1;

    uint8_t scan_angle_rank;
    uint8_t user_data;
    uint16_t point_source_id;

    // fmt 1 & 3
    double gps_time;

    // fmt 2 & 3 & 5
    uint16_t red;
    uint16_t green;
    uint16_t blue;

    // fmt 4 & 5
    las_wave_packet_t wave_packet;

    // all fmt (num can be 0)
    uint16_t num_extra_bytes;
    uint8_t *extra_bytes;
};

typedef struct las_raw_point_10_t las_raw_point_10_t;

/// Cleans the point data members
///
/// Does __not__ free the `point`, only frees what is 'inside'
void las_raw_point_10_deinit(las_raw_point_10_t *point);

/// Raw representation of point 1.4
///
/// This struct covers members for point format [6, 10]
/// which were introduced in LAS 1.4
struct las_raw_point_14_t
{
    int32_t x;
    int32_t y;
    int32_t z;

    uint16_t intensity;

    uint8_t return_number : 4;
    uint8_t number_of_returns : 4;

    uint8_t synthetic : 1;
    uint8_t key_point : 1;
    uint8_t withheld : 1;
    uint8_t overlap : 1;
    uint8_t scanner_channel : 2;
    uint8_t scan_direction_flag : 1;
    uint8_t edge_of_flight_line : 1;

    uint8_t classification;
    uint8_t user_data;
    uint16_t scan_angle;
    uint16_t point_source_id;
    double gps_time;

    // fmt 7 & 8 & 10
    uint16_t red;
    uint16_t green;
    uint16_t blue;

    // fmt 8 & 10
    uint16_t nir;

    // fmt 9 & 10
    las_wave_packet_t wave_packet;

    // all fmt (num can be 0)
    uint16_t num_extra_bytes;
    uint8_t *extra_bytes;
};

typedef struct las_raw_point_14_t las_raw_point_14_t;

/// Cleans the point data members
///
/// Does __not__ free the `point`, only frees what is 'inside'
void las_raw_point_14_deinit(las_raw_point_14_t *self);

/// Raw point tagged union
struct las_raw_point_t
{
    /// The point format to know which member is
    /// the correct one.
    uint8_t point_format_id;
    union
    {
        /// When 0 <= point_format_id <= 5.
        las_raw_point_10_t point10;
        /// When 6 <= point_format_id <= 10.
        las_raw_point_14_t point14;
    };
};

typedef struct las_raw_point_t las_raw_point_t;

void las_raw_point_copy_from_raw(
    las_raw_point_t *restrict self,
    const las_raw_point_t *restrict source);

int las_raw_point_eq(const las_raw_point_t *lhs, const las_raw_point_t *rhs);

/// Prepares the point to match the description from the header
///
/// Call `las_raw_point_deinit` once you are done with the point
void las_raw_point_prepare(las_raw_point_t *point, las_point_format_t point_format);

/// Cleans the point data members
///
/// Does __not__ free the `point`, only frees what is 'inside'
void las_raw_point_deinit(las_raw_point_t *point);

struct las_point_t
{
    double x;
    double y;
    double z;

    uint16_t intensity;

    uint8_t return_number : 4;
    uint8_t number_of_returns : 4;

    uint8_t synthetic : 1;
    uint8_t key_point : 1;
    uint8_t withheld : 1;
    uint8_t overlap : 1;
    uint8_t scanner_channel : 2;
    uint8_t scan_direction_flag : 1;
    uint8_t edge_of_flight_line : 1;

    uint8_t classification;
    uint8_t user_data;
    uint16_t scan_angle;
    uint16_t point_source_id;
    double gps_time;

    // fmt 2, 3, 5, 7, 8, 10
    uint16_t red;
    uint16_t green;
    uint16_t blue;

    // fmt 8 & 10
    uint16_t nir;

    // fmt 4, 5, 9, 10
    las_wave_packet_t wave_packet;

    // all fmt (num can be 0)
    uint16_t num_extra_bytes;
    uint8_t *extra_bytes;
};

typedef struct las_point_t las_point_t;


void las_raw_point_copy_from_point(las_raw_point_t *self,
                                   const las_point_t *point,
                                   las_scaling_t scaling);

void las_point_copy_from_raw(las_point_t *self,
                             const las_raw_point_t *raw_point,
                             las_scaling_t scaling);

void las_point_prepare(las_point_t *self, las_point_format_t point_format);

void las_point_deinit(las_point_t *self);

#endif // LAS_C_POINT_H
