#include <las/las.h>

#include <inttypes.h>

int main(int argc, char * argv[]) {
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s FILE_PATH\n", argv[0]);
        return 1;
    }

    las_error_t las_err = { .kind = LAS_ERROR_OK };

    las_reader_t *reader;

    las_err = las_reader_open_file_path(argv[1], &reader);
    if (las_error_is_failure(&las_err))
    {
        goto out;
    }

    const las_header_t *hdr = las_reader_header(reader);

    printf("Version: %" PRIu8 ".%" PRIu8 "\n", hdr->version.major, hdr->version.minor);
    printf("Point Format ID: %" PRIu8 "\n", hdr->point_format.id);
    printf("Point Format Standard Size: %" PRIu16 "\n", las_point_standard_size(hdr->point_format.id));
    printf("Point Format Extra Bytes: %" PRIu16 "\n", hdr->point_format.num_extra_bytes);
    printf("Point Format Total Size: %" PRIu16 "\n", las_point_format_point_size(hdr->point_format));
    printf("Point Count: %" PRIu64 "\n", hdr->point_count);

    printf("\n");

    printf("Scaling X: {Scale: %f, Offset: %f}\n", hdr->scaling.scales.x, hdr->scaling.offsets.x);
    printf("Scaling Y: {Scale: %f, Offset: %f}\n", hdr->scaling.scales.y, hdr->scaling.offsets.y);
    printf("Scaling Z: {Scale: %f, Offset: %f}\n", hdr->scaling.scales.z, hdr->scaling.offsets.z);

    printf("\n");

    printf("Extent X: [%f, %f] -> %f\n", hdr->mins.x, hdr->maxs.x, hdr->maxs.x - hdr->mins.x);
    printf("Extent Y: [%f, %f] -> %f\n", hdr->mins.y, hdr->maxs.y, hdr->maxs.y - hdr->mins.y);
    printf("Extent Z: [%f, %f] -> %f\n", hdr->mins.z, hdr->maxs.z, hdr->maxs.z - hdr->mins.z);

    printf("\n");

    printf("File Source Id: %" PRIu16 "\n", hdr->file_source_id);
    printf("Global Encoding: %" PRIu16 "\n", hdr->global_encoding);
    printf("System Identifier: %.*s\n", LAS_SYSTEM_IDENTIFIER_SIZE, hdr->system_identifier);
    printf("Generating Software: %.*s\n", LAS_GENERATING_SOFTWARE_SIZE, hdr->generating_software);
    printf("Creation day: %" PRIu16 "\n", hdr->file_creation_day_of_year);
    printf("Creation year: %" PRIu16 "\n", hdr->file_creation_year);
    printf("Number of extra header bytes: %" PRIu32 "\n", hdr->num_extra_header_bytes);
    printf("Offset to point data: %" PRIu32 "\n", hdr->offset_to_point_data);

    if (hdr->version.major == 1 && hdr->version.minor == 3)
    {
        printf("Start of waveform data: %" PRIu64 "\n", hdr->start_of_waveform_datapacket);
    }

    if (hdr->version.major == 1 && hdr->version.minor == 4)
    {
        printf("Start of EVLRs: %" PRIu64 "\n", hdr->start_of_evlrs);
        printf("Number of EVLRs: %" PRIu64 "\n", hdr->start_of_evlrs);
    }

    printf("---\n");
    printf("Number of VLRs: %" PRIu32 "\n", hdr->number_of_vlrs);
    for (uint32_t i = 0; i < hdr->number_of_vlrs; ++i)
    {
        const las_vlr_t *vlr = &hdr->vlrs[i];
        printf("VLR %" PRIu32 " / %" PRIu32 "\n", i + 1, hdr->number_of_vlrs);
        printf("\tUser ID: %.*s\n", LAS_VLR_USER_ID_SIZE, vlr->user_id);
        printf("\tRecord ID: %" PRIu16 "\n", vlr->record_id);
        printf("\tDescription: %.*s\n", LAS_VLR_DESCRIPTION_SIZE, vlr->description);
        printf("\tData Size: %" PRIu16 "\n", vlr->data_size);
    }

out:
    las_reader_destroy(reader);

    return 0;
}
