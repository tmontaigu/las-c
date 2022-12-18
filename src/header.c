#include "las/header.h"
#include "las/error.h"

#include <stdlib.h>

#include "private/macro.h"
#include "private/point.h"
#include "private/source.h"
#include "private/utils.h"

#define LAS_VLR_HEADER_SIZE 54
#define LAS_SIGNATURE "LASF"

#define LAS_HEADER_1_0_SIZE 227
#define LAS_HEADER_1_1_SIZE 227
#define LAS_HEADER_1_2_SIZE 227
#define LAS_HEADER_1_3_SIZE 235
#define LAS_HEADER_1_4_SIZE 375

static uint16_t las_header_size_for_version(las_version_t version)
{
    LAS_DEBUG_ASSERT(version.major == 1);
    LAS_DEBUG_ASSERT(version.minor <= 4);

    uint16_t size = 0;
    switch (version.minor)
    {
    case 0:
        size = LAS_HEADER_1_0_SIZE;
        break;
    case 1:
        size = LAS_HEADER_1_1_SIZE;
        break;
    case 2:
        size = LAS_HEADER_1_2_SIZE;
        break;
    case 3:
        size = LAS_HEADER_1_3_SIZE;
        break;
    case 4:
        size = LAS_HEADER_1_4_SIZE;
        break;
    }

    return size;
}

static inline int is_point_format_compressed(uint8_t point_format_id)
{
    uint8_t compression_bit_7 = (point_format_id & 0x80) >> 7;
    uint8_t compression_bit_6 = (point_format_id & 0x40) >> 6;
    return (int)(!compression_bit_6 && compression_bit_7);
}

static inline uint8_t compressed_id_to_uncompressed(uint8_t point_format_id)
{
    return point_format_id & 0x3F;
}

static int las_version_is_compatible_with_point_format(las_version_t version,
                                                       uint8_t point_format_id)
{
    LAS_DEBUG_ASSERT(version.major == 1);
    LAS_DEBUG_ASSERT(version.minor <= 4);
    LAS_DEBUG_ASSERT(point_format_id <= 10);

    if (point_format_id <= 3 && version.minor >= 0)
    {
        return 1;
    }

    if (point_format_id <= 5 && version.minor >= 3)
    {
        return 1;
    }

    if (point_format_id <= 10 && version.minor >= 4)
    {
        return 1;
    }

    return 0;
}

las_error_t las_vlr_read_into(las_source_t *source, las_vlr_t *vlr)
{
    LAS_DEBUG_ASSERT(vlr != NULL);

    las_error_t error;
    error.kind = LAS_ERROR_OK;
    uint8_t header_bytes[LAS_VLR_HEADER_SIZE];

    uint64_t n = las_source_read(source, LAS_VLR_HEADER_SIZE, &header_bytes[0]);
    if (n != LAS_VLR_HEADER_SIZE)
    {
        error.kind = LAS_ERROR_UNEXPECTED_EOF;
        return error;
    }

    // First two bytes are reserved
    buffer_reader_t rdr = {&header_bytes[2]};
    read_into(&rdr, (uint8_t *)&vlr->user_id, LAS_VLR_USER_ID_SIZE);
    read_intog(&rdr, &vlr->record_id);
    read_intog(&rdr, &vlr->data_size);
    read_into(&rdr, (uint8_t *)&vlr->description, LAS_VLR_DESCRIPTION_SIZE);

    LAS_DEBUG_ASSERT((rdr.ptr - header_bytes) == LAS_VLR_HEADER_SIZE);

    vlr->data = malloc(sizeof(uint8_t) * vlr->data_size);
    if (vlr->data == NULL)
    {
        error.kind = LAS_ERROR_MEMORY;
        return error;
    }

    n = las_source_read(source, vlr->data_size, vlr->data);

    if (n != vlr->data_size)
    {
        error.kind = LAS_ERROR_UNEXPECTED_EOF;
        vlr->data_size = 0;
        free(vlr->data);
        vlr->data = NULL;
    }

    return error;
}

