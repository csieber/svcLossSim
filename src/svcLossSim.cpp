#include <iostream>
#include <boost/program_options.hpp>
#include "Sim.h"
#include "config.hpp"

int main(int argc, char** argv) {

	/*
	 * Read command line options.
	 */
	namespace po = boost::program_options;

	bool dbg_log = false;

	po::options_description desc("Allowed options");
	desc.add_options()
	    ("help", "Help")
	    ("lua,l", po::value<std::string>()->required(), "The LUA drop script to use.")
		("svc,s", po::value<std::string>()->required(), "The SVC bitstream file.")
		("hdr-size,h", po::value<int32_t>()->default_value(config::default_assumed_nal_hdr_size), "Assumed NAL header size.")
		("pkt-size,p", po::value<int32_t>()->default_value(config::default_max_pkt_size), "Maximum packet/chunk size.")
		("dbglog,d", po::bool_switch(&dbg_log), "Write action debug log for this run.");


	po::variables_map vm;

	try {
		po::store(po::parse_command_line(argc, argv, desc), vm);
		po::notify(vm);
	}
	catch (boost::program_options::required_option& ex) {

		std::cerr << "Required option '" << ex.get_option_name()
				  << "' is missing!" << std::endl << std::endl
				  << desc << std::endl;

		return 1;
	}
	catch (boost::program_options::invalid_option_value& ex) {

		std::cerr << "Option '" << ex.get_option_name()
				  << "' has an invalid value!" << std::endl << std::endl
				  << desc << std::endl;

		return 1;
	}
	catch (boost::program_options::unknown_option& ex) {

		std::cerr << "Unknown option '" << ex.get_option_name()
				  << "'!" << std::endl << std::endl
				  << desc << std::endl;

		return 1;
	}
	catch (boost::program_options::invalid_command_line_syntax& ex) {

		std::cerr << "Invalid syntax! What(): '" << ex.what() << std::endl << std::endl
				  << desc << std::endl;

		return 1;
	}

	if (vm.count("help")) {
	    std::cout << desc << "\n";
	    return 1;
	}

	Sim simulation;

	simulation.lua_file(vm["lua"].as<std::string>());
	simulation.svc_file(vm["svc"].as<std::string>());

	int32_t assumed_hdr_size = vm["hdr-size"].as<int32_t>();

	if (assumed_hdr_size < config::min_assumed_nal_hdr_size || assumed_hdr_size > config::max_assumed_nal_hdr_size) {

		std::cerr << "Assumed header size must be at least "<< config::min_assumed_nal_hdr_size
				<<" and less then "<< config::max_assumed_nal_hdr_size <<" Bytes long." << std::endl;
		return 1;
	}
	else
		simulation.setAssumedHdrSize(assumed_hdr_size);

	int32_t max_pkt_size = vm["pkt-size"].as<int32_t>();

	if (max_pkt_size < config::min_pkt_size || max_pkt_size > config::max_pkt_size) {

		std::cerr << "Maximum packet size must be at least " << config::min_pkt_size
				<< " and less then " << config::max_pkt_size << " Bytes long." << std::endl;

		return 1;
	}

	simulation.setMaxPktSize(max_pkt_size);

	if (assumed_hdr_size > max_pkt_size) {

		std::cerr << "Warning: The assumed header size is limited by the maximum packet size!"
				<< std::endl;
	}

	if (dbg_log)
		simulation.useDbgLog();

	int32_t result = simulation.run();

	if (result != 0)
		return result;

	simulation.waitForCompletion();

	if (simulation.progress() != 100)
		return 1;

	return 0;
}
