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

#ifndef RPCPROPERTY_H_
#define RPCPROPERTY_H_

#include <assert.h>
#include <functional>
#include "rexjson/rexjson++.h"

namespace rexjson {

class property;
using enumarate_children_callback = std::function<void(const std::string&, property&)>;


typedef enum {readonly = 1, writeonly = 2, readwrite = 3} property_access;

class iproperty_object {
public:
	virtual ~iproperty_object() { }
	virtual iproperty_object* duplicate() const = 0;
	virtual void set_prop(const rexjson::value& val) = 0;
	virtual rexjson::value get_prop() = 0;
	virtual void enumarate_children(property &parent, const std::string& path, const enumarate_children_callback& callback) = 0;
	virtual property& navigate(property &parent, const std::string& path) = 0;
	virtual property& operator[](const std::string& name) = 0;
	virtual property& push_back(const property& v) = 0;
	virtual property_access access() const = 0;
};


template<typename T>
class property_object : public iproperty_object {
public:
	property_object() = delete;
	property_object(
			T* propaddr,
			property_access access,
			const std::function<void(const rexjson::value&)>& check_hook,
			const std::function<void(void *ctx)>& modified_hook,
			void* ctx)
	: propaddr_(propaddr)
	, access_(access)
	, check_hook_(check_hook)
	, modified_hook_(modified_hook)
	, ctx_(ctx)
	{
		assert(propaddr_);
	}

	T* propaddr_;
	property_access access_ = readwrite;
	std::function<void(const rexjson::value&)> check_hook_ = {};
	std::function<void(void *ctx)> modified_hook_ = {};
	void* ctx_ = nullptr;

	virtual iproperty_object* duplicate() const
	{
		return new property_object(propaddr_, access_, check_hook_, modified_hook_, ctx_);
	}

	virtual void set_prop(const rexjson::value& val) override
	{
		if ((access_ & property_access::writeonly) == 0)
			throw std::runtime_error("No write access");
		check_hook_(val);
		set_prop_impl<T>(propaddr_, val);
		modified_hook_(ctx_);
	}

	virtual rexjson::value get_prop() override
	{
		if ((access_ & property_access::readonly) == 0)
			throw std::runtime_error("No read access");
		rexjson::value val = get_prop_impl(propaddr_);
		return val;
	}

	virtual void enumarate_children(property &parent, const std::string& path, const enumarate_children_callback& callback) override
	{
		callback(path, parent);
	}

	virtual property& navigate(property &parent, const std::string& path) override
	{
		if (!path.empty())
			throw (std::runtime_error("Invalid path"));
		return parent;
	}

	virtual property& operator[](const std::string& name) override
	{
		throw std::runtime_error("property_object: Invalid call to operator[]");
	}

	virtual property& push_back(const property& v) override
	{
		throw std::runtime_error("property_object: Invalid call to push_back()");
	}

	virtual property_access access() const override
	{
		return access_;
	}

	template <typename U>
	typename std::enable_if<std::is_integral<U>::value || std::is_enum<U>::value, int64_t>::type
	get_prop_impl(U* prop)
	{
		return *prop;
	}

	template <typename U>
	typename std::enable_if<std::is_floating_point<U>::value, double>::type
	get_prop_impl(U* prop)
	{
		return *prop;
	}


	template <typename U>
	typename std::enable_if<std::is_base_of<std::string, U>::value, std::string>::type
	get_prop_impl(U* prop)
	{
		return *prop;
	}

	template <typename U>
	typename std::enable_if<std::is_integral<U>::value, void>::type
	set_prop_impl(U* prop, const rexjson::value& val)
	{
		if (val.get_type() == rexjson::int_type) {
			*prop = val.get_int64();
		} else if (val.get_type() == rexjson::real_type) {
			*prop = (U)val.get_real();
		} else if (val.get_type() == rexjson::bool_type) {
			*prop = val.get_bool() ? 1 : 0;
		} else {
			throw std::runtime_error("Invalid property type.");
		}
	}

	template <typename U>
	typename std::enable_if<std::is_floating_point<U>::value, void>::type
	set_prop_impl(U* prop, const rexjson::value& val)
	{
		*prop = val.get_real();
	}

	template <typename U>
	typename std::enable_if<std::is_base_of<std::string, U>::value, void>::type
	set_prop_impl(U* prop, const rexjson::value& val)
	{
		*prop = val.get_str();
	}