las_error_t las_vlr_write_to(const las_vlr_t *self, las_dest_t *dest)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(dest);

    las_error_t error;
    error.kind = LAS_ERROR_OK;
    uint8_t header_bytes[LAS_VLR_HEADER_SIZE];

    buffer_writer_t wrt = {&header_bytes[0]};

    const uint16_t reserved = 0;
    write_intog(&wrt, &reserved);
    write_into(&wrt, &self->user_id[0], LAS_VLR_USER_ID_SIZE);
    write_intog(&wrt, &self->record_id);
    write_intog(&wrt, &self->data_size);
    write_into(&wrt, &self->description[0], LAS_VLR_DESCRIPTION_SIZE);

    LAS_DEBUG_ASSERT((wrt.ptr - header_bytes) == LAS_VLR_HEADER_SIZE);

    uint64_t n = las_dest_write(dest, &header_bytes[0], LAS_VLR_HEADER_SIZE);
    if (self->data_size != 0 && self->data != NULL)
    {
        n += las_dest_write(dest, self->data, self->data_size);
    }

    if (n != self->data_size + LAS_VLR_HEADER_SIZE)
    {
        error.kind = LAS_ERROR_UNEXPECTED_EOF;
    }

    return error;
}

static uint16_t las_vlr_size(const las_vlr_t *self)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    LAS_ASSERT(self->data_size < (UINT16_MAX - LAS_VLR_HEADER_SIZE));

    return self->data_size + LAS_VLR_HEADER_SIZE;
}

int las_vlr_clone_into(const las_vlr_t *self, las_vlr_t *out_vlr)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(out_vlr != NULL);

    *out_vlr = *self;

    out_vlr->data = malloc(out_vlr->data_size);
    if (out_vlr->data == NULL)
    {
        out_vlr->data_size = 0;
        return 1;
    }

    return 0;
}

int las_vlr_clone(const las_vlr_t *self, las_vlr_t **out_vlr)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(out_vlr != NULL);

    las_vlr_t *vlr = malloc(sizeof(las_vlr_t));
    if (vlr == NULL)
    {
        return 1;
    }

    if (las_vlr_clone_into(self, vlr) != 0)
    {
        return 1;
    }

    return 0;
}

void las_vlr_deinit(las_vlr_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    if (self->data != NULL)
    {
        free(self->data);
        self->data = NULL;
        self->data_size = 0;
    }
}

const las_vlr_t *las_header_find_laszip_vlr(const las_header_t *las_header)
{
    const las_vlr_t *vlr = NULL;

    for (uint32_t i = 0; i < las_header->number_of_vlrs; ++i)
    {
        const las_vlr_t *current = &las_header->vlrs[i];
        if (strncmp(&current->user_id[0], "laszip encoded", LAS_VLR_USER_ID_SIZE - 1) == 0 &&
            current->record_id == 22204)
        {
            vlr = current;
            break;
        }
    }
    return vlr;
}

static las_error_t las_point_format_from_id_and_size(las_point_format_t *self,
                                                     uint8_t point_format_id,
                                                     const uint16_t point_size)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);

    las_error_t err = {LAS_ERROR_OK};

    int is_compressed = is_point_format_compressed(point_format_id);
    point_format_id = compressed_id_to_uncompressed(point_format_id);

    if (point_format_id > 10)
    {
        err.kind = LAS_ERROR_INVALID_POINT_FORMAT;
        err.point_format_id = point_format_id;
        return err;
    }

    uint16_t minimum_size = las_point_standard_size(point_format_id);
    if (point_size < minimum_size)
    {
        err.kind = LAS_ERROR_INVALID_POINT_SIZE;
        err.point_size.point_size = point_size;
        err.point_size.point_format_id = point_format_id;
        err.point_size.minimum = minimum_size;
        return err;
    }

    self->id = point_format_id;
    self->num_extra_bytes = point_size - minimum_size;
    self->is_compressed = is_compressed;

    return err;
}

