#ifndef LAS_C_VLR_H
#define LAS_C_VLR_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define LAS_VLR_USER_ID_SIZE 16
#define LAS_VLR_DESCRIPTION_SIZE 32


/// Variable Length Record (VLR)
typedef struct las_vlr
{
    char user_id[LAS_VLR_USER_ID_SIZE];
    uint16_t record_id;
    char description[LAS_VLR_DESCRIPTION_SIZE];

    uint16_t data_size;
    /// Raws data bytes
    uint8_t *data;
} las_vlr_t;

/// Clones and returns the header int out_header
///
/// - returns 0 if everything was ok.
/// - returns 1 if allocation failed
int las_vlr_clone(const las_vlr_t *self, las_vlr_t **out_vlr);


#ifdef __cplusplus
}
#endif

#endif // LAS_C_VLR_H
