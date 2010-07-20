/*
 *  Regular Pattern Analyzer (RPA)
 *  Copyright (c) 2009-2010 Martin Stoilov
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
 *  Martin Stoilov <martin@rpasearch.com>
 */

#ifndef _RVMCONFIG_H_
#define _RVMCONFIG_H_

#define RVM_USERDATA
#define RVM_REG_SIZE (1 << 3)

typedef unsigned long int rvm_uint_t;
typedef long int rvm_int_t;
typedef void* rvm_pointer_t;
typedef unsigned char rvm_u8_t;
typedef unsigned short rvm_u16_t;
typedef unsigned int rvm_u32_t;

#endif