static las_error_t las_point_format_validate(const las_point_format_t self)
{
    las_error_t err = {LAS_ERROR_OK};
    if (self.id > 10)
    {
        err.kind = LAS_ERROR_INVALID_POINT_FORMAT;
        err.point_format_id = self.id;
        return err;
    }

    return err;
}

las_error_t las_header_validate(const las_header_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    las_error_t err;
    err.kind = LAS_ERROR_OK;

    if (self->version.major != 1 || self->version.minor > 4)
    {
        err.kind = LAS_ERROR_INVALID_VERSION;
        err.version = self->version;
        return err;
    }

    err = las_point_format_validate(self->point_format);
    if (las_error_is_failure(&err))
    {
        return err;
    }

    return err;
}

las_error_t las_header_validate_for_writing(const las_header_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);
    las_error_t las_err = {LAS_ERROR_OK};

    las_err = las_header_validate(self);
    if (las_error_is_failure(&las_err))
    {
        return las_err;
    }

    if (las_version_is_compatible_with_point_format(self->version, self->point_format.id) == 0)
    {
        las_err.kind = LAS_ERROR_INCOMPATIBLE_VERSION_AND_FORMAT;
        las_err.incompatible.version = self->version;
        las_err.incompatible.point_format_id = self->point_format.id;
        return las_err;
    }

    return las_err;
}

void las_header_reset(las_header_t *header)
{
    memset(header, 0, sizeof(las_header_t));
}

