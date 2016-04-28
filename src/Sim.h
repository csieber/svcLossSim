#ifndef SIM_H_
#define SIM_H_

#include <inttypes.h>
#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>

#ifdef __MINGW32__
# define BOOST_THREAD_USE_LIB
#endif
#include <boost/thread.hpp>

#include "h264Decoder/h264Decoder.h"
#include "config.hpp"
#include "LuaDropScript.h"
#include "DropActionLog.h"

class Sim : boost::noncopyable {
public:

	Sim();
	virtual ~Sim();

	int32_t run();
	bool waitForCompletion(int32_t timeout = -1);
	float progress();

	void svc_file(std::string file) { svc_file_ = file; };
	void lua_file(std::string file) { lua_file_ = file; };

	std::string svc_file() { return svc_file_; };
	std::string lua_file() { return lua_file_; };

	void setAssumedHdrSize(int32_t assumed_hdr_size) { settings_.assumed_hdr_size_ = assumed_hdr_size; };
	void setMaxPktSize(int32_t max_pkt_size) { settings_.max_pkt_size_ = max_pkt_size; };
	void useDbgLog();

private:

	void simThread();

	bool outputNal(H264::nal_short_info& nal);

	void write(const char* data, int32_t size);
	void writeZeros(int32_t size);

	bool sim_started_;

	std::string svc_file_;
	std::string lua_file_;

	LuaDropScript::ptr lua_wrapper_;
	H264::h264Decoder::ptr decoder_;

	DropActionLog::ptr action_log_;

	boost::shared_ptr<boost::thread> sim_thread_;

	struct settings {

		settings() {
			max_pkt_size_ = config::default_max_pkt_size;
			assumed_hdr_size_ = config::default_assumed_nal_hdr_size;
		}

		int32_t max_pkt_size_;
		int32_t assumed_hdr_size_;
	} settings_;

	struct statistics {

		statistics() {

			total_bytes_processed_ = 0;
			total_bytes_written_ = 0;
			total_zeros_written_ = 0;

			elapsed_ms_since_sim_start_ = 0;
			total_frames_processed_ = 0;
			total_nal_units_processed_ = 0;

			total_packets_ = 0;

			total_action_output_ = 0;
			total_action_drop_ = 0;
			total_action_zero_ = 0;
			total_action_drop_but_keep_hdr_ = 0;
			total_action_zero_but_keep_hdr_ = 0;
		}

		int32_t total_bytes_processed_;
		int32_t total_bytes_written_;
		int32_t total_zeros_written_;

		int32_t elapsed_ms_since_sim_start_;
		int32_t total_frames_processed_;
		int32_t total_nal_units_processed_;

		int32_t total_packets_;

		int32_t total_action_output_;
		int32_t total_action_drop_;
		int32_t total_action_zero_;
		int32_t total_action_drop_but_keep_hdr_;
		int32_t total_action_zero_but_keep_hdr_;
	} statistics_;
};

#endif /* SIM_H_ */
