#ifndef LAS_C_PRIV_HEADER_H
#define LAS_C_PRIV_HEADER_H

#include "las/error.h"
#include "las/header.h"

#include "dest.h"
#include "point.h"
#include "source.h"

void las_vlr_deinit(las_vlr_t *self);

las_error_t
las_header_read_from(las_source_t *source, las_header_t *header, bool *is_data_compressed);

las_error_t las_header_write_to(const las_header_t *self, las_dest_t *dest);

las_error_t las_header_validate(const las_header_t *self);

/// When writing we are more strict than when reading a file.
/// eg, we want to make sure the point format is compatible with the version,
/// check the header size
///
/// Implies `las_header_validate` as it calls it.
las_error_t las_header_validate_for_writing(const las_header_t *self);

void las_header_deinit(las_header_t *self);

const las_vlr_t *las_header_find_laszip_vlr(const las_header_t *las_header);

#endif // LAS_C_PRIV_HEADER_H
