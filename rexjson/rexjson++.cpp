/*
 * Created on: Mar 10, 2015
 *     Author: Martin Stoilov
 *     License: MIT
 *
 * To generate the header file rexjsondfa.h from rexjsondfa.rexcc do:
 * # rexcc rexjsondfa.rexcc -o rexjsondfa.h
 * 
 * To build the test program:
 * # g++ -o testrexjson rexjson++.cpp -DREXJSON_TEST_MAIN
 *
 * To use the library in your own project, add the following files to your project:
 * rexjson++.cpp
 * rexjson++.h
 * rexjsondfa.h  (generated from rexjsondfa.rexcc)
 * rexdfatypes.h
 */

#include <sstream>
#include <stdexcept>
#include <cstdlib>
#include <limits>
#include "rexjson++.h"
#include "rexjsondfa.h"

namespace rexjson {

static const char* value_type_name[]= {"null", "object", "array", "string", "boolean", "integer", "real"};

value value::null;

value::value()
	: value_type_(null_type)
{
	store_.v_null_ = NULL;
}

value::~value()
{
	destroy();
}

std::string value::get_typename(unsigned int type)
{
	std::string ret;
	if (type <= real_type)
		ret = value_type_name[type];
	return ret;
}

static void escape_str_val(const value& v, std::ostream& os)
{
	for (std::string::const_iterator it = v.get_str().begin(); it != v.get_str().end(); it++) {
		if (*it == '\\' && (it + 1) != v.get_str().end()) {
			os << *it++;
			os << *it;
		} else if (*it == '"') {
			os << '\\' << *it;
		} else if (*it == '\b') {
			os << "\\b";
		} else if (*it == '\f') {
			os << "\\f";
		} else if (*it == '\n') {
			os << "\\n";
		} else if (*it == '\r') {
			os << "\\r";
		} else if (*it == '\t') {
			os << "\\t";
		} else {
			os << *it;
		}
	}
}

static std::string unescape_str(const std::string& s)
{
	std::ostringstream os;

	for (std::string::const_iterator it = s.begin(); it < s.end(); it++) {
		if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == '"') {
			os << '"';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == '\\') {
			os << '\\';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == '/') {
			os << '/';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == 'b') {
			os << '\b';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == 'f') {
			os << '\f';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == 'n') {
			os << '\n';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == 'r') {
			os << '\r';
			it++;
		} else if (*it == '\\' && (it + 1) < s.end() && *(it + 1) == 't') {
			os << '\t';
			it++;
		} else if (*it == '\\' && (it + 5) < s.end() && *(it + 1) == 'u') {
			std::string ustr(it + 2, it + 6);
			unsigned int wc = static_cast<unsigned int>(std::strtol(ustr.c_str(), NULL, 16));
			unsigned int count = 0;
			if (wc <= 0x007F)
				count = 1;
			else if (wc <= 0x07FF)
				count = 2;
			else if (wc <= 0xFFFF)
				count = 3;
			switch (count) {
				case 3:
					os << static_cast<unsigned char>(0x80 | (wc & 0x3f));
					wc = wc >> 6;
					wc |= 0x800;
				case 2:
					os << static_cast<unsigned char>(0x80 | (wc & 0x3f));
					wc = wc >> 6;
					wc |= 0xc0;
				case 1:
					os << static_cast<unsigned char>(wc);
			}
			it += 5;
		} else if (*it == '\\' && (it + 1) < s.end()) {
			os << *it++;
			os << *it;
		} else {
			os << *it;
		}
	}
	return os.str();
}

void value::destroy()
{
	if (get_type() == str_type)
		delete store_.v_string_;
	else if (get_type() == array_type)
		delete store_.v_array_;
	else if (get_type() == obj_type)
		delete store_.v_object_;
	store_.v_null_ = NULL;
	value_type_ = null_type;
}

void value::move(value& v)
{
	value_type_ = v.value_type_;
	store_ = v.store_;
	v.value_type_ = null_type;
	v.store_.v_null_ = NULL;
}

value::value(const value& v)
	: value_type_(null_type)
{
	store_.v_null_ = NULL;
	operator=(v);
}

value::value(const char* v)
{
	value_type_ = str_type;
	store_.v_string_ = new std::string(unescape_str(v));
}

value::value(const std::string& v)
{
	value_type_ = str_type;
	store_.v_string_ = new std::string(unescape_str(v));
}

