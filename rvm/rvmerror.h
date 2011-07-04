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

#ifndef _RVMERROR_H_
#define _RVMERROR_H_

#define RVM_E_NONE 0

#include "rtypes.h"

typedef ruinteger rvm_error_t;

#define RVM_ERROR_BITS 16
#define RVM_ERROR_MASK ((1 << RVM_ERROR_BITS) - 1)

#define rvm_make_error(__m__, __c__) ((rvm_error_t)(((__m__) << RVM_ERROR_BITS) | ((__c__) & RVM_ERROR_MASK)))
#define rvm_set_error(__p__, __m__, __c__) do { (*(__p__) = rvm_make_error(__m__, __c__)); } while (0)
#define rvm_error_code(__e__) ((__e__) & RVM_ERROR_MASK)
#define rvm_error_module(__e__) ((__e__) >> RVM_ERROR_BITS)

#endif
