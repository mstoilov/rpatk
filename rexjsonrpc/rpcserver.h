/*
 *  Sigmadrone
 *  Copyright (c) 2013-2015 The Sigmadrone Developers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Martin Stoilov <martin@sigmadrone.org>
 *  Svetoslav Vassilev <svassilev@sigmadrone.org>
 */
#ifndef _RPCSERVER_H_
#define _RPCSERVER_H_

#include <string>
#include <map>
#include <stdexcept>
#include <iostream>
#include <functional>
#include "rexjson/rexjson++.h"

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) (sizeof(x)/sizeof(x[0]))
#endif

namespace rexjson {

enum rpc_error_code
{
    // Standard JSON-RPC 2.0 errors
    RPC_INVALID_REQUEST  = -32600,
    RPC_METHOD_NOT_FOUND = -32601,
    RPC_INVALID_PARAMS   = -32602,
    RPC_INTERNAL_ERROR   = -32603,
    RPC_PARSE_ERROR      = -32700,

    // General application defined errors
    RPC_MISC_ERROR                  = -1,  // std::exception thrown in command handling
};

/*
 * The rpc types correspond directly to the json value types like obj_type, array_type ...
 * but they have values 2 pow n, so we can store them in unsigned int like (rpc_str_type|rpc_null_type)
 * Then we use a vector of these bit masks to specify the parameter types we expect for
 * the rpc calls.
 *
 * get_rpc_type will convert rexjson::value_type to one of the rpc types.
 *
 * create_json_spec will convert the rpc_type bit mask to an integer or array of integers
 * correspoinding to rexjson value_type(s)
 *
 * rpc_types will convert json spec to a std::vector of rpc_type_ bitmasks.
 *
 */
static const unsigned int rpc_null_type = 1;
static const unsigned int rpc_obj_type = 2;
static const unsigned int rpc_array_type = 4;
static const unsigned int rpc_str_type = 8;
static const unsigned int rpc_bool_type = 16;
static const unsigned int rpc_int_type = 32;
static const unsigned int rpc_real_type = 64;


enum rpc_exec_mode {
	execute = 0, 	// normal execution
	spec,			// produce machine readable parameter specification
	helpspec, 		// produce a human readable parameter specification
	help, 			// produce a help message
};

template <typename T> inline unsigned int get_rpc_type(T) { return rpc_int_type; }
template <> inline unsigned int get_rpc_type<bool>(bool) { return rpc_bool_type; }
template <> inline unsigned int get_rpc_type<double>(double) { return rpc_real_type; }
template <> inline unsigned int get_rpc_type<float>(float) { return rpc_real_type; }
template <> inline unsigned int get_rpc_type<int>(int) { return rpc_int_type; }
template <> inline unsigned int get_rpc_type<char>(char) { return rpc_int_type; }
template <> inline unsigned int get_rpc_type<std::string>(std::string) { return rpc_str_type; }


template<typename Ret, typename ...Args>
struct rpc_wrapperbase
{
	rpc_wrapperbase(const std::function<Ret(Args...)>& f, std::string help_msg)
		: f_(f)
		, help_msg_(help_msg)
	{
		json_types_ = make_types_array(std::tuple<Args...>(), std::index_sequence_for<Args...>());
	}

	rexjson::value call(const rexjson::array& params, rpc_exec_mode mode)
	{
		auto is = std::index_sequence_for<Args...>();
		std::tuple<Args...> tuple = params_to_tuple(params, is);
		return call_with_tuple(tuple, is);
	}

	rexjson::value operator()(const rexjson::array& params, rpc_exec_mode mode)
	{
		return call(params, mode);
	}

protected:
	template<std::size_t... is>
	std::array<unsigned int, sizeof...(is)> make_types_array(const std::tuple<Args...>& tuple, std::index_sequence<is...>)
	{
		return {{get_rpc_type(std::get<is>(tuple))...}};
	}

	template<std::size_t... is>
	std::tuple<Args...> params_to_tuple(const rexjson::array& params, std::index_sequence<is...>)
	{
		return std::make_tuple(Args{params[is].get_value<Args>()}...);
	}

	template<std::size_t... is>
	Ret call_with_tuple(const std::tuple<Args...>& tuple, std::index_sequence<is...>)
	{
		return f_(std::get<is>(tuple)...);
	}

