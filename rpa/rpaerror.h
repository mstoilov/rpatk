/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
 *  Copyright (c) 2009-2012 Martin Stoilov
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
 * @file rpa/rpaerror.h
 * @brief The public interface for examining errors.
 *
 *
 * <h2>Synopsis</h2>
 * The RPA error(s) interface.
 */

#ifndef _RPAERROR_H_
#define _RPAERROR_H_

#include "rtypes.h"

#define RPA_ERRINFO_NONE 0				/**< No error fields are set */
#define RPA_ERRINFO_CODE (1<<0)			/**< The code field is set */
#define RPA_ERRINFO_OFFSET (1<<1)		/**< The offset field is set */
#define RPA_ERRINFO_LINE (1<<2)			/**< The line field is set */
#define RPA_ERRINFO_RULEUID (1<<3)		/**< The ruleid is set */
#define RPA_ERRINFO_NAME (1<<4)			/**< The name field is set */


#define RPA_E_NONE 0					/**< No error */
#define RPA_E_OUTOFMEM 1001				/**< Not enough memory */
#define RPA_E_NOTCOMPILED 1002			/**< The database is not yet compiled to byte code. Some of the operations are not possible until the call to @ref rpa_dbex_compile is made. */
#define RPA_E_NOTOPEN 1003				/**< The database is not open. Use @ref rpa_dbex_open to open it. */
#define RPA_E_NOTCLOSED 1004			/**< The database is not closed. Use @ref rpa_dbex_close to close it. */
#define RPA_E_NOTFOUND 1005				/**< The specified rule cannot be found */
#define RPA_E_SYNTAXERROR 1006			/**< Syntax Error. Check the BNF syntax. */
#define RPA_E_UNRESOLVEDSYMBOL 1007		/**< A rule name is used in the BNF specification, but it is not defined */
#define RPA_E_PARAM 1008				/**< Invalid parameter */
#define RPA_E_COMPILE 1009				/**< Compile error */

#define RPA_E_EXECUTION 2001			/**< Execution error */
#define RPA_E_USERABORT 2002			/**< Operation is interrupted by user */
#define RPA_E_RULEABORT 2003			/**< If a rule is set to abort with the directive #!abort, if it cannot be matched the engine will generate this error */
#define RPA_E_INVALIDINPUT 2004			/**< Invalid input was specified */

/**
 * @brief RPA error description.
 */
typedef struct rpa_errinfo_s rpa_errinfo_t;

/**
 * \struct rpa_errinfo_s <rpa/rpaerror.h> <rpa/rpaerror.h>
 * @brief Error description.
 */
struct rpa_errinfo_s {
	/**
	 * Mask of valid fields
	 * - @ref RPA_ERRINFO_CODE
	 * - @ref RPA_ERRINFO_OFFSET
	 * - @ref RPA_ERRINFO_LINE
	 * - @ref RPA_ERRINFO_RULEUID
	 * - @ref RPA_ERRINFO_NAME
	 */
	unsigned long mask;

	/**
	 * Error code:
	 * - @ref RPA_E_NONE
	 * - @ref RPA_E_OUTOFMEM
	 * - ...
	 */
	long code;

	/**
	 * Error offset in the BNF schema
	 */
	long offset;

	/**
	 * Error line in the BNF schema
	 */
	long line;

	/**
	 * Error rule user ID. If the engine can determine the ruleuid of the rule generating the error,
	 * this field will be set accordingly
	 */
	long ruleuid;

	/**
	 * Error rule name
	 */
	char name[128];
};


#endif
