#include "DropActionLog.h"

const char hdr[] = "\
|    NAL | Type |   Size |  Frame | ( D/ T/ Q) | Split n/c | Pkt size |                 ACTION taken |\n\
|----------------------------------------------------------------------------------------------------|\n\
";

const char row_write[] = "\
| %6d |   %2d | %6d |  %5d | (%2d/%2d/%2d) | %4d/%4d | %8d | %24s (%d) |\n\
";

DropActionLog::DropActionLog() {
}

DropActionLog::~DropActionLog() {

}

DropActionLog::ptr DropActionLog::create(std::string str)
{
	DropActionLog::ptr ptr = DropActionLog::ptr(new DropActionLog());

	ptr->log_ = SimpleLogFile::open(str);

	if (!ptr->log_)
		return DropActionLog::ptr();

	ptr->log_->fs() << hdr;

	return ptr;
}

void DropActionLog::log(H264::nal_short_info& nal, int32_t split_n, int32_t split_c, int32_t pkt_size, LuaDropScript::ACTION& action)
{
	char buff[500];

	sprintf(buff, row_write, nal.nal_nr
			               , nal.type
						   , nal.size
						   , nal.frame_num
						   , nal.dependency_id
						   , nal.temporal_id
						   , nal.quality_id
						   , split_n
						   , split_c
						   , pkt_size
						   , LuaDropScript::actionToStr[action].c_str()
						   , action);

	log_->fs() << buff;
}