	std::function<Ret(Args...)> f_;

public:
	std::string help_msg_;
	std::array<unsigned int, sizeof...(Args)> json_types_;
};

template<typename Ret, class ...Args>
struct rpc_wrapper : public rpc_wrapperbase<Ret, Args...>
{
	using base = rpc_wrapperbase<Ret, Args...>;
	using base::base;
};

template<class ...Args>
struct rpc_wrapper<void, Args...> : public rpc_wrapperbase<void, Args...>
{
	using base = rpc_wrapperbase<void, Args...>;
	using base::base;

	rexjson::value call(const rexjson::array& params, rpc_exec_mode mode)
	{
		auto is = std::index_sequence_for<Args...>();
		std::tuple<Args...> tuple = base::params_to_tuple(params, is);
		base::call_with_tuple(tuple, is);
		return std::string();
	}

	rexjson::value operator()(const rexjson::array& params, rpc_exec_mode mode)
	{
		return call(params, mode);
	}

};


template<typename Ret, class ...Args>
rpc_wrapper<Ret, Args...> make_rpc_wrapper(Ret (&f)(Args...), std::string help_msg = std::string())
{
	return rpc_wrapper<Ret, Args...>(f, help_msg);
}

template<typename Ret, typename C, typename ...Args>
rpc_wrapper<Ret, Args...> make_rpc_wrapper(C* object, Ret (C::*f)(Args...), std::string help_msg = std::string())
{
	return rpc_wrapper<Ret, Args...>([=](Args... args)->Ret {return (object->*f)(args...);}, help_msg);
}

template<typename Ret, typename C, typename ...Args>
rpc_wrapper<Ret, Args...> make_rpc_wrapper_const(const C* object, Ret (C::*f)(Args...) const, std::string help_msg = std::string())
{
	return rpc_wrapper<Ret, Args...>([=](Args... args)->Ret {return (object->*f)(args...);}, help_msg);
}


struct rpc_server_dummy { };

template<typename T = rpc_server_dummy>
class rpc_server
{
public:

	using rpc_method_type = std::function<rexjson::value(rexjson::array& params, rpc_exec_mode mode)>;
	typedef std::map<std::string, rpc_method_type> method_map_type;

	rpc_server()
	{
		add("help", this, &rpc_server::rpc_help);
		add("spec", &rpc_server::rpc_spec);
		add("helpspec", &rpc_server::rpc_helpspec);
	}

	~rpc_server()
	{

	}

	rexjson::value rpc_spec(rexjson::array& params, rpc_exec_mode mode)
	{
		static unsigned int types[] = { rpc_str_type };
		if (mode != execute) {
			if (mode == spec)
				return create_json_spec(types, ARRAYSIZE(types));
			if (mode == helpspec)
				return create_json_helpspec(types, ARRAYSIZE(types));
			return
					"spec <\"name\">\n"
					"\nGet the spec for the specified rpc name.\n"
					"\nArguments:\n"
					"1. \"name\"     (string, required) The name of of the rpc method to get the spec on\n"
					"\nResult:\n"
					"\"json\"     (string) The rpc call spec in json\n";
		}

		verify_parameters(params, types, ARRAYSIZE(types));
		rexjson::array ignored;
		return call_method_name(params[0], ignored, spec);
	}

	rexjson::value rpc_helpspec(rexjson::array& params, rpc_exec_mode mode)
	{
		static unsigned int types[] = { rpc_str_type };
		if (mode != execute) {
			if (mode == spec)
				return create_json_spec(types, ARRAYSIZE(types));
			if (mode == helpspec)
				return create_json_helpspec(types, ARRAYSIZE(types));
			return
					"helpspec <\"name\">\n"
					"\nGet the helpspec for the specified rpc name.\n"
					"\nArguments:\n"
					"1. \"name\"     (string, required) The name of of the rpc method to get the spec on\n"
					"\nResult:\n"
					"\"json\"     (string) The rpc call helpspec\n";
		}

		verify_parameters(params, types, ARRAYSIZE(types));
		rexjson::array ignored;
		return call_method_name(params[0], ignored, helpspec);
	}


