#ifndef SIMPLELOGFILE_H_
#define SIMPLELOGFILE_H_

#include <fstream>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <boost/thread/mutex.hpp>

/*
 * \class SimpleLogFile
 *
 * Use fs() for non-thread-safe behavior and
 * write for thread-safe behavior.
 *
 */
class SimpleLogFile {
public:

	typedef boost::shared_ptr<SimpleLogFile> ptr;

	virtual ~SimpleLogFile();

	static SimpleLogFile::ptr open(std::string name);

	std::fstream& fs() { return file_; };

	void write(std::basic_ostream<char>& s);

private:
	SimpleLogFile();

	std::fstream file_;

	boost::mutex file_mt_;
};

#endif /* SIMPLELOGFILE_H_ */