value::value(bool v)
{
	value_type_ = bool_type;
	store_.v_bool_ = v;
}

value::value(int v)
{
	value_type_ = int_type;
	store_.v_int_ = v;
}

value::value(long unsigned int v)
{
	value_type_ = int_type;
	store_.v_int_ = v;
}

value::value(int64_t v)
{
	value_type_ = int_type;
	store_.v_int_ = v;
}

value::value(double v)
{
	value_type_ = real_type;
	store_.v_double_ = v;
}

value::value(const object& v)
{
	value_type_ = obj_type;
	store_.v_object_ = new object(v);
}

value::value(const array& v)
{
	value_type_ = array_type;
	store_.v_array_ = new array(v);
}

value_type value::get_type() const
{
	return value_type_;
}

std::string value::get_typename() const
{
	return std::string(value_type_name[value_type_]);
}

bool value::is_null() const
{
	return value_type_ == null_type ? true : false;
}

const std::string& value::get_str()    const
{
	check_type(str_type);
	return *store_.v_string_;
}

bool value::get_bool() const
{
	check_type(bool_type);
	return store_.v_bool_;
}

int value::get_int() const
{
	check_type(int_type);
	return static_cast<int>(store_.v_int_);
}

int64_t value::get_int64() const
{
	check_type(int_type);
	return static_cast<int64_t>(store_.v_int_);
}

double value::get_real() const
{
	if (get_type() == int_type)
		return static_cast<double>(get_int64());
	check_type(real_type);
	return store_.v_double_;
}

const object& value::get_obj() const
{
	check_type(obj_type);
	return *store_.v_object_;
}

const array& value::get_array() const
{
	check_type(array_type);
	return *store_.v_array_;
}

object& value::get_obj()
{
	check_type(obj_type);
	return *store_.v_object_;
}

array& value::get_array()
{
	check_type(array_type);
	return *store_.v_array_;
}

std::string value::to_string() const
{
	std::ostringstream os;
	if (value_type_ == null_type)
		os << "null";
	else if (value_type_ == bool_type)
		get_bool() ? os << "true" : os << "false";
	else if (value_type_ == int_type)
		os << get_int64();
	else if (value_type_ == real_type)
		os << get_real();
	else if (value_type_ == str_type)
		os << get_str();
	else if (value_type_ == obj_type) {
		for (rexjson::object::const_iterator it = get_obj().begin(); it != get_obj().end(); it++) {
			if (it != get_obj().begin())
				os << ",";
			os << it->first << ":" << it->second.to_string();
		}
	} else if (value_type_ == array_type) {
		for (rexjson::array::const_iterator it = get_array().begin(); it != get_array().end(); it++) {
			if (it != get_array().begin())
				os << ",";
			os << it->to_string();
		}
	}
	return os.str();
}

void value::check_type(value_type vtype) const
{
	if( get_type() != vtype ) {
		std::ostringstream os;
		os << "value is type '" << get_typename() << "', expected '" << value_type_name[vtype] << "'";
		throw std::runtime_error( os.str() );
	}
}

value& value::operator=(const char* v)
{
	destroy();
	check_type(null_type);
	value_type_ = str_type;
	store_.v_string_ = new std::string(v);
	return *this;
}

value& value::operator=(bool v)
{
	destroy();
	value_type_ = bool_type;
	store_.v_bool_ = v;
	return *this;
}

value& value::operator=(int v)
{
	destroy();
	value_type_ = int_type;
	store_.v_int_ = v;
	return *this;
}

value& value::operator=(int64_t v)
{
	destroy();
	value_type_ = int_type;
	store_.v_int_ = v;
	return *this;
}

value& value::operator=(double v)
{
	destroy();
	value_type_ = real_type;
	store_.v_double_ = v;
	return *this;
}

value& value::operator=(const std::string& v)
{
	std::string *tmp = new std::string(unescape_str(v));

	destroy();
	value_type_ = str_type;
	store_.v_string_ = tmp;
	return *this;
}

value& value::operator=(const object& v)
{
	object *tmp = new object(v);

	destroy();
	value_type_ = obj_type;
	store_.v_object_ = tmp;
	return *this;
}

value& value::operator=(const array& v)
{
	array *tmp = new array(v);

	destroy();
	value_type_ = array_type;
	store_.v_array_ = tmp;
	return *this;
}