	rexjson::value rpc_help(rexjson::array& params, rpc_exec_mode mode)
	{
		static unsigned int types[] = { (rpc_str_type | rpc_null_type) };
		if (mode != execute) {
			if (mode == spec)
				return create_json_spec(types, ARRAYSIZE(types));
			if (mode == helpspec)
				return create_json_helpspec(types, ARRAYSIZE(types));
			return
					"help [\"command\"]\n"
					"\nList all commands, or get help for a specified command.\n"
					"\nArguments:\n"
					"1. \"command\"     (string, optional) The command to get help on\n"
					"\nResult:\n"
					"\"text\"     (string) The help text\n";
		}

		verify_parameters(params, types, ARRAYSIZE(types));
		if (params[0].type() == rexjson::null_type) {
			std::string result;
			for (typename method_map_type::const_iterator it = map_.begin(); it != map_.end(); it++) {
				result += it->first + "\n";

//				rexjson::array ignored;
//				std::string ret = call_method_name(rexjson::value(it->first), ignored, help).get_str();
//				ret = ret.substr(ret.find('\n') + 1);
//				result += ret.substr(0, ret.find('\n')) + "\n";
			}
			return result;
		}
		rexjson::array ignored;
		return call_method_name(params[0], ignored, help);
	}


	static unsigned int get_rpc_type(rexjson::value_type value_type)
	{
		static const unsigned int rpc_types[] = {rpc_null_type, rpc_obj_type, rpc_array_type, rpc_str_type, rpc_bool_type, rpc_int_type, rpc_real_type};
		return rpc_types[value_type];
	}

	static rexjson::value create_json_spec(unsigned int *arr, size_t n)
	{
		rexjson::array params;

		for (size_t i = 0; i < n; i++) {
			rexjson::array param;
			if (arr[i] & rpc_obj_type)
				param.push_back(rexjson::obj_type);
			if (arr[i] & rpc_array_type)
				param.push_back(rexjson::array_type);
			if (arr[i] & rpc_str_type)
				param.push_back(rexjson::str_type);
			if (arr[i] & rpc_bool_type)
				param.push_back(rexjson::bool_type);
			if (arr[i] & rpc_int_type)
				param.push_back(rexjson::int_type);
			if (arr[i] & rpc_real_type)
				param.push_back(rexjson::real_type);
			if (arr[i] & rpc_null_type)
				param.push_back(rexjson::null_type);
			params.push_back((param.size() > 1) ? param : param[0]);
		}
		return params;
	}

	static std::vector<unsigned int> rpc_types(const rexjson::array& spec)
	{
		std::vector<unsigned int> ret;

		for (size_t i = 0; i < spec.size(); i++) {
			if (spec[i].get_type() == rexjson::array_type) {
				unsigned int rpc_types = 0;
				for (size_t j = 0; j < spec[i].get_array().size(); j++) {
					rpc_types |= get_rpc_type((rexjson::value_type)spec[i].get_array()[j].get_int());
				}
				ret.push_back(rpc_types);
			} else {
				ret.push_back(get_rpc_type((rexjson::value_type)spec[i].get_int()));
			}
		}
		return ret;
	}

	static std::vector<unsigned int> rpc_types(const std::string strspec)
	{
		rexjson::value jsonspec;

		jsonspec.read(strspec);
		return rpc_types(jsonspec.get_array());
	}

	static rexjson::value create_json_helpspec(unsigned int *arr, size_t n)
	{
		rexjson::value ret = create_json_spec(arr, n);
		convert_types_to_strings(ret);
		return ret;
	}

	static rexjson::value noexec(rexjson::array& params, rpc_exec_mode mode, unsigned int *types, size_t ntypes, const std::string& help_msg)
	{
		if (mode == spec)
			return create_json_spec(types, ntypes);
		if (mode == helpspec)
			return create_json_helpspec(types, ntypes);
		return help_msg;
	}

	static rexjson::object create_rpc_error(
			rpc_error_code code,
			const std::string& message)
	{
		rexjson::object error;
		error["code"] = code;
		error["message"] = message;
		return error;
	}

	void add(const std::string& name, const rpc_method_type& method)
	{
		if (name.empty())
			throw std::runtime_error("rpc_server::add, invalid name parameter");
		if (!method)
			throw std::runtime_error("rpc_server::add, invalid method parameter");
		map_[name] = method;
	}

	template <typename Type>
	void add(const std::string& name, Type* object, rexjson::value (Type::*func)(rexjson::array& params, rpc_exec_mode mode))
	{
		add(name, [=](rexjson::array& params, rpc_exec_mode mode)->rexjson::value{return (object->*func)(params, mode);});
	}

	void add(const std::string& name, rexjson::value (T::*func)(rexjson::array& params, rpc_exec_mode mode))
	{
		add(name, [=](rexjson::array& params, rpc_exec_mode mode)->rexjson::value{return (static_cast <T*>(this)->*func)(params, mode);});
	}

