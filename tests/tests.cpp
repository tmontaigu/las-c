#include <gtest/gtest.h>
#include <vector>

extern "C" {
#include <las/las.h>
#include <private/point.h>
}

/// Initialize all fields of a raw_point_10
/// which corresponds to point format id 5
static void init_raw_point_10(las_raw_point_10_t *rp) {
    rp->x = 1234;
    rp->y = -1234;
    rp->z = 56'757;

    rp->intensity = 43'564;

    rp->return_number = 5;
    rp->number_of_returns = 2;
    rp->scan_direction_flag = 1;
    rp->edge_of_flight_line = 1;

    rp->classification = 26;
    rp->synthetic = 1;
    rp->key_point = 1;
    rp->withheld = 1;

    rp->scan_angle_rank = 234;
    rp->user_data = 42;
    rp->point_source_id = 11'523;

    rp->gps_time = 54235.87;

    rp->red = 111;
    rp->green = 121;
    rp->blue = 311;

    rp->wave_packet.descriptor_index = 125;
    rp->wave_packet.byte_offset_to_data = 2'456'546;
    rp->wave_packet.size_in_bytes = 654'812;
    rp->wave_packet.return_point_waveform_location = 1.0;
    rp->wave_packet.xt = 23.4f;
    rp->wave_packet.yt = 21.4f;
    rp->wave_packet.zt = 11.3f;

    rp->num_extra_bytes = 4;
    rp->extra_bytes = (uint8_t *)std::malloc(rp->num_extra_bytes * sizeof(uint8_t));
    if (rp->extra_bytes != nullptr)
    {
        rp->extra_bytes[0] = 1;
        rp->extra_bytes[1] = 2;
        rp->extra_bytes[2] = 3;
        rp->extra_bytes[3] = 4;
    }
}
//
//TEST(PointCopy, raw_10_to_point)
//{
//    las_raw_point_t source_pt = {0};
//    source_pt.point_format_id = 5;
//    las_raw_point_10_t *rp = &source_pt.point10;
//
//    init_raw_point_10(rp);
//    ASSERT_NE(rp->extra_bytes, nullptr);
//
//    las_point_t dest_pt = {0};
//    dest_pt.num_extra_bytes = rp->num_extra_bytes;
//    dest_pt.extra_bytes = (uint8_t *)std::malloc(dest_pt.num_extra_bytes * sizeof(uint8_t));
//    ASSERT_NE(dest_pt.extra_bytes, nullptr);
//
//    las_point_copy_from_raw(&dest_pt, &source_pt, hea);
//
//    ASSERT_EQ(dest_pt.x, rp->x);
//    ASSERT_EQ(dest_pt.y, rp->y);
//    ASSERT_EQ(dest_pt.z, rp->z);
//
//    ASSERT_EQ(dest_pt.intensity, rp->intensity);
//    ASSERT_EQ(dest_pt.return_number, rp->return_number);
//    ASSERT_EQ(dest_pt.number_of_returns, rp->number_of_returns);
//
//    ASSERT_EQ(dest_pt.synthetic, rp->synthetic);
//    ASSERT_EQ(dest_pt.key_point, rp->key_point);
//    ASSERT_EQ(dest_pt.withheld, rp->withheld);
//    ASSERT_EQ(dest_pt.scan_direction_flag, rp->scan_direction_flag);
//    ASSERT_EQ(dest_pt.edge_of_flight_line, rp->edge_of_flight_line);
//
//    ASSERT_EQ(dest_pt.classification, rp->classification);
//
//    ASSERT_EQ(dest_pt.user_data, rp->user_data);
//    ASSERT_EQ(dest_pt.scan_angle, rp->scan_angle_rank);
//    ASSERT_EQ(dest_pt.point_source_id, rp->point_source_id);
//
//    ASSERT_EQ(dest_pt.red, rp->red);
//    ASSERT_EQ(dest_pt.green, rp->green);
//    ASSERT_EQ(dest_pt.blue, rp->blue);
//
//    ASSERT_EQ(las_wave_packet_eq(&dest_pt.wave_packet, &rp->wave_packet), 1);
//
//    ASSERT_EQ(dest_pt.num_extra_bytes, rp->num_extra_bytes);
//    ASSERT_EQ(memcmp(dest_pt.extra_bytes, rp->extra_bytes, sizeof(uint8_t) * dest_pt.num_extra_bytes), 0);
//
//    // Zero as it does not exist in legacy point format
//    ASSERT_EQ(dest_pt.overlap, 0);
//    ASSERT_EQ(dest_pt.scanner_channel, 0);
//    ASSERT_EQ(dest_pt.nir, 0);
//
//    las_point_deinit(&dest_pt);
//    las_raw_point_deinit(&source_pt);
//}
//
//TEST(PointCopy, raw_14_to_point)
//{
//    las_raw_point_t source_pt = {0};
//    source_pt.point_format_id = 10;
//    las_raw_point_14_t *rp = &source_pt.point14;
//
//    rp->x = 123.3;
//    rp->y = 454.234;
//    rp->z = 892.7545;
//
//    rp->intensity = 43'564;
//
//    rp->return_number = 5;
//    rp->number_of_returns = 2;
//    rp->scan_direction_flag = 1;
//    rp->edge_of_flight_line = 1;
//
//    rp->classification = 164;
//    rp->synthetic = 1;
//    rp->key_point = 1;
//    rp->withheld = 1;
//    rp->overlap = 1;
//    rp->scanner_channel = 3;
//
//    rp->scan_angle = 60'348;
//    rp->user_data = 42;
//    rp->point_source_id = 11'523;
//
//    rp->gps_time = 54235.87;
//
//    rp->red = 111;
//    rp->green = 121;
//    rp->blue = 311;
//
//    rp->nir = 245;
//
//    rp->wave_packet.descriptor_index = 125;
//    rp->wave_packet.byte_offset_to_data = 2'456'546;
//    rp->wave_packet.size_in_bytes = 654'812;
//    rp->wave_packet.return_point_waveform_location = 1.0;
//    rp->wave_packet.xt = 23.4f;
//    rp->wave_packet.yt = 21.4f;
//    rp->wave_packet.zt = 11.3f;
//
//    rp->num_extra_bytes = 4;
//    rp->extra_bytes = (uint8_t *)std::malloc(rp->num_extra_bytes * sizeof(uint8_t));
//    ASSERT_NE(rp->extra_bytes, nullptr);
//
//    las_point_t dest_pt = {0};
//    dest_pt.num_extra_bytes = rp->num_extra_bytes;
//    dest_pt.extra_bytes = (uint8_t *)std::malloc(dest_pt.num_extra_bytes * sizeof(uint8_t));
//    ASSERT_NE(dest_pt.extra_bytes, nullptr);
//    dest_pt.extra_bytes[0] = 1;
//    dest_pt.extra_bytes[1] = 2;
//    dest_pt.extra_bytes[2] = 3;
//    dest_pt.extra_bytes[3] = 4;
//
//    las_point_copy_from_raw(&dest_pt, &source_pt);
//
//    ASSERT_EQ(dest_pt.x, rp->x);
//    ASSERT_EQ(dest_pt.y, rp->y);
//    ASSERT_EQ(dest_pt.z, rp->z);
//
//    ASSERT_EQ(dest_pt.intensity, rp->intensity);
//    ASSERT_EQ(dest_pt.return_number, rp->return_number);
//    ASSERT_EQ(dest_pt.number_of_returns, rp->number_of_returns);
//
//    ASSERT_EQ(dest_pt.synthetic, rp->synthetic);
//    ASSERT_EQ(dest_pt.key_point, rp->key_point);
//    ASSERT_EQ(dest_pt.withheld, rp->withheld);
//    ASSERT_EQ(dest_pt.scan_direction_flag, rp->scan_direction_flag);
//    ASSERT_EQ(dest_pt.edge_of_flight_line, rp->edge_of_flight_line);
//
//    ASSERT_EQ(dest_pt.classification, rp->classification);
//
//    ASSERT_EQ(dest_pt.user_data, rp->user_data);
//    ASSERT_EQ(dest_pt.scan_angle, rp->scan_angle);
//    ASSERT_EQ(dest_pt.point_source_id, rp->point_source_id);
//
//    ASSERT_EQ(dest_pt.red, rp->red);
//    ASSERT_EQ(dest_pt.green, rp->green);
//    ASSERT_EQ(dest_pt.blue, rp->blue);
//
//    ASSERT_EQ(las_wave_packet_eq(&dest_pt.wave_packet, &rp->wave_packet), 1);
//
//    ASSERT_EQ(dest_pt.num_extra_bytes, rp->num_extra_bytes);
//    ASSERT_EQ(
//        memcmp(dest_pt.extra_bytes, rp->extra_bytes, sizeof(uint8_t) * dest_pt.num_extra_bytes), 0);
//
//    // Zero as is does not exist in legacy point format
//    ASSERT_EQ(dest_pt.overlap, rp->overlap);
//    ASSERT_EQ(dest_pt.scanner_channel, rp->scanner_channel);
//    ASSERT_EQ(dest_pt.nir, rp->nir);
//
//    las_point_deinit(&dest_pt);
//    las_raw_point_deinit(&source_pt);
//}

