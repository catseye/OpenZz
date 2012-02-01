/* 
    Zz Dynamic Parser Library
    Copyright (C) 1989 - I.N.F.N - S.Cabasino, P.S.Paolucci, G.M.Todesco

    The Zz Library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    The Zz Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#ifndef TRACE_DEFINED
#define TRACE_DEFINED

#include <stdio.h>
#include <assert.h>
#include "printz.h"

#if defined(DEBUG)

#define zz_trace(FMT, ARGS...) fprintz(stdout, "Zz trace: " FMT, ## ARGS)
#define zz_assert(COND) assert(COND)

#else

static __inline void __void_trace(const char* ff, ...) {}
static __inline void __void_assert(int cond) {}

#define zz_trace(FMT, ARGS...)	__void_trace(FMT, ## ARGS)
//#define zz_assert(COND)         __void_assert(COND)
#define zz_assert(COND) assert(COND)

#endif

int zz_trace_mask();

#endif
