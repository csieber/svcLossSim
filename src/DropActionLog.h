#ifndef DROPACTIONLOG_H_
#define DROPACTIONLOG_H_

#include <string>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include "Utils/SimpleLogFile.h"
#include "h264Decoder/h264Decoder.h"
#include "LuaDropScript.h"

class DropActionLog : boost::noncopyable {

public:
	virtual ~DropActionLog();

	typedef boost::shared_ptr<DropActionLog> ptr;

	static DropActionLog::ptr create(std::string str);

	void log(H264::nal_short_info& nal, int32_t split_n, int32_t split_c, int32_t pkt_size,  LuaDropScript::ACTION& action);

private:
	DropActionLog();

	SimpleLogFile::ptr log_;
};

#endif /* DROPACTIONLOG_H_ */