las_error_t las_header_read_from(las_source_t *source, las_header_t *header)
{
    uint64_t n = 0;
    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;

    las_header_reset(header);

    uint8_t header_bytes[LAS_HEADER_1_4_SIZE] = {0};

    buffer_reader_t rdr = {&header_bytes[0]};

    n += las_source_read(source, LAS_HEADER_1_2_SIZE, &header_bytes[0]);
    if (n < LAS_HEADER_1_2_SIZE)
    {
        if (las_source_eof(source))
        {
            las_err.kind = LAS_ERROR_UNEXPECTED_EOF;
        }
        else
        {
            las_err.kind = LAS_ERROR_ERRNO;
        }
        return las_err;
    }

    // + 1 so that it's a null terminated string we can print
    char signature[LAS_SIGNATURE_SIZE + 1] = "";
    read_into(&rdr, &signature[0], LAS_SIGNATURE_SIZE);
    if (strncmp(signature, LAS_SIGNATURE, LAS_SIGNATURE_SIZE) != 0)
    {
        las_err.kind = LAS_ERROR_INVALID_SIGNATURE;
        strcpy(las_err.signature, signature);
        return las_err;
    }

    read_intog(&rdr, &header->file_source_id);
    read_intog(&rdr, &header->global_encoding);

    read_into(&rdr, &header->guid[0], LAS_GUID_SIZE);

    read_intog(&rdr, &header->version.major);
    read_intog(&rdr, &header->version.minor);

    read_into(&rdr, &header->system_identifier[0], LAS_SYSTEM_IDENTIFIER_SIZE);
    read_into(&rdr, &header->generating_software[0], LAS_GENERATING_SOFTWARE_SIZE);

    read_intog(&rdr, &header->file_creation_day_of_year);
    read_intog(&rdr, &header->file_creation_year);

    uint16_t header_size;
    read_intog(&rdr, &header_size);
    read_intog(&rdr, &header->offset_to_point_data);

    read_intog(&rdr, &header->number_of_vlrs);
    uint8_t point_format_id;
    uint16_t point_size;
    read_intog(&rdr, &point_format_id);
    read_intog(&rdr, &point_size);
    las_err = las_point_format_from_id_and_size(&header->point_format, point_format_id, point_size);
    if (las_error_is_failure(&las_err))
    {
        return las_err;
    }

    uint32_t legacy_point_count;
    read_intog(&rdr, &legacy_point_count);
    header->point_count = (uint64_t)legacy_point_count;

    uint32_t rn;
    for (int i = 0; i < LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE; ++i)
    {
        read_intog(&rdr, &rn);
        header->number_of_points_by_return[i] = (uint64_t)rn;
    }

    read_intog(&rdr, &header->scaling.scales.x);
    read_intog(&rdr, &header->scaling.scales.y);
    read_intog(&rdr, &header->scaling.scales.z);

    read_intog(&rdr, &header->scaling.offsets.x);
    read_intog(&rdr, &header->scaling.offsets.y);
    read_intog(&rdr, &header->scaling.offsets.z);

    read_intog(&rdr, &header->maxs.x);
    read_intog(&rdr, &header->mins.x);

    read_intog(&rdr, &header->maxs.y);
    read_intog(&rdr, &header->mins.y);

    read_intog(&rdr, &header->maxs.z);
    read_intog(&rdr, &header->mins.z);

    // End of header for versions [1.0, 1.1, 1.2]
    LAS_DEBUG_ASSERT((rdr.ptr - &header_bytes[0]) == LAS_HEADER_1_2_SIZE);

    if (header->version.major >= 1 && header->version.minor >= 3)
    {
        n += las_source_read(
            source, LAS_HEADER_1_3_SIZE - LAS_HEADER_1_2_SIZE, &header_bytes[LAS_HEADER_1_2_SIZE]);

        read_intog(&rdr, &header->start_of_waveform_datapacket);

        // End of header for versions [1.3]
        LAS_DEBUG_ASSERT((rdr.ptr - &header_bytes[0]) == LAS_HEADER_1_3_SIZE);
    }

    if (header->version.major >= 1 && header->version.minor >= 4)
    {
        n += las_source_read(
            source, LAS_HEADER_1_4_SIZE - LAS_HEADER_1_3_SIZE, &header_bytes[LAS_HEADER_1_3_SIZE]);

        read_intog(&rdr, &header->start_of_evlrs);
        read_intog(&rdr, &header->number_of_evlrs);
        read_intog(&rdr, &header->point_count);

        for (int i = 0; i < LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE; ++i)
        {
            uint64_t *elem = &header->number_of_points_by_return[i];
            read_intog(&rdr, elem);
        }

        // End of header for versions [1.4]
        LAS_DEBUG_ASSERT((rdr.ptr - &header_bytes[0]) == LAS_HEADER_1_4_SIZE);
    }

    if (las_source_eof(source))
    {
        las_err.kind = LAS_ERROR_UNEXPECTED_EOF;
        return las_err;
    }

    if (n < (uint64_t)header_size)
    {
        // There are some unknown extra header bytes
        uint32_t extra_size = (uint32_t)header_size - (uint32_t)n;
        header->extra_header_bytes = malloc(sizeof(uint8_t) * extra_size);
        if (header->extra_header_bytes == NULL)
        {
            las_err.kind = LAS_ERROR_MEMORY;
            return las_err;
        }
        n += las_source_read(source, header->num_extra_header_bytes, header->extra_header_bytes);
        header->num_extra_header_bytes = extra_size;
    }

    if (header->number_of_vlrs > 0)
    {
        header->vlrs = malloc(sizeof(las_vlr_t) * header->number_of_vlrs);
        if (header->vlrs == NULL)
        {
            las_header_deinit(header);
            las_err.kind = LAS_ERROR_MEMORY;
            return las_err;
        }
        for (uint32_t i = 0; i < header->number_of_vlrs; ++i)
        {
            las_err = las_vlr_read_into(source, &header->vlrs[i]);
            if (las_error_is_failure(&las_err))
            {
                las_header_deinit(header);
                break;
            }
        }
    }

    return las_err;
}

