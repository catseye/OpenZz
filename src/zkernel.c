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

#include <stdint.h>
#include "zlex.h"
#include "kernel.h"

void zkernel(void)
{
/* redefinition of the syntax */

OPEN(stat) M("/") GSB(lvalue) M("->") GSB($thread) GSB($action) 
      PROC(z_link_rule_default) END
OPEN(stat) M("/(") GSB(lvalue) M(")") GSB(ident) M("->") GSB($thread) 
  GSB($action) PROC(z_link_rule) END

/* action */
OPEN($action) END
OPEN($action) GSB(list) PROC(z_set_action) END
OPEN($action) M("{") GSB($ablock) M("}") PROC(z_set_action) END
OPEN($action) M(":") GSB(ident) PROC(z_set_action) END
OPEN($action) M(":return") GSB($arg) PROC(z_set_return_action) END
OPEN($action) M(":return") GSB($arg)M("as") GSB(ident)
   PROC(z_set_return_action)END

/* execute the action when last rule defined is re-defined */
OPEN(stat) M("/when change action") GSB(list_e) 
   PROC(z_set_when_change_action) END
/* execute the action when  */
OPEN(stat) M("/when delete scope") GSB(list_e) 
   PROC(z_set_when_exit_scope) END

/* la thread */
OPEN($thread) END
OPEN($thread) GSB($thread) GSB(ident) PROC(z_bead) END
OPEN($thread) GSB($thread) GSB(int)  PROC(z_bead) END
OPEN($thread) GSB($thread) GSB(float)  PROC(z_bead) END
OPEN($thread) GSB($thread) GSB(double)  PROC(z_bead) END
OPEN($thread) GSB($thread) GSB(qstring)  PROC(z_bead) END
OPEN($thread) GSB($thread) GSB(ident) M("^") GSB(ident) PROC(z_bead) END

/* precedence */
OPEN(stat) M("/prec") GSB(any) /*ACTION(z_show_prec)*/END
OPEN(stat) M("/prec") GSB(any) GSB(num_e) /*ACTION(z_set_left_prec)*/ END
OPEN(stat) M("/prec") GSB(any) M("right") GSB(num_e) 
   /*ACTION(z_set_right_prec)*/END


/* OPEN(stat) M("/remove rules") GSB(ident) PROC(z_remove_rules) END */

OPEN(stat) M("/push scope") GSB(ident) SPROC(zz_push_scope) END
OPEN(stat) M("/delpush scope") GSB(ident) SPROC(delete_and_push_scope) END
OPEN(stat) M("/pop scope") SPROC(zz_pop_scope) END
OPEN(stat) M("/delete scope") GSB(ident) SPROC(delete_scope) END
/* OPEN(stat) M("/scope") GSB(ident) SPROC(set_scope) END */


OPEN($ablock) LIST END
OPEN($ablock) GSB($ablock) M("{") GSB($ablock) M("}") MERGE_ALL END
OPEN($ablock) GSB($ablock) GSB(any) APPEND END
OPEN($ablock) GSB($ablock) GSB(gparam) APPEND END
/* OPEN($bparam) GSB(param) PROC(s_param_filter) END */

  /* 
     set case_sensitive state of lexer:
     != 0 is true
     == 0 is false
  */
OPEN(stat) M("/zlex_set_case_sensitive") GSB(int) SPROC(zlex_set_case_sensitive) END
  /* 
     switch between C-like and TAO-like lexer behaviour:
     != 0 is true  (eol is returned as a token and breaks rule paring)
     == 0 is false (eol is ignored and ';' is necessary to delimit rules)
  */
OPEN(stat) M("/zlex_set_parse_eol") GSB(int) SPROC(zlex_set_parse_eol) END
  /*
     set default type for real consts
     != 0 is true  (every floating const is of double type)
     == 0 is false (float is default)
   */
OPEN(stat) M("/zlex_set_default_real_as_double") GSB(int) SPROC(zlex_set_default_real_as_double) END
  /*
     set default type for integer const
     != 0 is true  (every floating const is of double type)
     == 0 is false (float is default)
   */
OPEN(stat) M("/zlex_set_default_integer_as_int64") GSB(int) SPROC(zlex_set_default_integer_as_int64) END


}
static char sccsid[]="@(#)zkernel.c	6.2\t9/7/94";
static char rcsid[] = "$Id: zkernel.c,v 1.6 2002/06/03 11:06:13 kibun Exp $ ";
