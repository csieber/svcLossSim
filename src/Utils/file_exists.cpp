#include <stdio.h>
#include <string>
#include <inttypes.h>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include "file_exists.h"


bool file_exists(std::string name)
{
	FILE* file = fopen(name.c_str(), "r");

	if (file) {
		fclose(file);
		return true;
	}
	else
		return false;
}

std::string find_free_log_name(std::string file)
{
	const int32_t max_tries = 1000;

	int32_t counter = 0;
	std::string new_file;

	while (counter < max_tries) {

		new_file = boost::lexical_cast<std::string>(boost::format("%s.%d.txt") % file % counter++);

		if (file_exists(new_file))
			continue;
		else
			break;
	}

	if (counter == max_tries)
		return std::string();

	return new_file;
}