las_error_t las_header_write_to(const las_header_t *self, las_dest_t *dest)
{
    LAS_DEBUG_ASSERT_NOT_NULL(self);
    LAS_DEBUG_ASSERT_NOT_NULL(dest);

    las_error_t las_err;
    las_err.kind = LAS_ERROR_OK;

    uint32_t legacy_point_count;

    if (self->point_count > (uint64_t)UINT32_MAX)
    {
        if (self->version.major == 1 && self->version.minor <= 3)
        {
            las_err.kind = LAS_ERROR_POINT_COUNT_TOO_HIGH;
            las_err.point_count = self->point_count;
            return las_err;
        }
        else
        {
            legacy_point_count = 0;
        }
    }
    else
    {
        legacy_point_count = (uint32_t)self->point_count;
    }

    uint8_t header_bytes[LAS_HEADER_1_4_SIZE];

    buffer_writer_t wtr_s = {&header_bytes[0]};
    buffer_writer_t *wtr = &wtr_s;

    write_into(wtr, LAS_SIGNATURE, LAS_SIGNATURE_SIZE);
    write_intog(wtr, &self->file_source_id);
    write_intog(wtr, &self->global_encoding);
    write_into(wtr, &self->guid[0], LAS_GUID_SIZE);
    write_intog(wtr, &self->version.major);
    write_intog(wtr, &self->version.minor);
    write_into(wtr, &self->system_identifier[0], LAS_SYSTEM_IDENTIFIER_SIZE);
    write_into(wtr, &self->generating_software[0], LAS_GENERATING_SOFTWARE_SIZE);
    write_intog(wtr, &self->file_creation_day_of_year);
    write_intog(wtr, &self->file_creation_year);
    uint16_t header_size =
        las_header_size_for_version(self->version) + self->num_extra_header_bytes;
    write_intog(wtr, (const uint16_t *)&header_size);

    uint16_t total_vlr_byte_size = 0;
    for (uint32_t i = 0; i < self->number_of_vlrs; ++i)
    {
        total_vlr_byte_size += las_vlr_size(&self->vlrs[i]);
    }
    const uint32_t offset_to_point_data = (uint32_t)(header_size + total_vlr_byte_size);

    write_intog(wtr, &offset_to_point_data);
    write_intog(wtr, &self->number_of_vlrs);
    write_intog(wtr, &self->point_format.id);
    const uint16_t point_size = las_point_standard_size(self->point_format.id);
    write_intog(wtr, &point_size);
    write_intog(wtr, (const uint32_t *)&legacy_point_count);

    for (uint32_t i = 0; i < LAS_LEGACY_NUMBER_OF_POINTS_BY_RETURN_SIZE; ++i)
    {
        const uint32_t val = (self->number_of_points_by_return[i] > (uint64_t)UINT32_MAX)
                                 ? UINT32_MAX
                                 : self->number_of_points_by_return[i];
        write_intog(wtr, &val);
    }

    write_intog(wtr, &self->scaling.scales.x);
    write_intog(wtr, &self->scaling.scales.y);
    write_intog(wtr, &self->scaling.scales.z);

    write_intog(wtr, &self->scaling.offsets.x);
    write_intog(wtr, &self->scaling.offsets.y);
    write_intog(wtr, &self->scaling.offsets.z);

    write_intog(wtr, &self->maxs.x);
    write_intog(wtr, &self->mins.x);

    write_intog(wtr, &self->maxs.y);
    write_intog(wtr, &self->mins.y);

    write_intog(wtr, &self->maxs.z);
    write_intog(wtr, &self->mins.z);

    if (self->version.major == 1 && self->version.minor >= 3)
    {
        write_intog(wtr, &self->start_of_waveform_datapacket);
    }

    if (self->version.major == 1 && self->version.minor >= 4)
    {
        write_intog(wtr, &self->start_of_evlrs);
        write_intog(wtr, &self->number_of_evlrs);
        write_intog(wtr, &self->point_count);
        for (uint32_t i = 0; i < LAS_NUMBER_OF_POINTS_BY_RETURN_SIZE; ++i)
        {
            write_intog(wtr, &self->number_of_points_by_return[i]);
        }
    }

    LAS_DEBUG_ASSERT((wtr->ptr - &header_bytes[0]) == header_size);
    uint64_t n = las_dest_write(dest, &header_bytes[0], header_size);
    if (self->extra_header_bytes != NULL && self->num_extra_header_bytes > 0)
    {
        n += las_dest_write(dest, self->extra_header_bytes, self->num_extra_header_bytes);
    }

    if (n < header_size + self->num_extra_header_bytes)
    {
        las_err = las_dest_err(dest);
    }

    if (self->vlrs != NULL && self->number_of_vlrs != 0)
    {
        for (uint32_t i = 0; i < self->number_of_vlrs; ++i)
        {
            las_vlr_write_to(&self->vlrs[i], dest);
        }
    }

    return las_err;
}

