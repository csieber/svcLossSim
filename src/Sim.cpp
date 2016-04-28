#include "Sim.h"
#include <iostream>

#ifdef _WIN32
# include <windows.h>
# include <fcntl.h> // _O_BINARY

  unsigned int _CRT_fmode = _O_BINARY;
#endif

#include "h264Decoder/NALStreams/NALFileStream.h"

Sim::Sim()
{
	sim_started_ = false;
}

Sim::~Sim()
{
}

int32_t Sim::run()
{
	/*
	 * Initializing LUA
	 */
	lua_wrapper_ = LuaDropScript::create(lua_file_);

	int32_t result = lua_wrapper_->initialiseLua();

	if (result != 0) {

		std::cerr << "Initializing LUA script failed! Error msg: " << lua_wrapper_->err_msg() << std::endl;
		return result;
	}

	result = lua_wrapper_->callInitialise();

	if (result != 0) {

		std::cerr << "Calling LUA initialize failed! Error msg: " << lua_wrapper_->err_msg() << std::endl;
		return result;
	}

	/*
	 * Initializing SVC bitstream
	 */
	using H264::NALFileStream;
	using H264::h264Decoder;
	using H264::nal_short_info;

	NALFileStream* stream = new NALFileStream();

	if (!stream->open(svc_file_)) {

		std::cerr << "h264: Failed to open " << svc_file_ << ". Exiting!" << std::endl;
		return false;
	}

	if (!stream->loadIndex()) {

		std::cerr << "h264: file index missing. Creating new one. This might take a while." << std::endl;

		if (!stream->createIndex()) {

			std::cerr << "h264: Failed to create index! Exiting!" << std::endl;
			return -1;
		}

		if (!stream->saveIndex()) {

			std::cerr << "h264: Failed to save index! Please check your file or folder permissions. Exiting" << std::endl;
			return -1;
		}
	}

	/*
	 * Start the sim thread
	 */
	decoder_ = h264Decoder::ptr(new h264Decoder(stream));;


	boost::thread* th = new boost::thread(
								boost::bind(&Sim::simThread, this));

	sim_thread_ = boost::shared_ptr<boost::thread>(th);

	return 0;
}

void Sim::useDbgLog()
{
	action_log_ = DropActionLog::create("dbg_actionlog");
}

void Sim::simThread()
{
	sim_started_ = true;

	using H264::nal_short_info;
	using H264::NALStream;

	NALStream::ptr stream = (NALStream::ptr)decoder_->stream();

	nal_short_info nal = decoder_->next();

	boost::posix_time::ptime start = boost::posix_time::microsec_clock::local_time();
	boost::posix_time::time_duration elapsed;

	int32_t frame_counter = 0;
	bool result = false;

	/*
	 * Loop through the whole file.
	 */
	while (nal.pNal.get<1>() != -1) {

		/*
		 * Loop through one frame.
		 */
		while (nal.pNal.get<1>() != -1) {

			if (frame_counter == nal.frame_num)
				break;

			result = outputNal(nal);

			if (!result)
				return;

			nal = decoder_->next();
		}

		frame_counter++;

		statistics_.total_frames_processed_++;

		elapsed = boost::posix_time::microsec_clock::local_time() - start;

		statistics_.elapsed_ms_since_sim_start_ = (int32_t)elapsed.total_milliseconds();
	}

	lua_wrapper_->callFinished();

	std::cerr
	<< "Simulation finished." << std::endl << std::endl
	<< "Total elapsed time: " << statistics_.elapsed_ms_since_sim_start_ << " ms." << std::endl
	<< "Total frames processed: " << statistics_.total_frames_processed_ << std::endl
	<< "Total NAL units processed: " << statistics_.total_nal_units_processed_ << std::endl
	<< "Total Bytes processed: " << statistics_.total_bytes_processed_ << " (including NAL separator)" << std::endl
	<< "Total Bytes written: " << statistics_.total_bytes_written_
		<< " ( including " << statistics_.total_zeros_written_ << " 'zeroed' bytes)"<< std::endl << std::endl

	<< "Actions taken (for the " << statistics_.total_packets_ << " splitted NAL chunks ('packets')):" << std::endl
	<< "ACTION_OUTPUT: " << statistics_.total_action_output_
		<< " (" << ((float)statistics_.total_action_output_/(float)statistics_.total_packets_) * 100 << "%)" << std::endl
	<< "ACTION_DROP: " << statistics_.total_action_drop_
		<< " (" << ((float)statistics_.total_action_drop_/(float)statistics_.total_packets_) * 100 << "%)" << std::endl
	<< "ACTION_ZERO: " << statistics_.total_action_zero_
		<< " (" << ((float)statistics_.total_action_zero_/(float)statistics_.total_packets_) * 100 << "%)" << std::endl
	<< "ACTION_DROP_BUT_KEEP_HDR: " << statistics_.total_action_drop_but_keep_hdr_
		<< " (" << ((float)statistics_.total_action_drop_but_keep_hdr_/(float)statistics_.total_packets_) * 100 << "%)" << std::endl
	<< "ACTION_ZERO_BUT_KEEP_HDR: " << statistics_.total_action_zero_but_keep_hdr_
		<< " (" << ((float)statistics_.total_action_zero_but_keep_hdr_/(float)statistics_.total_packets_) * 100 << "%)" << std::endl

	<< std::endl
	<< "Settings: " << std::endl
	<< "Maximum chunk ('packet') size: " << settings_.max_pkt_size_ << " bytes (excluding NAL separator)" << std::endl
	<< "Assumed NAL header size: " << settings_.assumed_hdr_size_ << " bytes" << std::endl
	;

}

