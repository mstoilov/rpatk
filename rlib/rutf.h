/*
 * Copyright (C) 1999-2001, 2004 Free Software Foundation, Inc.
 * This file is part of the GNU LIBICONV Library.
 *
 * The GNU LIBICONV Library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * The GNU LIBICONV Library is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with the GNU LIBICONV Library; see the file COPYING.LIB.
 * If not, write to the Free Software Foundation, Inc., 51 Franklin Street,
 * Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef _RUTF_H_
#define _RUTF_H_

#include "rtypes.h"

#ifdef __cplusplus
extern "C" {
#endif


rinteger r_utf8_mbtowc(ruint32 *pwc, const ruchar *input, const ruchar *end);
rinteger r_utf8_wctomb(ruint32 wc, ruchar *output, ruint32 size);
rinteger r_utf16_mbtowc(ruint32 *pwc, const ruchar *s, const ruchar *end);
rinteger r_utf16_wctomb(ruint32 wc, ruchar *output, ruint32 size);


#ifdef __cplusplus
}
#endif

#endif
