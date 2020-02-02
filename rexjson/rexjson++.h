/*
 * rexjsonpp.h
 *
 * Created on: Mar 10, 2015
 *     Author: Martin Stoilov
 *     License: MIT
 */

#ifndef REXJSON_HPP_
#define REXJSON_HPP_

#include <string>
#include <map>
#include <vector>
#include <sstream>
#include <iostream>
#include <stdint.h>

namespace rexjson {

class value;
class input;
class output;

typedef std::map<std::string, value> object;

class array : public std::vector<value>
{
public:
	using base = std::vector<value>;
	using base::base;
	base::reference operator[](base::size_type n)
	{
		if (n >= size())
			throw std::range_error("Invalid index");
		return base::operator [](n);
	}
	base::const_reference operator[](base::size_type n) const
	{
		if (n >= size())
			throw std::range_error("Invalid index");
		return base::operator [](n);
	}
};


enum value_type {null_type = 0, obj_type, array_type, str_type, bool_type, int_type, real_type};

class value {
	friend class input;
	friend class output;

public:
	static value null;

	value(); // creates null value
	value(const value& v);
	value(const char* v);
	value(const std::string& v);
	value(bool v);
	value(int v);
	value(unsigned int v);
	value(long int v);
	value(unsigned long int v);
	value(long long int v);
	value(unsigned long long int v);
	value(double v);
	value(const object& v);
	value(const array& v);
	~value();

	/**
	 * Serialize the JSON tree to string.
	 *
	 * @param pretty Format the output string.
	 * @param nullprop If set to false properties with null values will be omitted
	 * @param tabsize if pretty is true the formatting will use indentation with the specified tabsize
	 * @param precision Specifies how many digits to use after the decimal dot
	 */
	std::string write(bool pretty = false, bool nullprop = true, size_t tabsize = 4, size_t precision = 4) const;

	/**
	 * Parse the string and return JSON value representation.
	 * If parsing fails this function will throw exception std::runtime_error
	 *
	 * @param str JSON string to be parsed
	 * @param maxlevels The maximum nested levels the parser can parse
	 */
	value& read(const std::string& str, size_t maxlevels = 32);
	value& read(const char* s, size_t n, size_t maxlevels = 32);

	value& push_back(const value& v = value::null);
	value& operator[](size_t i);
	value& operator[](const std::string& name);
	object& get_obj();
	const object& get_obj() const;
	array& get_array();
	const array& get_array() const;
	const std::string& get_str() const;
	bool is_null() const;
	bool get_bool() const;
	int get_int() const;
	int64_t get_int64() const;
	double get_real() const;
	value_type get_type() const;
	value_type type() const { return get_type(); }
	std::string get_typename() const;
	std::string to_string() const;
	static std::string get_typename(unsigned int type);

	template<typename T>
	void get(T& ret) const
	{
		ret = get_int();
	}

	template<typename T>
	T get_value() const
	{
		T ret;
		get<T>(ret);
		return ret;
	}

	template <typename T> void get_v(T& val);

	void set_object(const object& v) {
		operator=(v);
	}
	void set_array(const array& v) {
		operator=(v);
	}
	void set_str(const std::string& v) {
		operator=(v);
	}
	void set_str(const char* v) {
		operator=(v);
	}
	void set_bool(bool v) {
		operator=(v);
	}
	void set_int(int v) {
		operator=(v);
	}
	void set_int(int64_t v) {
		operator=(v);
	}
	void set_real(double v) {
		operator=(v);
	}
	value& operator=(const value& v);
	value& operator=(const object& v);
	value& operator=(const array& v);
	value& operator=(const std::string& v);
	value& operator=(const char* v);
	value& operator=(bool v);
	value& operator=(int v);
	value& operator=(int64_t v);
	value& operator=(double v);

	explicit operator float() { return get_real(); }
	explicit operator double() { return get_real(); }
	explicit operator int() { return get_int(); }
	explicit operator uint() { return get_int(); }
	explicit operator int64_t() { return get_int64(); }
	explicit operator bool() { return get_bool(); }

	void check_type(value_type vtype) const;
	void move(value& v);

protected:
	void destroy();

protected:
	value_type value_type_;
	union {
		bool v_bool_;
		double v_double_;
		int64_t v_int_;
		void* v_null_;
		std::string *v_string_;
		array *v_array_;
		std::map<std::string, value> *v_object_;
	} store_;
};


template<>
inline void value::get<int64_t>(int64_t& ret) const
{
	ret = get_int64();
}

template<>
inline void value::get<bool>(bool& ret) const
{
	ret = get_bool();
}

template<>
inline void value::get<double>(double& ret) const
{
	ret = get_real();
}

template<>
inline void value::get<float>(float& ret) const
{
	ret = get_real();
}

template<>
inline void value::get<std::string>(std::string& ret) const
{
	ret = get_str();
}

class input {
public:
	input(std::istream& is) : is_(is), token_id_(0), offset_(0), levels_(0), errline_(1), errpos_(1) {}
	void read_steam(value& v, size_t maxlevels = 32);

protected:
	void get_token();
	void next_token();
	void error_unexpectedtoken();
	void parse_token(int token);
	void parse_name(std::string &str);
	void parse_namevalue(object& parent);
	void parse_object(value& parent);
	void parse_array(value& parent);
	void parse_primitive(value& v);
	void parse_value(value& v);

protected:
	std::istream& is_;
	std::string token_;
	std::string loctok_;
	int token_id_;
	size_t offset_;
	size_t levels_;
	size_t errline_;
	size_t errpos_;
};

class output {
public:
	output(bool pretty = false, bool nullprop = true, size_t tabsize = 4, size_t precision = 3, const std::string& crlf = "\n");
	void write(const value& v, std::ostream& os);
	std::string write(const value& v);

protected:
	void write_stream(const value& v, std::ostream& os, size_t level);
	void write_stream_namevalue(const std::string& n, const value& v, std::ostream& os, size_t level);
	void write_stream_value(const value& v, std::ostream& os, size_t level);

protected:
	bool pretty_;
	bool nullprop_;
	size_t tabsize_;
	size_t precision_;
	std::string crlf_;
};

/**
 * Parse the string and return JSON value representation.
 * If parsing fails this function will throw std::runtine_error
 */
inline value read(const std::string& s)
{
	return value().read(s);
}

inline value read(char* s, size_t n)
{
	return value().read(s, n);
}

/**
 * Parse the string and return JSON value representation.
 * If parsing fails this function will return false
 */
inline bool read_no_throw(value& v, const std::string& str, size_t maxlevels = 32)
{
	try {
		std::stringstream oss(str);
		rexjson::input in(oss);
		in.read_steam(v, maxlevels);
	} catch (std::runtime_error &e) {
		return false;
	}
	return true;
}


inline std::string write(const value& v)
{
	return v.write(false);
}

inline std::string write_formatted(const value& v)
{
	return v.write(true);
}


} // namespace rexjson

#endif /* REXJSON_HPP_ */