value& value::operator=(const value& v)
{
	if (&v == this)
		return *this;
	destroy();
	value_type_ = v.value_type_;
	if (get_type() == str_type)
		store_.v_string_ = new std::string(*v.store_.v_string_);
	else if (get_type() == array_type)
		store_.v_array_ = new array(*v.store_.v_array_);
	else if (get_type() == obj_type)
		store_.v_object_ =  new object(*v.store_.v_object_);
	else
		store_ = v.store_;
	return *this;
}

value& value::push_back(const value& v)
{
	check_type(array_type);
	store_.v_array_->push_back(v);
	return *store_.v_array_->rbegin();
}

value& value::operator[](size_t i)
{
	check_type(array_type);
	return store_.v_array_->operator[](i);
}

value& value::operator[](const std::string& name)
{
	check_type(obj_type);
	return store_.v_object_->operator[](name);
}

value& value::read(const std::string& str, size_t maxlevels)
{
	std::stringstream oss(str);
	rexjson::input in(oss);
	in.read_steam(*this, maxlevels);
	return *this;
}

value& value::read(const char* s, size_t n, size_t maxlevels)
{
	std::string str(s, n);
	return read(str, maxlevels);
}

std::string value::write(bool pretty, bool nullprop, size_t tabsize, size_t precision) const
{
	return rexjson::output(pretty, nullprop, tabsize, precision).write(*this);
}

output::output(bool pretty, bool nullprop, size_t tabsize, size_t precision, const std::string& crlf)
	: pretty_(pretty)
	, nullprop_(nullprop)
	, tabsize_(tabsize)
	, precision_(precision)
	, crlf_(crlf)
{
}

void output::write(const value& v, std::ostream& os)
{
	write_stream(v, os, 0);
}

std::string output::write(const value& v)
{
	std::stringstream os;
	write_stream(v, os, 0);
	return os.str();
}

void output::write_stream_namevalue(const std::string& n, const value& v, std::ostream& os, size_t level)
{
	std::string indent(level * tabsize_, ' ');
	if (pretty_)
		os << indent;
	os << '"' << n << '"';
	if (pretty_)
		os << ' ';
	os << ":";
	if (pretty_)
		os << ' ';
	write_stream(v, os, level);
}

void output::write_stream_value(const value& v, std::ostream& os, size_t level)
{
	std::string indent(level * tabsize_, ' ');
	if (pretty_)
		os << indent;
	write_stream(v, os, level);
}

void output::write_stream(const value& v, std::ostream& os, size_t level)
{
	std::string indent(level * tabsize_, ' ');

	if (v.value_type_ == null_type) {
		os << "null";
	} else if (v.value_type_ == bool_type) {
		v.get_bool() ? os << "true" : os << "false";
	} else if (v.value_type_ == int_type) {
		os << v.get_int64();
	} else if (v.value_type_ == real_type) {
		if (v.get_real() != std::numeric_limits<double>::infinity() && v.get_real() != std::numeric_limits<double>::quiet_NaN()) {
			os.precision(precision_);
			os.setf(std::ios::fixed, std:: ios::floatfield);
			os << v.get_real();
		} else {
			os << "null";
		}
	} else if (v.value_type_ == str_type) {
		os << '"';
		escape_str_val(v, os);
		os << '"';
	} else if (v.value_type_ == obj_type) {
		size_t elems = 0;
		os << '{';
		if (pretty_ && !v.get_obj().empty())
			os << crlf_;
		for (object::const_iterator it = v.get_obj().begin(); it != v.get_obj().end();it++) {
			if (it->second.is_null() && nullprop_ == false) {
				continue;
			}
			if (elems) {
				os << ',';
				if (pretty_)
					os << crlf_;
			}
			write_stream_namevalue(it->first, it->second, os, level + 1);
			++elems;
		}
		if (pretty_)
			os << crlf_ << indent;
		os << '}';
	} else if (v.value_type_ == array_type) {
		os << '[';
		if (pretty_ && !v.get_array().empty())
			os << crlf_;
		for (array::const_iterator it = v.get_array().begin(); it != v.get_array().end(); it++) {
			if (it != v.get_array().begin()) {
				os << ',';
				if (pretty_)
					os << crlf_;
			}
			write_stream_value(*it, os, level + 1);
		}
		if (pretty_)
			os << crlf_ << indent;
		os << ']';
	}
}

