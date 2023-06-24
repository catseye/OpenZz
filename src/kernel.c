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

#include <stdlib.h>
#include <stdint.h>
#include "zlex.h"
#include "kernel.h"
#include "sys.h"


void kernel(void)
{
struct s_tag *tag;
extern int max_error_n;

OPEN(root) GSB(statlist) M_EOF END
OPEN(statlist) END
OPEN(statlist) GSB(statlist) GSB(stat) M(";") END
OPEN(statlist) GSB(statlist) GSB(stat) M("\n") END

OPEN(stat) END

set_recovery("statlist","\n;");
set_nt_prompt("statlist",zz_get_prompt());
set_nt_prompt("stat",zz_get_prompt());
set_nt_prompt(0,zz_get_prompt());
set_nt_prompt("bu",">>>>");

OPEN($arglist) GSB($arg) LIST END
OPEN($arglist) GSB($arglist) M(",") GSB($arg) APPEND END

OPEN($arg) GSB(num_e) PASS END
OPEN($arg) GSB(string_e) PASS END
OPEN($arg) GSB(list_e) PASS END

OPEN($num_f) GSB(int)    PASS END
OPEN($num_f) GSB(int64)  PASS END
OPEN($num_f) GSB(float)  PASS END
OPEN($num_f) GSB(double) PASS END


OPEN($cond) GSB(string_e) M("==") GSB(string_e) PROC(s_eq) END
OPEN($cond) GSB(string_e) M("!=") GSB(string_e) PROC(s_ne) END
OPEN($cond) GSB(num_e) M("==") GSB(num_e) PROC(s_eq) END
OPEN($cond) GSB(num_e) M("!=") GSB(num_e) PROC(s_ne) END
OPEN($cond) GSB(num_e) M(">=") GSB(num_e) PROC(s_ge) END
OPEN($cond) GSB(num_e) M(">") GSB(num_e) PROC(s_gt) END
OPEN($cond) GSB(num_e) M("<=") GSB(num_e) PROC(s_le) END
OPEN($cond) GSB(num_e) M("<") GSB(num_e) PROC(s_lt) END

/*
* Nested boolean logic(&&,||):
*/
OPEN($cond_e) GSB($cond_e) M("&&") GSB($cond) PROC(s_boolean_and) END
OPEN($cond_e) GSB($cond_e) M("||") GSB($cond) PROC(s_boolean_or) END
OPEN($cond) M("(") GSB($cond_e) M(")") PASS END
OPEN($cond_e) GSB($cond) PASS END

OPEN(num_e) GSB(num_e) M("+") GSB($num_t) PROC(s_add) END
OPEN(num_e) GSB(num_e) M("-") GSB($num_t) PROC(s_sub) END
OPEN(num_e) GSB($num_t) PASS END

OPEN($num_t) GSB($num_t) M("*") GSB($num_f) PROC(s_mult) END
OPEN($num_t) GSB($num_t) M("/") GSB($num_f) PROC(s_div) END
OPEN($num_t) GSB($num_f) PASS END

OPEN($num_f) M("-") GSB($num_f) PROC(s_chs) END
OPEN($num_f) M("(") GSB(num_e) M(")") PASS END

/*
 * Do & while loops
 *  Pretty ugly stuff here - the full syntax for conditional logic is
 *  repeated here for the 'do' and 'while' loops.  This needs to be
 *  done because these loops reinterpret their loop control argument
 *  upon each pass through the loop (whereas 'for' loops can calculate
 *  their loop args just once at the loop entry.
 *
 *  Inside the loop control functions, the loop control argument is 
 *  re-evaluated using the internal exec command - using the internal
 *  'if' statement logic.
 *
 *  The 's_condecho_xxx' functions were needed to correctly reconstruct
 *  the loop control argument.
 */
  OPEN($loop_num_prim) GSB(int)    PASS END
  OPEN($loop_num_prim) GSB(int64)  PASS END
  OPEN($loop_num_prim) GSB(float)  PASS END
  OPEN($loop_num_prim) GSB(double) PASS END

  OPEN($loop_fact) GSB(param)  LIST END
  OPEN($loop_fact) GSB($loop_num_prim) LIST END

  OPEN($loop_cond_e)  GSB($loop_cond_e)  M("&&")  GSB($loop_cond)  PROC(s_condecho_and) END
  OPEN($loop_cond_e)  GSB($loop_cond_e)  M("||")  GSB($loop_cond)  PROC(s_condecho_or) END
  OPEN($loop_cond) M("(") GSB($loop_cond_e) M(")") PROC(s_condecho_passparens) END
  OPEN($loop_cond_e) GSB($loop_cond) PASS END

  // Double char binary ops (==,<=,...):
  OPEN($loop_cond)   GSB($loop_exp) M("==") GSB($loop_exp) PROC(s_condecho_eq) END
  OPEN($loop_cond)   GSB($loop_exp) M("!=") GSB($loop_exp) PROC(s_condecho_ne) END
  OPEN($loop_cond)   GSB($loop_exp) M(">=") GSB($loop_exp) PROC(s_condecho_ne) END
  OPEN($loop_cond)   GSB($loop_exp) M(">")  GSB($loop_exp) PROC(s_condecho_ne) END
  OPEN($loop_cond)   GSB($loop_exp) M("<=") GSB($loop_exp) PROC(s_condecho_le) END
  OPEN($loop_cond)   GSB($loop_exp) M("<")  GSB($loop_exp) PROC(s_condecho_lt) END

  // Single char binary ops (+,-):
  OPEN($loop_exp) GSB($loop_exp) M("+") GSB($loop_term) PROC(s_condecho_add) END
  OPEN($loop_exp) GSB($loop_exp) M("-") GSB($loop_term) PROC(s_condecho_sub) END
  OPEN($loop_exp) GSB($loop_term) PASS END

  // Single char binary ops (*,/):
  OPEN($loop_term)  GSB($loop_term) M("*") GSB($loop_fact) PROC(s_condecho_mult) END
  OPEN($loop_term)  GSB($loop_term) M("/") GSB($loop_fact) PROC(s_condecho_div)  END
  OPEN($loop_term)  GSB($loop_fact) PASS END

  OPEN($loop_fact) M("-") GSB($loop_fact) PROC(s_condecho_chs) END
  OPEN($loop_fact) M("(") GSB($loop_exp) M(")") PROC(s_condecho_passparens) END

OPEN(stat) M("/while") GSB($loop_cond_e)
  M("{") GSB($flow_block) M("}") PROC(s_while) END

OPEN(stat) M("/do") M("{") GSB($flow_block) M("}")
  M("/while") GSB($loop_cond_e) PROC(s_do) END


OPEN(string_e) GSB(string_e) M("&") GSB(string_t) PROC(s_strcat) END
OPEN(string_e) GSB(num_e) M("&") GSB(string_t) PROC(s_strcat) END
OPEN(string_e) GSB(string_e) M("&") GSB(num_e) PROC(s_strcat) END
OPEN(string_e) GSB(string_t) PASS END

OPEN(string_t) GSB(ident) PASS END
OPEN(string_t) GSB(qstring) PASS END
OPEN(string_t) M("$(") GSB(string_e) M(")") SFUN(tag_qstring,s_getenv) END
OPEN(string_t) M("$") RETURN_IDENT("$") END

OPEN(list_e) GSB($list_f) PASS END
OPEN($list_f) GSB(list) PASS END
OPEN($list_f) M("{") GSB($block) M("}") PASS END

OPEN(list_e) GSB(list_e) M("&") GSB($list_f)  MERGE END
/*was SFUN(tag_list,s_concat_list); I am changing it back */
OPEN($arg) GSB($list_f) M(".") GSB(num_e) PROC(s_extract) END 
OPEN($arg) GSB($list_f) M(".length") PROC(s_list_length) END

OPEN($arg) GSB(qstring) M(".length") PROC(s_string_length) END

/* new in OpenZz */
OPEN($list_f) M("$zz$split") M("(") GSB(qstring) M(",") GSB(qstring) M(")") PROC(s_split) END

OPEN($arg) M("$current_line") SFUN(tag_int,get_current_line) END

OPEN($flow_block) LIST END
OPEN($flow_block) GSB($flow_block) M("{") GSB($flow_block) M("}") MERGE_ALL END
OPEN($flow_block) GSB($flow_block) GSB(any) APPEND END
OPEN($flow_block) GSB($flow_block) GSB(param) APPEND END

OPEN($block) LIST END
OPEN($block) GSB($block) M("{") GSB($block) M("}") MERGE_ALL END
OPEN($block) GSB($block) GSB(any) APPEND END
OPEN($block) GSB($block) M("\\{") PROC(s_append_bra) END
OPEN($block) GSB($block) M("\\}") PROC(s_append_ket) END

OPEN(stat) M("/print") GSB($arglist) SPROC(s_print) END
OPEN(stat) M("/error") GSB($arglist) SPROC(s_error) END

OPEN(stat) M("/max_error_n") GSB(num_e) PROC(set_max_error_n) END

OPEN(lvalue) GSB(ident) PASS END

  /*
    this rule allows param re-assignment, ie:
      /aa=1
      /aa=2
    ...
   */
OPEN(lvalue) GSB(param) PASS END

  /*
    ... so a de-reference rule is needed for indirect param definition, ie:
      /lvalue->"*" ident^$ :pass
      /bar=foo
      /*bar=22
     
    which has same effect as:
      /foo=22

   */
  OPEN(lvalue) M("*") GSB(ident) PASS END

  /*
    zz local var assignment
   */
OPEN(stat) M("/") GSB(lvalue) M("=") GSB($arg) GSB($argtype) 
	PROC(s_param_assign) END
  /*
    zz global var assignment
   */
OPEN(stat) M("/") GSB(lvalue) M(":=") GSB($arg) GSB($argtype) 
	PROC(s_param_g_assign) END
  /*
    zz global var plus delta assignment
    param is defined N scopes upper than the current one
   */
OPEN(stat) M("/") GSB(lvalue) GSB(int) M("=") GSB($arg) GSB($argtype) 
	PROC(s_param_gn_assign) END

OPEN(stat) M("/return") GSB($arg) GSB($argtype) PROC(s_return) END


OPEN($argtype) END
OPEN($argtype) M("as") GSB(ident) PASS END

OPEN(stat) M("/param") SPROC(list_params) END

OPEN(stat) M("/rules") GSB(ident) SPROC(list_rules) END
OPEN(stat) M("/krules") GSB(ident) SPROC(list_krules) END
OPEN(stat) M("/rules") SPROC(list_all_rules) END
OPEN(stat) M("/krules") SPROC(list_all_krules) END
OPEN(stat) M("/write rules") GSB(filename) SPROC(write_rules) END

OPEN(stat) M("/trace") GSB(num_e) SPROC(s_trace) END

OPEN(stat) M("/dumpnet") GSB(ident) PROC(s_dumpnet) END

OPEN(stat) M("/execute") GSB(list_e) PROC(s_exec) END

OPEN(stat) M("/include") GSB(filename) PROC(s_include) END
OPEN(stat) M("/include") M("<") GSB(filename) M(">") PROC(s_include_default) END
OPEN(filename) GSB(qstring) PASS END
OPEN(filename) GSB(ident) PASS END
OPEN(filename) GSB(ident) M(".") GSB(ident)
     SFUN(tag_qstring,strcat_filename) END

OPEN(stat) M("/add_includedir") GSB(filename) PROC(s_add_includedir) END
OPEN(stat) M("/print_includedirs") PROC(s_print_includedirs) END

OPEN(int) M("/load_lib") GSB(qstring) PROC(s_load_lib) END

OPEN(stat) M("/foreach") GSB(lvalue) M("in") GSB(list_e)
  M("{") GSB($flow_block) M("}") PROC(s_foreach) END

OPEN(stat) M("/for") GSB(lvalue) M("=") GSB(num_e) M("to") GSB(num_e)
  M("{") GSB($flow_block) M("}") PROC(s_for) END
OPEN(stat) M("/for") GSB(lvalue) M("=") GSB(num_e) M("to") GSB(num_e)
  M("step") GSB(num_e) M("{") GSB($flow_block) M("}") PROC(s_for) END
OPEN(stat) M("/if") GSB($cond_e) 
  M("{") GSB($flow_block) M("}") PROC(s_if) END
OPEN(stat) M("/if") GSB($cond_e)  
  M("{") GSB($flow_block) M("}") 
  M("else") M("{") GSB($flow_block) M("}") PROC(s_ifelse) END


OPEN(identlist) GSB(ident) LIST END
OPEN(identlist) GSB(identlist) M(",") GSB(ident) APPEND END

OPEN(stat) M("/memory") PROC(show_memory) END
OPEN(stat) M("/report") PROC(print_report) END
OPEN(stat) M("/lazy") PROC(print_lazy_report) END
OPEN(stat) M("/subtag") GSB(string_e) GSB(string_e) SPROC(subtag) END

OPEN(stat) M("/beep") PROC(proc_beep) END
OPEN(stat) M("/beep") GSB(string_e) PROC(proc_beep) END
OPEN(stat) M("/bye") PROC(proc_quit) END
OPEN(stat) M("$pretend_eof") PROC(pretend_eof) END
OPEN(stat) M("/readonce") GSB(ident) SPROC(read_once_only) END

OPEN(float)  M("cast_to_float") M("(") GSB(double) M(")") PROC(zz_doubletofloat) END
OPEN(int)  M("$zz$qtoi") GSB(qstring) SFUN(tag_int,zz_qtoi) END
OPEN(qstring) M("$zz$hexify") M("(") GSB(int) M(")") SFUN(tag_qstring,zz_inttohex) END
OPEN(qstring) M("$zz$hexify") M("(") GSB(int64) M(")") PROC(zz_int64tohex) END
OPEN(qstring) M("$zz$stringify") M("(") GSB(int) M(")") SFUN(tag_qstring,zz_inttostring) END

OPEN(qstring) M("tag_of") M("(") GSB(param) M(")") PROC(s_tag_of) END
}


static char rcsid[] = "$Id: kernel.c,v 1.25 2002/06/03 11:06:13 kibun Exp $ ";
