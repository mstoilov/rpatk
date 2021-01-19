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