void input::get_token()
{
	rexdfss_t *acc_ss = NULL;
	rexuint_t nstate = REX_DFA_STARTSTATE;
	char ch = 0;

	loctok_.clear();
	token_.clear();
	token_id_ = 0;
	while (is_) {
		++offset_;
		ch = is_.get();
		loctok_ += ch;
		REX_DFA_NEXT(dfa, nstate, ch, &nstate);
		if (nstate == REX_DFA_DEADSTATE) {
			is_.unget();
			--offset_;
			break;
		}
		if (ch == '\n') {
			errline_++;
			errpos_ = 1;
		} else {
			errpos_++;
		}
		if (REX_DFA_STATE(dfa, nstate)->type == REX_STATETYPE_ACCEPT) {
			/*
			 * Note: We will not break out of the loop here. We will keep going
			 * in order to find the longest match.
			 */
			acc_ss = REX_DFA_ACCSUBSTATE(dfa, nstate, 0);
			token_id_ = (int) acc_ss->userdata;
			token_ = loctok_;
		}
	}
}

void input::read_steam(value& v, size_t maxlevels)
{
	levels_ = maxlevels;
	offset_ = 0;
	errline_ = 1;
	errpos_ = 1;
	v.destroy();
	next_token();
	parse_value(v);
}

void input::next_token()
{
	get_token();
	while (token_id_ == TOKEN_SPACE)
		get_token();
}

void input::error_unexpectedtoken()
{
	std::ostringstream os;
	os << "Unexpected (" << errline_ << ":" << errpos_ + 1 - loctok_.length() << "): " << "'" << loctok_ << "' at offset " << offset_ + 1 - loctok_.length();
	throw std::runtime_error(os.str());
}

void input::parse_token(int token)
{
	if (token_id_ != token)
		error_unexpectedtoken();
	next_token();
}

void input::parse_name(std::string &str)
{
	if (token_id_ != TOKEN_STRING)
		error_unexpectedtoken();
	str = (token_.length() > 2) ? token_.substr(1, token_.length() - 2) : std::string();
	next_token();
}

void input::parse_namevalue(object& parent)
{
	std::string name;
	value v;

	parse_name(name);
	parse_token(TOKEN_COLON);
	parse_value(v);
	parent[name] = value();
	parent[name].move(v);
}

void input::parse_object(value& parent)
{
	parse_token(TOKEN_LEFTCB);
	if (token_id_ != TOKEN_RIGHTCB) {
		parse_namevalue(*parent.store_.v_object_);
		while (token_id_ == TOKEN_COMMA) {
			parse_token(TOKEN_COMMA);
			parse_namevalue(*parent.store_.v_object_);
		}
	}
	parse_token(TOKEN_RIGHTCB);
}

void input::parse_array(value& parent)
{
	parse_token(TOKEN_LEFTSB);
	if (token_id_ != TOKEN_RIGHTSB) {
		parent.store_.v_array_->push_back(value());
		parse_value(parent.get_array()[parent.get_array().size() - 1]);
		while (token_id_ == TOKEN_COMMA) {
			parse_token(TOKEN_COMMA);
			parent.store_.v_array_->push_back(value::null);
			parse_value(parent.get_array()[parent.get_array().size() - 1]);
		}
	}
	parse_token(TOKEN_RIGHTSB);
}

void input::parse_primitive(value& v)
{
	switch (token_id_) {
	case TOKEN_STRING:
		v = (token_.length() > 2) ? token_.substr(1, token_.length() - 2) : std::string();
		break;
	case TOKEN_INT:
		v = (int64_t)strtoll(token_.c_str(), NULL, 10);
		break;
	case TOKEN_NUMBER:
		v = strtod(token_.c_str(), NULL);
		break;
	case TOKEN_NULL:
		v = value::null;
		break;
	case TOKEN_TRUE:
		v = true;
		break;
	case TOKEN_FALSE:
		v = false;
		break;
	default:
		error_unexpectedtoken();
	}
	next_token();
}

void input::parse_value(value& v)
{
	if (!levels_)
		throw std::runtime_error("exceeded maximum levels of recursion");
	--levels_;
	switch (token_id_) {
	case TOKEN_LEFTCB:
		v = object();
		parse_object(v);
		break;
	case TOKEN_LEFTSB:
		v = array();
		parse_array(v);
		break;
	default:
		parse_primitive(v);
		break;
	}
	++levels_;
}

}

