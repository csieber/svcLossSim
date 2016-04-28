#include "LuaDropScript.h"
#include <boost/foreach.hpp>


boost::unordered_map<LuaDropScript::ACTION, std::string>
	LuaDropScript::actionToStr
			= boost::assign::map_list_of
				(LuaDropScript::ACTION_INVALID			, "ACTION_INVALID")
				(LuaDropScript::ACTION_OUTPUT			, "ACTION_OUTPUT")
				(LuaDropScript::ACTION_DROP			    , "ACTION_DROP")
				(LuaDropScript::ACTION_ZERO             , "ACTION_ZERO")
				(LuaDropScript::ACTION_DROP_BUT_KEEP_HDR, "ACTION_DROP_BUT_KEEP_HDR")
				(LuaDropScript::ACTION_ZERO_BUT_KEEP_HDR, "ACTION_ZERO_BUT_KEEP_HDR");

LuaDropScript::LuaDropScript()
{
	lstate_ = NULL;
}

LuaDropScript::~LuaDropScript()
{
	if (lstate_) {

		lua_close(lstate_);
		lstate_ = NULL;
	}
}

LuaDropScript::ptr LuaDropScript::create(std::string file)
{
	LuaDropScript::ptr script = LuaDropScript::ptr(new LuaDropScript());

	script->file_ = file;

	return script;
}

int32_t LuaDropScript::initialiseLua()
{
	lstate_ = luaL_newstate();

	if (!lstate_) {

		err_msg_ = "Failed to create LUA state!";
		return -1;
	}

	luaL_openlibs(lstate_);

	int32_t err_code = luaL_loadfile(lstate_, file_.c_str());

	if (err_code == LUA_ERRFILE ) {

		err_msg_ = "Failed to open LUA file!";
		lstate_ = NULL;
		return 1;
	}

	err_code = lua_pcall(lstate_, 0, LUA_MULTRET, 0);

	if (err_code != LUA_OK) {

		err_msg_ = "Failed to call LUA script!\n";
		err_msg_ += lua_tostring(lstate_, -1);
		lstate_ = NULL;
		return 2;
	}

	return 0;
}

int32_t LuaDropScript::callInitialise()
{
	std::vector<int32_t> args;

	int32_t result = callNumericFunc("initialise", args);

	return result;
}

LuaDropScript::
ACTION LuaDropScript::callDrop(const int32_t& frame_nr,
							   const int32_t& nal_nr,
							   const int32_t& nal_type,
							   const int32_t& nal_size,
							   const boost::tuple<int32_t, int32_t, int32_t>& layer,
							   const int32_t& pkt_size,
							   const int32_t& pkt_n,
							   const int32_t& pkt_c)
{

	std::vector<int32_t> args;

	args.push_back(frame_nr);
	args.push_back(nal_nr);
	args.push_back(nal_type);
	args.push_back(nal_size);
	args.push_back(layer.get<0>());
	args.push_back(layer.get<1>());
	args.push_back(layer.get<2>());
	args.push_back(pkt_size);
	args.push_back(pkt_n);
	args.push_back(pkt_c);

	int32_t result = callNumericFunc("drop", args);

    switch (result) {
    	case ACTION_INVALID:
    		return ACTION_INVALID;
    	break;
		case ACTION_OUTPUT:
			return ACTION_OUTPUT;
		break;
		case ACTION_DROP:
			return ACTION_DROP;
		break;
		case ACTION_ZERO:
			return ACTION_ZERO;
		break;
		case ACTION_DROP_BUT_KEEP_HDR:
			return ACTION_DROP_BUT_KEEP_HDR;
		break;

		case ACTION_ZERO_BUT_KEEP_HDR:
			return ACTION_ZERO_BUT_KEEP_HDR;
		break;
		default:
			err_msg_ = "LUA 'drop' returned invalid/unknown value!";
			return ACTION_INVALID;
    }

    return ACTION_INVALID;
}

void LuaDropScript::callFinished()
{
	std::vector<int32_t> args;

	callNumericFunc("finished", args);
}

/*
 * Calls a given LUA numeric function. At the moment this is limited to n int32_t input
 * parameters and 1 int32_t output parameter.
 *
 * \return
 * 			< -1 : Values below -1 are indicating errors cases by LUA itself.
 * 				   Example: The given function could not be found in the LUA script.
 *
 * 			  -1 : A value of -1 indicates, that the LUA function could be called,
 * 			  	   but it was not able complete successfully.
 *
 * 			>= 0 : Values equal or above 0 are returned by the LUA script itself and
 * 				   are just forwarded.
 *
 */
int32_t LuaDropScript::callNumericFunc(std::string name, std::vector<int32_t>& params)
{
	if (!lstate_) {

		err_msg_ = "LUA drop script called without being properly initialised!";
		return -2;
	}

	lua_getglobal(lstate_, name.c_str());

	if(!lua_isfunction(lstate_, -1)) {

		err_msg_ = "LUA function '" + name +"' not found!";
		lua_pop(lstate_,1);
	    return -3;
	}

	BOOST_FOREACH(std::vector<int32_t>::value_type& value, params) {

		lua_pushnumber(lstate_, value);
	}

    if (lua_pcall(lstate_, params.size(), 1, 0) != 0) {

    	err_msg_ += "LUA: ";
    	err_msg_ += std::string(lua_tostring(lstate_, -1));
        return -4;
    }

    if (!lua_isnumber(lstate_, -1)) {

    	err_msg_ = "LUA '" + name + "' script did not return a number!";
    	return -5;
    }

    int32_t result = lua_tonumber(lstate_, -1);

    lua_pop(lstate_, 1);

    if (result < -1) {

    	err_msg_ = "Violation: LUA scripts must not return numbers < - 1. Those are reserved for internal use!";
    	return -6;
    }

	return result;
}