static void las_vlr_array_deinit(las_vlr_t *vlrs, uint32_t size)
{
    LAS_DEBUG_ASSERT(vlrs != NULL);
    for (uint32_t i = 0; i < size; ++i)
    {
        las_vlr_deinit(&vlrs[i]);
    }
}

static void las_vlr_array_delete(las_vlr_t *vlrs, uint32_t size)
{
    if (vlrs == NULL)
    {
        return;
    }
    las_vlr_array_deinit(vlrs, size);
    free(vlrs);
}

int las_header_clone_into(const las_header_t *self, las_header_t *out_header)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(out_header != NULL);

    // Copy basic fields
    *out_header = *self;

    // Clone pointers
    las_vlr_t *vlrs = NULL;
    if (self->vlrs != NULL && self->number_of_vlrs != 0)
    {
        vlrs = malloc(self->number_of_vlrs * sizeof(las_vlr_t));
        if (vlrs == NULL)
        {
            return 1;
        }

        for (uint32_t i = 0; i < self->number_of_vlrs; ++i)
        {
            int failed = las_vlr_clone_into(&self->vlrs[i], &vlrs[i]);
            if (failed)
            {
                las_vlr_array_delete(vlrs, i);
                return 1;
            }
        }
    }

    uint8_t *extra_header_bytes = NULL;
    if (self->extra_header_bytes != NULL && self->num_extra_header_bytes != 0)
    {
        extra_header_bytes = malloc(sizeof(uint8_t) * self->num_extra_header_bytes);
        if (extra_header_bytes == NULL)
        {
            las_vlr_array_delete(vlrs, self->number_of_vlrs);
            return 1;
        }

        memcpy(extra_header_bytes, self->extra_header_bytes, self->num_extra_header_bytes);
    }

    out_header->vlrs = vlrs;
    out_header->extra_header_bytes = extra_header_bytes;

    return 0;
}

int las_header_clone(const las_header_t *self, las_header_t **out_header)
{
    LAS_DEBUG_ASSERT(self != NULL);
    LAS_DEBUG_ASSERT(out_header != NULL);

    las_header_t *header = malloc(sizeof(las_header_t));
    if (header == NULL)
    {
        return 1;
    }

    if (las_header_clone_into(self, header) != 0)
    {
        free(header);
        return 1;
    }

    *out_header = header;
    return 0;
}

void las_header_deinit(las_header_t *self)
{
    LAS_DEBUG_ASSERT(self != NULL);

    if (self->vlrs != NULL)
    {
        las_vlr_array_delete(self->vlrs, self->number_of_vlrs);
        self->vlrs = NULL;
        self->number_of_vlrs = 0;
    }
}

void las_header_delete(las_header_t *self)
{
    if (self == NULL)
    {
        return;
    }

    las_header_deinit(self);
    free(self);
}