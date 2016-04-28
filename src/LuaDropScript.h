#ifndef LUADROPSCRIPT_H_
#define LUADROPSCRIPT_H_

#include <string>
#include <vector>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/unordered_map.hpp>
#include <boost/assign/list_of.hpp>
#include "lua-5.2.0/lua.hpp"

class LuaDropScript : boost::noncopyable {
public:

	/*
	 * Defines the actions the LUA drop() function is allowed to
	 * return.
	 *
	 * All actions must also be set in actionToStr at the beginning
	 * of the Sim.cpp file!
	 */
	enum ACTION {
		ACTION_INVALID = -1,
		ACTION_OUTPUT = 0,
		ACTION_DROP = 1,
		ACTION_ZERO = 2,
		ACTION_DROP_BUT_KEEP_HDR = 3,
		ACTION_ZERO_BUT_KEEP_HDR = 4
	};

	static boost::unordered_map<ACTION, std::string> actionToStr;

	typedef boost::shared_ptr<LuaDropScript> ptr;

	static LuaDropScript::ptr create(std::string file);

	int32_t initialiseLua();

	/*
	 * LUA functions
	 */
	int32_t callInitialise();

	ACTION callDrop(const int32_t& frame_nr,
					const int32_t& nal_nr,
					const int32_t& nal_type,
					const int32_t& nal_size,
					const boost::tuple<int32_t, int32_t, int32_t>& layer,
					const int32_t& pkt_size,
					const int32_t& pkt_n,
					const int32_t& pkt_c);

	void callFinished();

	std::string err_msg() { return err_msg_; };

	virtual ~LuaDropScript();

private:
	LuaDropScript();

	int32_t callNumericFunc(std::string name, std::vector<int32_t>& params);

	lua_State* lstate_;
	std::string file_;

	std::string err_msg_;
};

#endif /* LUADROPSCRIPT_H_ */
