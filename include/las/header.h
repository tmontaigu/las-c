#ifndef LAS_C_HEADER_H
#define LAS_C_HEADER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <las/vlr.h>

#include <stdint.h>

#define LAS_SIGNATURE_SIZE 4
#define LAS_GUID_SIZE 16
#define LAS_SYSTEM_IDENTIFIER_SIZE 32
#define LAS_GENERATING_SOFTWARE_SIZE 32
#define LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE 15
#define LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE 5

typedef struct las_version
{
    uint8_t major;
    uint8_t minor;
} las_version_t;

typedef struct
{
    uint8_t id;
    uint16_t num_extra_bytes;
    int is_compressed;
} las_point_format_t;

/// Returns the size in bytes that the given format_id takes
uint16_t las_point_standard_size(uint8_t format_id);

/// Returns the size in bytes that the point format takes
///
/// This incldues the base size and extra bytes (if any)
static inline uint16_t las_point_format_point_size(const las_point_format_t self)
{
    return las_point_standard_size(self.id) + self.num_extra_bytes;
}

typedef struct
{
    double x;
    double y;
    double z;
} las_vector_3_t;

typedef struct
{
    las_vector_3_t scales;
    las_vector_3_t offsets;
} las_scaling_t;

static inline double scaling_apply(const double scale, const double offset, const int32_t value)
{
    return (((double)value) * scale) + offset;
}

static inline int32_t scaling_unapply(const double scale, const double offset, const double value)
{
    return (int32_t)((value - offset) / scale);
}

static inline double las_scaling_apply_x(const las_scaling_t self, const int32_t x)
{
    return scaling_apply(self.scales.x, self.offsets.x, x);
}

static inline double las_scaling_apply_y(const las_scaling_t self, const int32_t y)
{
    return scaling_apply(self.scales.y, self.offsets.y, y);
}

static inline double las_scaling_apply_z(const las_scaling_t self, const int32_t z)
{
    return scaling_apply(self.scales.z, self.offsets.z, z);
}

static inline int32_t las_scaling_unapply_x(const las_scaling_t self, const double x)
{
    return scaling_unapply(self.scales.x, self.offsets.x, x);
}

static inline int32_t las_scaling_unapply_y(const las_scaling_t self, const double y)
{
    return scaling_unapply(self.scales.y, self.offsets.y, y);
}

static inline int32_t las_scaling_unapply_z(const las_scaling_t self, const double z)
{
    return scaling_unapply(self.scales.z, self.offsets.z, z);
}

/// LAS header
///
/// Contains metadata describing the points contained in the file
struct las_header_t
{
    uint16_t file_source_id;
    uint16_t global_encoding;
    uint8_t guid[LAS_GUID_SIZE];
    las_version_t version;

    /// Does not need to be null terminated
    char system_identifier[LAS_SYSTEM_IDENTIFIER_SIZE];
    /// Does not need to be null terminated
    char generating_software[LAS_GENERATING_SOFTWARE_SIZE];

    uint16_t file_creation_day_of_year;
    uint16_t file_creation_year;

    uint32_t number_of_vlrs;
    las_vlr_t *vlrs;

    las_point_format_t point_format;
    uint64_t point_count;

    /// For point_format_id <= 5, the real size is
    /// `LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE`
    /// and the values are stored as uint32_t (in the file)
    uint64_t number_of_points_by_return[LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE];

    las_scaling_t scaling;

    las_vector_3_t mins;
    las_vector_3_t maxs;

    // version >= 1.3
    uint64_t start_of_waveform_datapacket;

    // version >= 1.4
    uint64_t start_of_evlrs;
    uint32_t number_of_evlrs;

    uint32_t num_extra_header_bytes;
    uint8_t *extra_header_bytes;

    /// Internal information
    uint32_t offset_to_point_data;
};

typedef struct las_header_t las_header_t;

/// Clones and returns the header in out_header
///
/// - returns 0 if everything was ok.
/// - returns 1 if allocation failed
int las_header_clone(const las_header_t *self, las_header_t **out_header);

/// frees the header and everything that it contains
///
/// \param self __can__ be null
void las_header_delete(las_header_t *self);


#ifdef __cplusplus
}
#endif

#endif // LAS_C_HEADER_H