bool Sim::outputNal(H264::nal_short_info& nal)
{
	statistics_.total_nal_units_processed_++;
	statistics_.total_bytes_processed_ += nal.pNal.get<1>() + sizeof(config::nal_separator);

	int32_t nal_bytes_sent = 0;
	int32_t bytes_to_send  = 0;
	bool    nal_start      = true;

	int32_t pkt_split_c = (nal.pNal.get<1>() / settings_.max_pkt_size_) + 1;
	int32_t pkt_split_n = 0;

	LuaDropScript::ACTION drop_action = LuaDropScript::ACTION_INVALID;

	/*
	 * Send the NAL unit
	 */
	nal_bytes_sent = 0;

	while (nal_bytes_sent != nal.pNal.get<1>()) {

		pkt_split_n++;
		statistics_.total_packets_++;

		bytes_to_send = std::min(settings_.max_pkt_size_, nal.pNal.get<1>() - nal_bytes_sent);


		/*
		 * Decide what to do with the NAL unit by calling the LUA script.
		 */
		boost::tuple<int32_t, int32_t, int32_t> layer =
				boost::make_tuple(nal.dependency_id, nal.temporal_id, nal.quality_id);

		drop_action = lua_wrapper_->callDrop(
							nal.frame_num,
							nal.nal_nr,
							nal.type,
							nal.size,
							layer,
							bytes_to_send,
							pkt_split_n,
							pkt_split_c);

		switch (drop_action) {

		/*
		 * Action: OUTPUT
		 *
		 * Outputs the packet.
		 *
		 */
		case LuaDropScript::ACTION_OUTPUT:

			if (nal_start) {

				/*
				 * We have to add the NAL separator for the first packet of a NAL here, because
				 * the decoder removes it from the stream.
				 */
				write(config::nal_separator, sizeof(config::nal_separator));
			}

			write(&(nal.pNal.get<0>()[nal_bytes_sent]), bytes_to_send);

			statistics_.total_action_output_++;

		break;

		/*
		 * Action: DROP
		 *
		 * Drops the packet regardless of any of its properties.
		 * Especially for the first packet of a NAL this may be
		 * really bad for the stream.
		 *
		 */
		case LuaDropScript::ACTION_DROP:
			/* do nothing */

			statistics_.total_action_drop_++;
		break;

		/*
		 * Action: ZERO
		 *
		 * Instead of outputting the packet we output zeros. This keeps
		 * the size of the output file constant and may help the decoder
		 * to play the damaged stream.
		 */
		case LuaDropScript::ACTION_ZERO:

			if (nal_start)
				writeZeros(sizeof(config::nal_separator));

			writeZeros(bytes_to_send);

			statistics_.total_action_zero_++;
		break;

		/*
		 * Action DROP_BUT_KEEP_HDR
		 *
		 * The packet is dropped as with ACTION_DROP, but if the packet is the
		 * first of a NAL unit we keep the NAL separator and the NAL header.
		 */
		case LuaDropScript::ACTION_DROP_BUT_KEEP_HDR:

			statistics_.total_action_drop_but_keep_hdr_++;

			// If it is not the first packet of the NAL unit we drop the entire
			// packet.
			if (!nal_start)
				break;

			// If it is the first packet, we output the NAL separator and the NAL
			// header.
			write(config::nal_separator, sizeof(config::nal_separator));
			write(
				&(nal.pNal.get<0>()[nal_bytes_sent]),
				std::min(settings_.assumed_hdr_size_, bytes_to_send));
		break;

		/*
		 * Action: ZERO_BUT_KEEP_HDR
		 *
		 * The packet is 'zeroed' as with ACTION_ZERO, but as with DROP_BUT_KEEP_HDR we
		 * keep the NAL separator and NAL header if it is the first packet of a NAL unit.
		 */
		case LuaDropScript::ACTION_ZERO_BUT_KEEP_HDR:

			statistics_.total_action_zero_but_keep_hdr_++;

			// If it is not the first packet of the NAL unit we zero the entire
			// packet.
			if (!nal_start) {

				writeZeros(bytes_to_send);
				break;
			}

			// If it is the first packet, we output the NAL separator, the NAL header
			// and fill the rest with zeros.
			write(config::nal_separator, sizeof(config::nal_separator));
			write(
				&(nal.pNal.get<0>()[nal_bytes_sent]),
				std::min(settings_.assumed_hdr_size_, bytes_to_send));

			writeZeros(bytes_to_send - std::min(settings_.assumed_hdr_size_, bytes_to_send));

		break;

		default:
			std::cerr << "Invalid LUA drop action '" << drop_action << "'. Exiting." << std::endl;
			return false;
		}

		nal_bytes_sent += bytes_to_send;

		nal_start = false;

		if (action_log_)
			action_log_->log(nal, pkt_split_n, pkt_split_c, bytes_to_send, drop_action);
	}

	return true;
}

void Sim::write(const char* data, int32_t size)
{
	std::cout.write(data, size);

	statistics_.total_bytes_written_ += size;
}

void Sim::writeZeros(int32_t size)
{
	statistics_.total_bytes_written_ += size;
	statistics_.total_zeros_written_ += size;

	const char zero = 0x00;

	while (size-- > 0)
		std::cout.write(&zero, sizeof(zero));
}

bool Sim::waitForCompletion(int32_t timeout)
{
	sim_thread_->join();

	return true;
}

float Sim::progress()
{
	return 0;
}
