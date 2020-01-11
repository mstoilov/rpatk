/*
 * rpcproperty.cpp
 *
 *  Created on: Oct 22, 2019
 *      Author: mstoilov
 */

#include "property.h"

namespace rexjson {

property::~property()
{
	delete object_;
}

property::property()
{
	object_ = nullptr;
}

property::property(const property& v)
{
	object_ = v.object_->duplicate();
}

property::property(property&& v)
{
	object_ = v.object_;
	v.object_ = nullptr;
}

property::property(const property_map& map)
{
	object_ = map.duplicate();
}

property::property(const property_array& array)
{
	object_ = array.duplicate();
}

property& property::push_back(const property& v)
{
	check_object();
	return object_->push_back(v);
}

property& property::operator[](size_t i)
{
	return operator[](std::to_string(i));
}

property& property::operator[](const std::string& name)
{
	check_object();
	return object_->operator[](name);
}

property& property::operator=(const property& v)
{
	delete(object_);
	object_ = v.object_->duplicate();
	return *this;
}

property& property::navigate(const std::string& path)
{
	check_object();
	return object_->navigate(*this, path);
}

void property::set_prop(const rexjson::value& val)
{
	check_object();
	object_->set_prop(val);
}

rexjson::value property::get_prop()
{
	check_object();
	return object_->get_prop();
}

property_access property::access() const
{
	return object_->access();
}

void property::enumerate_children(const std::string& path, const enumarate_children_callback& callback)
{
	check_object();
	object_->enumarate_children(*this, path, callback);
}

void property::check_object()
{
	if (!object_)
		throw std::runtime_error("Invalid property");
}

rexjson::value property::to_json()
{
	rexjson::object ret;
	enumerate_children("", [&](const std::string& path, rexjson::property& prop)->void {
		ret[path] = prop.get_prop();
	});
	return ret;
}

}