#if defined(REXJSON_TEST_MAIN)
int main(int argc, const char *argv[])
{
	char text1[]="{\n\"name\" : \"Jack (\\\"Bee\\\") Nimble \u0434\u0432\u0435\", \n\"format\": {\"type\":       \"rect\", \n\"width\":      1920, \n\"height\":     1080, \n\"interlace\":  false,\"frame rate\": 24\n}\n}}";
	char text2[]="[\"Sunday\", \"Monday\", \"Tuesday\", \"Wednesday\", \"Thursday\", \"Friday\", \"Saturday\"]";
	char text3[]="[\n    [0.1, -1, 0],\n    [1, 0, 0],\n    [0, 0, 1]\n	]\n";
	char text4[]="{\n		\"Image\": {\n			\"Width\":  800,\n			\"Height\": 600,\n			\"Title\":  \"View from 15th Floor\",\n		\"Thumbnail\": {\n				\"Url\":    \"http:/*www.example.com/image/481989943\",\n				\"Height\": 125,\n				\"Width\":  \"100\"\n			},\n			\"IDs\": [116, 943, 234, 38793]\n		}\n	}";
	char text5[]="[\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.7668,\n	 \"Longitude\": -122.3959,\n	 \"Address\":   \"\",\n	 \"City\":      \"SAN FRANCISCO\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94107\",\n	 \"Country\":   \"US\"\n	 },\n	 {\n	 \"precision\": \"zip\",\n	 \"Latitude\":  37.371991,\n	 \"Longitude\": -122.026020,\n	 \"Address\":   \"\",\n	 \"City\":      \"SUNNYVALE\",\n	 \"State\":     \"CA\",\n	 \"Zip\":       \"94085\",\n	 \"Country\":   \"US\", \"tst\" : null\n	 }\n	 ]";

	std::cout << "*** text1 ***\n" << rexjson::read(text1).write(false) << std::endl << std::endl;
	std::cout << "*** text2 ***\n" << rexjson::read(text2).write(false) << std::endl << std::endl;
	std::cout << "*** text3 ***\n" << rexjson::read(text3).write(false) << std::endl << std::endl;
	std::cout << "*** text4 ***\n" << rexjson::read(text4, sizeof(text4)).write(true, true, 8) << std::endl << std::endl;
	std::cout << "*** text5 ***\n";
	rexjson::output(false).write(rexjson::read(text5), std::cout);
	std::cout << std::endl << std::endl;

	rexjson::value v = rexjson::object();
	v["prop1"] = "Value \\\"one\\\"";
	v["prop2"] = "Value \u0434\u0432\u0435 two \u0434\u0432\u0435";
	v["prop3"] = rexjson::array();
	v["prop3"].push_back().read("\"Value 3\"");
	v["prop3"].push_back() = "Value 4";
	v["prop3"].push_back(true);
	v["prop3"].push_back(false);
	v["prop3"].push_back(44);
	v["prop3"].push_back(rexjson::read("{}"));
	v["prop3"][5]["prop1"] = 2.3332;
	v["prop3"][5]["prop2"] = rexjson::value::null;
	v["prop3"][5]["prop3"] = rexjson::object();
	v["prop3"][5]["prop3"]["prop1"] = rexjson::object();
	v["prop3"][5]["prop3"]["prop2"] = rexjson::value::null;
	v["prop3"].push_back(rexjson::value().read("[1,2,3]"));
	v["prop3"].push_back(23.33);
	v["prop3"].push_back(rexjson::array());
	v["prop4"] = true;
	v["prop5"] = 3.345;
	v["prop7"].read("[1, 2, true, false, 0.25, {\"prop1\": true}]");
	std::cout << "*** New Text ***\n" << v.write(true, true, 8, 8) << std::endl;
	return 0;
}

#elif defined(REXJSON_TEST_PARSER)

#include <fstream>

int main(int argc, const char *argv[])
{
	try {
		std::stringstream iss;

		if (argc > 1) {
			std::ifstream file(argv[1]);
			iss << file.rdbuf();
		} else {
			iss << std::cin.rdbuf();
		}
		std::cout << rexjson::read(iss.str()).write(true) << std::endl;
	} catch (std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}

#endif
