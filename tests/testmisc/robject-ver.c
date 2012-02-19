/*
 *  Regular Pattern Analyzer Toolkit (RPA/Tk)
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

#include <stdio.h>

#include "rlib/robject.h"

typedef struct _RTestStruct {
	int i;
	char a;
	char b;
} RTestStruct;

int main(int argc, char *argv[])
{
	RTestStruct t;

	fprintf(stdout, 
			"It works. Major: %d, Minor: %d, sizeof(t) = %lu\n", 
			r_object_major_version(), 
			r_object_minor_version(),
			sizeof(t));
	return 0;
}
