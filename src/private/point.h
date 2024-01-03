#ifndef LAS_C_PRIV_POINT_H
#define LAS_C_PRIV_POINT_H

#include "las/point.h"

#include "header.h"
#include "source.h"

#define LAS_WAVE_PACKET_SIZE 29

// TODO there is no reason for the to_ and from_ function to take
// parameters with slight order change

void las_wave_packet_from_buffer(const uint8_t *buffer, las_wave_packet_t *wave_packet);
void las_wave_packet_to_buffer(const las_wave_packet_t *wave_packet, uint8_t *buffer);

/// Populates members of the `point10` by reading the `buffer`
///
/// \param buffer Input buffer, its size __must__ be >= header->point_size
/// \param point_format The point format, to know which fields needs to be populated
/// \param point10 Output point that will be populated
void las_raw_point_10_from_buffer(const uint8_t *buffer,
                                  las_point_format_t point_format,
                                  las_raw_point_10_t *point10);

/// Writes the members values of the `point10` to the `buffer`
///
/// \param point10 The point to write
/// \param point_format The point format, to know which fields needs to be populated
/// \param buffer Output buffer, its size __must__ be >= header->point_size
void las_raw_point_10_to_buffer(const las_raw_point_10_t *point10,
                                las_point_format_t point_format,
                                uint8_t *buffer);

/// Populates members of the `point14` by reading the `buffer`
///
/// \param buffer Input buffer, its size __must__ be >= header->point_size
/// \param point_format The point format, to know which fields needs to be populated
/// \param point14 Output point that will be populated
void las_raw_point_14_from_buffer(const uint8_t *buffer,
                                  las_point_format_t point_format,
                                  las_raw_point_14_t *point14);

/// Writes the members values of the `point14` to the `buffer`
///
/// \param point14 The point to write
/// \param point_format The point format, to know which fields needs to be populated
/// \param buffer Output buffer, its size __must__ be >= header->point_size
void las_raw_point_14_to_buffer(const las_raw_point_14_t *point14,
                                las_point_format_t point_format,
                                uint8_t *buffer);

#endif // LAS_C_PRIV_POINT_H