	template <typename Ret, typename ...Args>
	void add(const std::string& name, const rpc_wrapper<Ret, Args...>& wrap)
	{
		add(name, [=](rexjson::array& params, rpc_exec_mode mode)->rexjson::value
		{
			rpc_wrapper<Ret, Args...> w = wrap;
			if (mode != execute)
				return noexec(params, mode, w.json_types_.data(), w.json_types_.size(), w.help_msg_.c_str());
			return w.call(params, mode);
		});
	}

	rexjson::value call_method_name(const rexjson::value& methodname, rexjson::array& params, rpc_exec_mode mode = execute)
	{
		if (methodname.get_type() != rexjson::str_type)
			throw create_rpc_error(RPC_INVALID_REQUEST, "method must be a string");
		typename method_map_type::const_iterator method_entry = map_.find(methodname.get_str());
		if (method_entry == map_.end())
			throw create_rpc_error(RPC_METHOD_NOT_FOUND, "method not found");
		return method_entry->second(params, mode);
	}

	rexjson::value call(const rexjson::value& val, rpc_exec_mode mode = execute)
	{
		rexjson::object ret;
		rexjson::value result;
		rexjson::value error;
		rexjson::value id;
		rexjson::array params;

		try {
			if (val.get_type() != rexjson::obj_type)
				throw create_rpc_error(RPC_PARSE_ERROR, "top-level object parse error");
			rexjson::object::const_iterator params_it = val.get_obj().find("params");
			if (params_it != val.get_obj().end() && params_it->second.type() != rexjson::array_type)
				throw create_rpc_error(RPC_INVALID_REQUEST, "params must be an array");
			if (params_it != val.get_obj().end())
				params = params_it->second.get_array();
			rexjson::object::const_iterator id_it = val.get_obj().find("id");
			if (id_it != val.get_obj().end())
				id = id_it->second;
			rexjson::object::const_iterator method_it = val.get_obj().find("method");
			if (method_it == val.get_obj().end())
				throw create_rpc_error(RPC_INVALID_REQUEST, "missing method");
			result = call_method_name(method_it->second, params, mode);
		} catch (rexjson::object& e) {
			error = e;
		} catch (std::exception& e) {
			rexjson::object errobj;
			errobj["message"] = e.what();
			errobj["code"] = RPC_MISC_ERROR;
			error = errobj;
		}
		ret["result"] = result;
		ret["id"] = id;
		ret["error"] = error;
		return ret;
	}

	rexjson::value call(const std::string& name, const rexjson::value& id, const rexjson::array& params, rpc_exec_mode mode = execute)
	{
		rexjson::object req;
		req["id"] = id;
		req["method"] = rexjson::value(name);
		req["params"] = rexjson::value(params);
		return call(rexjson::value(req), mode);
	}

	rexjson::value call(const std::string& request, rpc_exec_mode mode = execute)
	{
		rexjson::object ret;
		rexjson::value result;
		rexjson::value error;
		rexjson::value id;

		try {
			rexjson::value val;
			val.read(request);
			return call(val, mode);
		} catch (rexjson::object& e) {
			error = e;
		} catch (std::exception& e) {
			rexjson::object errobj;
			errobj["message"] = e.what();
			errobj["code"] = RPC_MISC_ERROR;
			error = errobj;
		}
		ret["result"] = result;
		ret["id"] = id;
		ret["error"] = error;
		return ret;
	}

protected:
	static void convert_types_to_strings(rexjson::value& val)
	{
		if (val.get_type() == rexjson::int_type && val.get_int() >= rexjson::null_type && val.get_int() <= rexjson::real_type) {
			std::string strtypename = rexjson::value::get_typename(val.get_int());
			val = strtypename;
		} else if (val.get_type() == rexjson::array_type) {
			for (size_t i = 0; i < val.get_array().size(); i++) {
				convert_types_to_strings(val.get_array()[i]);
			}
		}
	}

	static void verify_parameters(rexjson::array& params, unsigned int *types, size_t n)
	{
		params.resize(n);
		for (size_t i = 0; i < n; i++) {
			rexjson::value_type value_type = params[i].get_type();
			if ((get_rpc_type(value_type) & types[i]) == 0) {
				throw create_rpc_error(RPC_INVALID_PARAMS, "Invalid parameter: '" + params[i].write(false) + "'");
			}
		}
	}

protected:
	method_map_type map_;
};

} /* namespace rexjson */


#endif /* _RPCSERVER_H_ */
