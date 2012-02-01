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

/*
  Zz C bindings

  $Id: zzbind.h,v 1.4 2002/02/13 14:26:28 kibun Exp $
*/

#ifndef __ZZBIND_H__
#define __ZZBIND_H__

#ifdef  __cplusplus
extern "C" {
#endif


int zz_bind_open(const char *);
int zz_bind_keyword(const char *);
int zz_bind_match(const char *);

int zz_bind_close(void);

/* PROC/FUN */
struct s_content; /* forward declaration */
typedef int  (*zz_fun)(int argc, struct s_content argv[], struct s_content *ret);

int zz_bind_call_exe_proc(zz_fun fun, const char *tag);
int zz_bind_call_exe_no_tag(zz_fun fun);

/* SPROC/SFUN: Simple PROCedures/FUNctions */
struct s_tag; /* forward declaration */
typedef void (*zz_sproc)(int /*, ....*/);
typedef int  (*zz_sfun )(int /*, ....*/);
int zz_bind_call(zz_sproc sproc);
int zz_bind_call_fun(zz_sfun sfun, const char *tag);
int zz_bind_call_fun_tag(zz_sfun sfun, struct s_tag *tag);

int zz_bind_pass(void);

struct s_content* zz_bind_get_ret_value(void);

#ifdef  __cplusplus
}
#endif

#endif /* __ZZBIND_H__ */
