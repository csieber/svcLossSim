#ifndef CONFIG_HPP_
#define CONFIG_HPP_

namespace config {


const int32_t default_max_pkt_size = 1000;

const int32_t max_pkt_size = 1000000;
const int32_t min_pkt_size = 50;

/*
 * Determining the correct size of the NAL
 * header is not that easy, that is why we
 * assume a certain size when we drop the
 * packet but want to keep the NAL header.
 *
 * NAL Header size: 1 byte
 * SVC Header size: 4 bytes
 *
 * To be safe we use 10 bytes.
 */
const int32_t default_assumed_nal_hdr_size = 20;

const int32_t max_assumed_nal_hdr_size = 100000;
const int32_t min_assumed_nal_hdr_size = 4;

const char nal_separator[] = {0x00, 0x00, 0x00, 0x01};

} /* namespace settings */

#endif /* CONFIG_HPP_ */
