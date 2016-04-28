#include "SimpleLogFile.h"
#include "file_exists.h"

SimpleLogFile::SimpleLogFile() {
}

SimpleLogFile::~SimpleLogFile() {
	file_.close();
}

SimpleLogFile::ptr SimpleLogFile::open(std::string name)
{
	SimpleLogFile::ptr file = SimpleLogFile::ptr(new SimpleLogFile());

	std::string log_name = find_free_log_name(name);

	file->file_.open(log_name.c_str(), std::ios_base::out);

	if (file->file_.fail())
		return SimpleLogFile::ptr();

	return file;
}

void SimpleLogFile::write(std::basic_ostream<char>& s)
{
	boost::mutex::scoped_lock locK(file_mt_);

	file_ << s.rdbuf();
}
