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

#ifndef PRINTZ_DEFINED
#define PRINTZ_DEFINED

#ifdef __STDC__
#define PRINTZ_USE_STDARG
#define PRINTZ_USE_PROTOTYPES
#endif

#ifdef VAX
#define PRINTZ_USE_STDARG
#define PRINTZ_USE_PROTOTYPES
#endif

#ifndef PRINTZ
#ifdef PRINTZ_USE_PROTOTYPES
int fprintz (FILE *file_ptr, const char *format_spec, ...);
int printz  (const char *format_spec, ...);
int sprintz (char *str, const char *format_spec, ...);
int printz_aux(FILE *chan);
int printz_code(int code,int (*fprint_proc)(FILE *chan, void*),int (*sprint_proc)(char*, void*));
#else
int fprintz ();
int printz  ();
int sprintz ();
int printz_aux();
int printz_code();
#endif
#endif
#endif