TEST(ReadWriteBuffers, WavePacketRoundrip) {
    las_wave_packet_t wave_packet;
    wave_packet.descriptor_index = 125;
    wave_packet.byte_offset_to_data = 2'456'546;
    wave_packet.size_in_bytes = 654'812;
    wave_packet.return_point_waveform_location = 1.0;
    wave_packet.xt = 23.4f;
    wave_packet.yt = 21.4f;
    wave_packet.zt = 11.3f;

    las_wave_packet_t output_wave_packet;
    uint8_t buffer[LAS_WAVE_PACKET_SIZE];
    las_wave_packet_to_buffer(&wave_packet, &buffer[0]);
    las_wave_packet_from_buffer(&buffer[0], &output_wave_packet);

    ASSERT_EQ(las_wave_packet_eq(&wave_packet, &output_wave_packet), 1);
}

TEST(ReadWriteBuffers, RawPoint10Roundrip)
{
    las_raw_point_10_t rp;
    init_raw_point_10(&rp);

    las_header_t header = {0};
    header.point_format.id = 5;
    header.point_format.num_extra_bytes = rp.num_extra_bytes;
    header.scaling.scales.x = 0.001;
    header.scaling.scales.y = 0.01;
    header.scaling.scales.z = 0.1;

    header.scaling.offsets.x = 0;
    header.scaling.offsets.y = 1;
    header.scaling.offsets.z = 2;

    std::vector<uint8_t> buffer;
    buffer.resize(las_point_format_point_size(header.point_format));


    las_raw_point_10_t output_point;
    output_point.extra_bytes = static_cast<uint8_t *>(malloc(sizeof(uint8_t) * rp.num_extra_bytes));
    ASSERT_NE(output_point.extra_bytes, nullptr);
    output_point.num_extra_bytes = rp.num_extra_bytes;
    las_raw_point_10_to_buffer(&rp, header.point_format, buffer.data());
    las_raw_point_10_from_buffer(buffer.data(), header.point_format, &output_point);

    ASSERT_DOUBLE_EQ(output_point.x, rp.x);
    ASSERT_DOUBLE_EQ(output_point.y, rp.y);
    ASSERT_DOUBLE_EQ(output_point.z, rp.z);
}