	template <typename U>
	typename std::enable_if<std::is_enum<U>::value, void>::type
	set_prop_impl(U* prop, const rexjson::value& val)
	{
		*reinterpret_cast<typename std::underlying_type<U>::type*>(prop) = val.get_int();
	}
};

template<>
class property_object<bool> : public iproperty_object {
public:
	property_object(
			bool* propaddr,
			property_access access = property_access::readwrite,
			const std::function<void(const rexjson::value&)>& check_hook = {},
			const std::function<void(void *ctx)>& modified_hook = {},
			void *ctx = nullptr)
	: propaddr_(propaddr)
	, access_(access)
	, check_hook_(check_hook)
	, modified_hook_(modified_hook)
	, ctx_(ctx)
	{}

	bool* propaddr_;
	property_access access_ = readwrite;
	std::function<void(const rexjson::value&)> check_hook_ = {};
	std::function<void(void *ctx)> modified_hook_ = {};
	void *ctx_ = nullptr;


	virtual iproperty_object* duplicate() const
	{
		return new property_object(propaddr_, access_, check_hook_, modified_hook_, ctx_);
	}

	virtual void set_prop(const rexjson::value& val) override
	{
		if ((access_ & property_access::writeonly) == 0)
			throw std::runtime_error("No write access");
		check_hook_(val);
		*propaddr_ = val.get_bool();
		modified_hook_(ctx_);
	}

	virtual rexjson::value get_prop() override
	{
		if ((access_ & property_access::readonly) == 0)
			throw std::runtime_error("No read access");
		return *propaddr_;
	}

	virtual void enumarate_children(property &parent, const std::string& path, const enumarate_children_callback& callback) override
	{
		callback(path, parent);
	}

	virtual property& navigate(property &parent, const std::string& path) override
	{
		if (!path.empty())
			throw (std::runtime_error("Invalid path"));
		return parent;
	}

	virtual property& operator[](const std::string& name) override
	{
		throw std::runtime_error("property_object: Invalid call to operator[]");
	}

	virtual property& push_back(const property& v) override
	{
		throw std::runtime_error("property_object: Invalid call to push_back()");
	}

	virtual property_access access() const override
	{
		return access_;
	}

};

template<typename T>
class property_range : public property_object<T> {
public:
	using base = property_object<T>;

	property_range() = delete;
	property_range(
			T* propaddr,
			size_t size,
			property_access access,
			const std::function<void(const rexjson::value&)>& check_hook,
			const std::function<void(void *ctx)>& modified_hook,
			void* ctx)
	: property_object<T>(propaddr, access, check_hook, modified_hook, ctx)
	, size_(size)
	{
	}

	virtual iproperty_object* duplicate() const
	{
		return new property_range(base::propaddr_, size_, base::access_, base::check_hook_, base::modified_hook_, base::ctx_);
	}

	virtual void set_prop(const rexjson::value& val) override
	{
		if ((base::access_ & property_access::writeonly) == 0)
			throw std::runtime_error("No write access");
		base::check_hook_(val);
		size_t i = 0;
		const rexjson::array& a = val.get_array();
		for (auto v : a) {
			if (i >= size_)
				throw std::range_error("The array of values passed is of bigger size than the property range.");
			base::set_prop_impl(base::propaddr_ + i, v);
			++i;
		}
		base::modified_hook_(base::ctx_);
	}

	virtual rexjson::value get_prop() override
	{
		rexjson::array ret;
		for (size_t i = 0; i < size_; i++) {
			ret.push_back(base::get_prop_impl(base::propaddr_ + i));
		}
		return ret;
	}


protected:
	size_t size_ = 0;
};
class property_map;
class property_array;

class property {
public:
	virtual ~property();
	template<typename T>
	property(
			T* propaddr,
			property_access access = property_access::readwrite,
			const std::function<void(const rexjson::value&)>& check_hook = [](const rexjson::value& v)->void{},
			const std::function<void(void *ctx)>& modified_hook = [](void *ctx)->void{},
			void *ctx = nullptr)
	{
		object_ = static_cast<iproperty_object*>(new property_object<T>(propaddr, access, check_hook, modified_hook, ctx));
	}

	template<typename T>
	property(
			T* propaddr,
			size_t size,
			property_access access = property_access::readwrite,
			const std::function<void(const rexjson::value&)>& check_hook = [](const rexjson::value& v)->void{},
			const std::function<void(void *ctx)>& modified_hook = [](void *ctx)->void{},
			void *ctx = nullptr)
	{
		object_ = static_cast<iproperty_object*>(new property_range<T>(propaddr, size, access, check_hook, modified_hook, ctx));
	}

