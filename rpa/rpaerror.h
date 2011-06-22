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

/**
 * \file rpaerror.h
 * \brief The public interface for examining errors.
 *
 *
 * <h2>Synopsis</h2>
 * The RPA error(s) interface.
 */

#ifndef _RPAERROR_H_
#define _RPAERROR_H_

#include "rtypes.h"

#define RPA_ERRINFO_NONE 0
#define RPA_ERRINFO_CODE (1<<0)
#define RPA_ERRINFO_OFFSET (1<<1)
#define RPA_ERRINFO_LINE (1<<2)
#define RPA_ERRINFO_RULEID (1<<3)
#define RPA_ERRINFO_NAME (1<<4)


#define RPA_E_NONE 0
#define RPA_E_OUTOFMEM 1001
#define RPA_E_NOTCOMPILED 1002
#define RPA_E_NOTOPEN 1003
#define RPA_E_NOTCLOSED 1004
#define RPA_E_NOTFOUND 1005
#define RPA_E_SYNTAXERROR 1006
#define RPA_E_UNRESOLVEDSYMBOL 1007
#define RPA_E_PARAM 1008
#define RPA_E_COMPILE 1009

#define RPA_E_EXECUTION 2001
#define RPA_E_USERABORT 2002
#define RPA_E_RULEABORT 2003
#define RPA_E_INVALIDINPUT 2004

/**
 * \typedef rpaerror_t
 * \brief RPA error description.
 */
typedef struct rpa_errinfo_s rpa_errinfo_t;
struct rpa_errinfo_s {
	rulong mask;
	rlong code;
	rlong offset;
	rlong line;
	rlong ruleid;
	rchar name[128];
};


#endif