	property();
	property(const property& v);
	property(property&& v);
	property(const property_map& map);
	property(const property_array& array);
	property& push_back(const property& v);
	property& operator[](size_t i);
	property& operator[](const std::string& name);
	property& operator=(const property& v);
	property& navigate(const std::string& path);
	void set_prop(const rexjson::value& val);
	rexjson::value get_prop();
	property_access access() const;
	void enumerate_children(const std::string& path, const enumarate_children_callback& callback);
	void check_object();
	rexjson::value to_json();


protected:
	iproperty_object* object_;
};

class property_map : public iproperty_object {
public:
	property_map() = default;
	property_map(std::initializer_list<typename std::map<std::string, property>::value_type>lst) : map_(lst) {}
	property_map(const property_map& map) : map_(map.map_) {}
	virtual ~property_map() { }
	virtual iproperty_object* duplicate() const override { return new property_map(*this); }
	virtual void set_prop(const rexjson::value& val) override { throw std::runtime_error("property_map: Illegal SetProp() call."); }
	virtual rexjson::value get_prop() override { throw std::runtime_error("property_map: Illegal GetProp() call."); }
	virtual void enumarate_children(property &parent, const std::string& path, const enumarate_children_callback& callback) override
	{
		for (auto &o : map_) {
			o.second.enumerate_children(path + "." + o.first, callback);
		}
	}

	virtual property& navigate(property &parent, const std::string& path) override
	{
		std::string toc = path;
		std::size_t toc_pos = toc.find_first_of(".[", 1);
		if (toc_pos != std::string::npos)
			toc = toc.substr(0, toc_pos);
		else
			toc = toc.substr(0);
		if (toc.empty())
			throw (std::runtime_error("Invalid path"));
		std::string restpath = path.substr(toc.size());
		std::string trimmed = (toc.size() && toc.at(0) == '.') ? toc.substr(1) : toc;
		return (operator [](trimmed)).navigate(restpath);
	}

	virtual property& operator[](const std::string& name)
	{
		std::map<std::string, property>::iterator it = map_.find(name);
		if (it == map_.end())
			throw std::runtime_error("Invalid property: " + name);
		return it->second;
	}

	virtual property& push_back(const property& v) override
	{
		throw std::runtime_error("property_map: Invalid call to push_back()");
	}

	virtual property_access access() const override
	{
		throw std::runtime_error("Invalid call to method access()");
	}


protected:
	std::map<std::string, property> map_;
};

class property_array : public iproperty_object {
public:
	property_array() = default;
	property_array(std::initializer_list<property>lst) : array_(lst) {}
	property_array(const property_array& array) : array_(array.array_) {}
	virtual ~property_array() { }
	virtual iproperty_object* duplicate() const override { return new property_array(*this); }
	virtual void set_prop(const rexjson::value& val) override { throw std::runtime_error("property_array: Illegal SetProp() call."); }
	virtual rexjson::value get_prop() override { throw std::runtime_error("property_array: Illegal GetProp() call."); }
	virtual void enumarate_children(property &parent, const std::string& path, const enumarate_children_callback& callback) override
	{
		size_t i = 0;
		for (auto &o : array_) {
			o.enumerate_children(path + "[" + std::to_string(i++) + "]", callback);
		}
	}

	virtual property& navigate(property &parent, const std::string& path)
	{
		std::string toc = path;
		if (!toc.size() || toc.at(0) != '[')
			throw (std::runtime_error("Invalid path"));
		std::size_t toc_pos = toc.find_first_of("]", 1);
		if (toc_pos == std::string::npos)
			throw (std::runtime_error("Invalid path"));
		toc = toc.substr(1, toc_pos - 1);
		if (toc.empty())
			throw (std::runtime_error("Invalid path"));
		std::string restpath = path.substr(toc.size() + 2);
		return (operator [](toc)).navigate(restpath);
	}

	virtual property& operator[](const std::string& name)
	{
		size_t idx = atol(name.c_str());
		if (idx >= array_.size())
			throw std::range_error("Invalid index");
		return array_.operator [](idx);
	}

	virtual property& push_back(const property& v) override
	{
		array_.push_back(v);
		return *array_.rbegin();
	}

	virtual property_access access() const override
	{
		throw std::runtime_error("Invalid call to method access()");
	}

protected:
	std::vector<property> array_;
};

} // namespace rexjson

#endif /* RPCPROPERTY_H_ */
