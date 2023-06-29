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

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <dlfcn.h>
#include <ctype.h>

#include "zlex.h"
#include "list.h"
#include "rule.h"
#include "err.h"
#include "mem.h"
#include "param.h"
#include "sys.h"
/*#define DEBUG 1*/
#include "trace.h"

extern FILE *zz_chanout;
extern struct s_content zz_ret_value;

/*PROTOTYPES*/
char *get_source_name(void);
int get_source_line(void);
void error_token(struct s_content *);
void error_head(int);
void error_tail_1(void);
int parse(struct s_nt *);
void show_zlex_memory(void);
void show_rule_memory(void);

const char* zz_includes = "";

long zztrace=0x0;

const char *zz_include_default_extension = "hz";

void get_extension(char*, char*); //Max : 05.04.01

static int sys_qstring_mem=0;
int include_fatal=0;

#define MAX_INCLUDEDIRS 20
static char *zz_includedirs[MAX_INCLUDEDIRS];
static int zz_num_includedirs=0;

/*----------------------------------------------------------------------------*/

// Macros to simplify use of s_tag cast function:
#define to_int(SRC)    ({ struct s_content ss; s_content_ivalue(*((SRC).tag->cast(&SRC, tag_int, &ss)));})
#define to_int64(SRC)  ({ struct s_content ss; s_content_llvalue(*((SRC).tag->cast(&SRC, tag_int64, &ss)));})
#define to_float(SRC)  ({ struct s_content ss; s_content_fvalue(*((SRC).tag->cast(&SRC, tag_float, &ss)));})
#define to_double(SRC) ({ struct s_content ss; s_content_dvalue(*((SRC).tag->cast(&SRC, tag_double, &ss)));})

// set highest type macro equal to highest type numerical
//  macro - used in comparison functons (ie. e_eq...)
#define to_highest_type(SRC)  to_double(SRC)

/*----------------------------------------------------------------------------*/
/* Utilities for algabraic operations */

/**
 * DATATYPE_ORDERING - for determining result (target) types for
 * algebraic operations (ordering for arithmetic promotion).
 * Operation params are promoted to highest op-type listed here:
 */
typedef enum { TYPE_INVALID, TYPE_INT, TYPE_INT64, TYPE_FLOAT, TYPE_DOUBLE } DATATYPE_ORDERING;


/**
 * is_numeric() - returns true(1) for types which are valid
 *   for use in algabraic operations (+/-*,sign change...)
 */
#define is_numeric(SRC) \
(((SRC).tag==tag_int || \
  (SRC).tag==tag_int64 || \
  (SRC).tag==tag_float || \
  (SRC).tag==tag_double \
 ) ? 1 : 0)


/**
 * For a given argument return an enum value representing it's type
 */
DATATYPE_ORDERING get_rank_for_type(val)
     struct s_content *val;
{
  if (val->tag==tag_int)
    return TYPE_INT;

  if (val->tag==tag_int64)
    return TYPE_INT64;

  if (val->tag==tag_float)
    return TYPE_FLOAT;

  if (val->tag==tag_double)
    return TYPE_DOUBLE;

  return TYPE_INVALID;
}


/**
 * s_target_type() - determine result type for an algabraic operation.
 * Checks validitiy of args and determines target type.
 * If either type is not valid for numerical operations, 
 * TYPE_INVALID is returned.
 */
struct s_tag *s_target_type(argc, argv)
     int argc;
     struct s_content argv[];
{
  int iArg0, iArg1;
  
  if(argc!=2) {
    zz_error(ERROR,"s_target_type: invalid argument count, expecting 2");
    return NULL;
  }
  
  iArg0 = get_rank_for_type(&argv[0]);
  iArg1 = get_rank_for_type(&argv[1]);

  if (iArg0==TYPE_INVALID || iArg1==TYPE_INVALID)
    return NULL;

  return argv[ ((iArg0 > iArg1) ? 0 : 1) ].tag;
}


/*----------------------------------------------------------------------------*/

s_print(list)
     struct s_list *list;
{
  int i;

  for(i=0;i<list->n;i++) {
    fprintz(zz_chanout,"%z ",list->array+i);
  }

  fprintz(zz_chanout,"\n");

  return 1;
}

/*---------------------------------------------------------------------------*/

int s_error(struct s_list *list)
{
  int i;
  error_head(2);

  for(i=0;i<list->n;i++)
    error_token(list->array+i);
  error_tail_1();

  return 1;
}

/*---------------------------------------------------------------------------*/

s_dump(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  int i;
  fprintz(zz_chanout,"argc=%d\n",argc);
  for(i=0;i<argc;i++)
    {
      fprintz(zz_chanout,"argv[%d] = %z\n",i,argv+i); 
    }
}

/*---------------------------------------------------------------------------*/

int s_param_assign(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
  struct s_content value;
  value=argv[1];
  if(argv[2].tag==tag_ident)
    value.tag = find_tag(s_content_svalue(argv[2]));
  set_param(s_content_svalue(argv[0]),&value);
  return 1;
}


/*---------------------------------------------------------------------------*/

s_param_g_assign(int argc, struct s_content *argv , struct s_content*ret)
{
  struct s_content value;
  value=argv[1];
  if(argv[2].tag==tag_ident)
    value.tag = find_tag(s_content_svalue(argv[2]));

  gset_param(s_content_svalue(argv[0]), &value);
  return 1;
} 

/*---------------------------------------------------------------------------*/

s_param_gn_assign(int argc, struct s_content argv[], struct s_content *ret)
{
  int delta;
  struct s_content value;
  value=argv[2];
  delta = (int)s_content_value(argv[1]);
  if(argv[3].tag==tag_ident)
    value.tag = find_tag(s_content_svalue(argv[3]));
  gnset_param(s_content_svalue(argv[0]),&value,delta);
  return 1;
}


/*----------------------------------------------------------------------------*/


s_dumpnet(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
if(argc<=0 || argv[0].tag!=tag_ident)
  {
   zz_error(ERROR,"dumpnet: bad argument");return 0;
  }
dumpnet(s_content_value(argv[0]));
return 1;
}


/*--------------------------------------------------------------------------*/


s_trace(trace)
     int trace;
{
  zztrace = trace;
}


/*---------------------------------------------------------------------------*/


s_eq(int argc, struct s_content argv[], struct s_content* ret)
{
  ret->tag=tag_int;

  /*
    printf("in s_eq:\n");
    fprintz(zz_chanout, " argv[0] = %z(%s)\n", &argv[0], argv[0].tag->name);
    fprintz(zz_chanout, " argv[1] = %z(%s)\n", &argv[1], argv[1].tag->name);
  */

  if( is_numeric(argv[0]) && is_numeric(argv[1]) ) {
    s_content_value(*ret) = (to_highest_type(argv[0]) == to_highest_type(argv[1]));
  }
  else if(argv[0].tag==tag_qstring && argv[1].tag==tag_qstring ||
	  argv[0].tag==tag_ident && argv[1].tag==tag_ident)
    {
      s_content_value(*ret) = ((strcmp((char *)s_content_value(argv[0]),
				       (char *)s_content_value(argv[1]))==0) ? 1 : 0);
    }
  else {
    s_content_value(*ret) = 0;
  }

  return 1;
}

/*---------------------------------------------------------------------------*/

s_ne(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  if(s_eq(argc,argv,ret)) {
    s_content_value(*ret) = ! (int)s_content_value(*ret);
    return 1; 
  }
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

s_ge(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  ret->tag=tag_int;

  if( is_numeric(argv[0]) && is_numeric(argv[1])) {
    s_content_value(*ret) = ( to_highest_type(argv[0]) >= to_highest_type(argv[1]) ? 1:0);
  }
  else if(argv[0].tag==tag_qstring && argv[1].tag==tag_qstring ||
     argv[0].tag==tag_ident && argv[1].tag==tag_ident)
    {
      s_content_value(*ret) = ((strcmp((char*)s_content_value(argv[0]),(char*)s_content_value(argv[1]))>=0) ? 1 : 0);
    }
  else 
    s_content_value(*ret)=0;

  return 1;
}

/*---------------------------------------------------------------------------*/

s_gt(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  ret->tag=tag_int;

  if( is_numeric(argv[0]) && is_numeric(argv[1])) {
    s_content_value(*ret) = to_highest_type(argv[0]) > to_highest_type(argv[1]) ? 1:0;
  }
  else if(argv[0].tag==tag_qstring && argv[1].tag==tag_qstring ||
	  argv[0].tag==tag_ident && argv[1].tag==tag_ident)
    {
      s_content_value(*ret) = (strcmp((char*)s_content_value(argv[0]),(char*)s_content_value(argv[1]))>0) ? 1 : 0;
    }
  else 
    s_content_value(*ret)=0;
  
  return 1;
}

/*---------------------------------------------------------------------------*/

s_le(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  if(s_gt(argc,argv,ret)) {
    s_content_value(*ret) = ! (int)s_content_value(*ret);
    return 1; 
  }
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

s_lt(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  if(s_ge(argc,argv,ret)) {
    s_content_value(*ret) = !s_content_value(*ret);
    return 1; 
  }
  else
    return 0;
}

/*---------------------------------------------------------------------------*/

s_add(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  struct s_tag *targetType = s_target_type(argc, argv);

  if (!targetType) {
    zz_error(ERROR,"Error in s_add, unexpected argument types.");
    s_dump(argc,argv,ret);
    return 0;
  }
  
  ret->tag = targetType;

  if (targetType == tag_int)
    s_content_ivalue(*ret) = to_int(argv[0]) + to_int(argv[1]);
  else if (targetType == tag_int64)
    s_content_llvalue(*ret) = to_int64(argv[0]) + to_int64(argv[1]);
  else if (targetType == tag_float)
    s_content_fvalue(*ret) = to_float(argv[0]) + to_float(argv[1]);
  else if (targetType == tag_double)
    s_content_dvalue(*ret) = to_double(argv[0]) + to_double(argv[1]); 

  zz_trace("s_add: %z(%s)+%z(%s)=%z(%s)\n", 
	   argv,   s_content_tag(argv[0])->name,
	   argv+1, s_content_tag(argv[1])->name,
	   ret,    s_content_tag(*ret)->name);

  return 1;
}


/*----------------------------------------------------------------------------*/


s_boolean_and(argc, argv, ret)
     int argc;
     struct s_content argv[], *ret;
{
  if (argc != 2) {
    zz_error(ERROR, "Error in s_boolean_and: wrong number of arguments(%i)", argc);
    return 0;
  }

  if (argv[0].tag != tag_int) {
    zz_error(ERROR, "Error in s_boolean_and: first argument(argv[0]) not int(%s)", argv[0].tag->name);
    return 0;
  }

  if (argv[1].tag != tag_int) {
    zz_error(ERROR, "Error in s_boolean_and: second argument(argv[1]) not int(%s)", argv[1].tag->name);
    return 0;
  }

  ret->tag=tag_int;

  if( s_content_ivalue(argv[0]) && s_content_ivalue(argv[1]) )
    s_content_ivalue(*ret) = 1;
  else 
    s_content_ivalue(*ret) = 0;

  return 1;
}


/*----------------------------------------------------------------------------*/


s_boolean_or(argc, argv, ret)
     int argc;
     struct s_content argv[], *ret;
{
  if (argc != 2) {
    zz_error(ERROR, "Error in s_boolean_or: wrong number of arguments(%i)", argc);
    return 0;
  }

  if (argv[0].tag != tag_int) {
    zz_error(ERROR, "Error in s_boolean_or: first argument(argv[0]) not int(%s)", argv[0].tag->name);
    return 0;
  }

  if (argv[1].tag != tag_int) {
    zz_error(ERROR, "Error in s_boolean_or: second argument(argv[1]) not int(%s)", argv[1].tag->name);
    return 0;
  }

  ret->tag=tag_int;

  if( s_content_ivalue(argv[0]) || s_content_ivalue(argv[1]) )
    s_content_ivalue(*ret) = 1;
  else 
    s_content_ivalue(*ret) = 0;

  return 1;
}


/*----------------------------------------------------------------------------*/


s_sub(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  struct s_tag *targetType = s_target_type(argc, argv);

  if (!targetType) {
    zz_error(ERROR,"Error in s_sub");
    return 0;
  }
  
  ret->tag = targetType;

  if (targetType == tag_int)
    s_content_ivalue(*ret) = to_int(argv[0]) - to_int(argv[1]);
  else if (targetType == tag_int64)
    s_content_llvalue(*ret) = to_int64(argv[0]) - to_int64(argv[1]);
  else if (targetType == tag_float)
    s_content_fvalue(*ret) = to_float(argv[0]) - to_float(argv[1]);
  else if (targetType == tag_double)
    s_content_dvalue(*ret) = to_double(argv[0]) - to_double(argv[1]); 

  return 1;
}


/*---------------------------------------------------------------------------*/

s_mult(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  struct s_tag *targetType = s_target_type(argc, argv);

  if (!targetType) {
    zz_error(ERROR,"Error in s_mult");
    return 0;
  }
  
  ret->tag = targetType;

  if (targetType == tag_int)
    s_content_ivalue(*ret) = to_int(argv[0]) * to_int(argv[1]);
  else if (targetType == tag_int64)
    s_content_llvalue(*ret) = to_int64(argv[0]) * to_int64(argv[1]);
  else if (targetType == tag_float)
    s_content_fvalue(*ret) = to_float(argv[0]) * to_float(argv[1]);
  else if (targetType == tag_double)
    s_content_dvalue(*ret) = to_double(argv[0]) * to_double(argv[1]); 

  return 1;
}

/*----------------------------------------------------------------------------*/

s_div(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  struct s_tag *targetType = s_target_type(argc, argv);

  if (!targetType) {
    zz_error(ERROR,"Error in s_div");
    return 0;
  }

  // Check for division by zero - promote to top type (won't loose precision) and test
  if (to_highest_type(argv[1]) == 0.0) {
    zz_error(ERROR,"Error in s_div - division by zero");
    return 0;
  }

  ret->tag = targetType;

  if (targetType == tag_int)
    s_content_ivalue(*ret) = to_int(argv[0]) / to_int(argv[1]);
  else if (targetType == tag_int64)
    s_content_llvalue(*ret) = to_int64(argv[0]) / to_int64(argv[1]);
  else if (targetType == tag_float)
    s_content_fvalue(*ret) = to_float(argv[0]) / to_float(argv[1]);
  else if (targetType == tag_double)
    s_content_dvalue(*ret) = to_double(argv[0]) / to_double(argv[1]); 

  return 1;
}

/*----------------------------------------------------------------------------*/

s_chs(argc,argv,ret)         /* s_chs - change of sign */
     int argc;
     struct s_content argv[],*ret;
{
  if(argc!=1) {
    zz_error(ERROR,"chs: bad argument number");
    return 0;
  }
  
  if (is_numeric(argv[0])) {  
    ret->tag = argv[0].tag;
    
    if(argv[0].tag==tag_int) {
      s_content_value(*ret) = - s_content_value(argv[0]);
    }
    else if(argv[0].tag==tag_int64) {
      s_content_llvalue(*ret) = - s_content_llvalue(argv[0]);
    }
    else if(argv[0].tag==tag_float) {
      // f = - s_content_fvalue(argv[0]);
      s_content_fvalue(*ret) = - s_content_fvalue(argv[0]); 
    }
    else if(argv[0].tag==tag_double) {
      s_content_dvalue(*ret) = - s_content_dvalue(argv[0]);
    }
    else {
      zz_error(ERROR,"chs: unsupported type");
      return 0;
    }
  }
  else {
    ret->tag=tag_none;
    s_content_value(*ret)=0;
    zz_error(ERROR,"chs: bad argument type(non numeric)");
    return 0;
  }
  
 return 1;
}

/*--------------------------------------------------------------------------*/

s_strcat(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
char buffer[256];
char *w,*s;
float f;
int i,len,chunk,size,l;
if(argc!=2) {zz_error(ERROR,"strcat: bad argument number");return 0;}

chunk = 20;
s = (char*)malloc(chunk);
size = chunk;
sys_qstring_mem+=chunk;
len = 0;
*s='\0';
for(i=0;i<argc;i++)
  {
   if(argv[i].tag==tag_char 
      || argv[i].tag==tag_ident 
      || argv[i].tag==tag_qstring)
      w=(char*)(s_content_value(argv[i]));
   else
     {
      sprintz(buffer,"%z",&argv[i]);
      w=buffer;
     }
   l=strlen(w);
   if(l+len+1>size)
     {
      while(l+len+1>size) {size+=chunk;sys_qstring_mem+=chunk;}
      s=(char*)realloc(s,size);
     }
   strcpy(s+len,w);
   len+=l;
  }
w=s;

zlex(&w,ret);

if(*w || ret->tag==tag_eol)
  {
   ret->tag=tag_qstring;
   s_content_value(*ret)=(long)s;
  }

/*   
correzione per centomille: 
scommentare qui e commentare da prima di zlex(&w,ret); a qui
ret->tag=tag_qstring;
ret->value=(long)s;
*/
return 1;
}


/*----------------------------------------------------------------------------*/

s_return(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
  zz_ret_value=argv[0];

  if(argv[1].tag==tag_ident)
    zz_ret_value.tag = find_tag(s_content_svalue(argv[1]));

  argv[1].tag=tag_none; /* brutto. serve per evitare che venga fatto delete
			   cfr. fine di action.c */
  return 1;
}

/*----------------------------------------------------------------------------*/

s_exec(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
source_list(&argv[0]);
parse(find_nt("root"));
pop_source();
return 1;
}

/*----------------------------------------------------------------------------*/

s_dumplist(argc,argv,ret)
int argc;
struct s_content argv[];
int ret;
{
int i;
struct s_list *lst;
lst = (struct s_list*)s_content_value(argv[0]);
for(i=0;i<lst->n;i++)
  printz("[%d] = %z\n",i,lst->array+i);
return 1;

}


/*--------------------------------------------------------------------------*/

s_foreach(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
int i,created,rr;
char *paramname;
struct s_list *lst;
struct s_content blk,paramval;
paramname = (char*)s_content_value(argv[0]);
lst = (struct s_list *) s_content_value(argv[1]);
blk = argv[2];
 created = 0; /* if new param is created, delete it at the end, but
		 never the case as if argv[0] is already present, it'd get
		 subst'd giving an error*/
               
for(i=0;i<lst->n;i++)
  {
   rr=set_param(paramname,lst->array+i);
   if(i==0) created=rr;
   source_list(&blk);
   parse(find_nt("root"));
   pop_source();
  }
if(created) unset_param(paramname);
}


/*--------------------------------------------------------------------------*/

s_for(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  int from,to,step,i,created,rr;
  char *paramname;
  struct s_content blk,paramval;

  paramname = (char*)s_content_value(argv[0]);
  from = s_content_value(argv[1]);
  to = s_content_value(argv[2]);

  if(argc==5)
    {
      step= s_content_value(argv[3]);
      blk = argv[4];
    }
  else
    {
      step=1;
      blk = argv[3];
    }

  paramval.tag=tag_int;

  for(i=from;i<=to;i+=step)
    {
      s_content_value(paramval)=i;
      rr=set_param(paramname,&paramval);
      if(i==from) created=rr;
      source_list(&blk);
      parse(find_nt("root"));
      pop_source();
    }

  if(created) 
    unset_param(paramname);
}


/*--------------------------------------------------------------------------*/

/*
 * The following functions provide the ability to echo their arguments
 * back to the parser - this allows structures (do, while) that need to
 * to continually re-evaluate their arguments to build an uninterpreted
 * string from them for later evaluation.
 */

/* Echo back an argument surrounded by parens as a list (with parens) */
s_condecho_passparens(argc,argv, ret)
     int argc;
     struct s_content argv[], *ret;
{
  struct s_content tmp;

  create_list(ret,3);
  ret->tag = tag_list;
  
  tmp.tag=tag_char;
  s_content_value(tmp)= (int)zlex_strsave("(");
  append_to_list(ret,&tmp);
  
  merge_list(ret, &argv[0]);

  tmp.tag=tag_char;
  s_content_value(tmp)= (int)zlex_strsave(")");
  append_to_list(ret,&tmp);

  return 1;
}


s_condecho_chs(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  struct s_content tmp;

  create_list(ret, 2);
  ret->tag = tag_list;

  tmp.tag=tag_char;
  s_content_value(tmp)= (int)zlex_strsave("-");
  append_to_list(ret, &tmp);

  merge_list(ret, &argv[0]);

  return 1;
}


/* 
   Called by unary and binary loop operations 
   to relect arguments while retaining operators.
*/
s_condecho(argc,argv, action, ret)
     int argc;
     char *action;
     struct s_content argv[], *ret;
{
  int i, action_len;
  struct s_content tmp;
  char char_tmp[] = " ";

  action_len = strlen(action);

  create_list(ret, 2 + action_len);
  ret->tag = tag_list;

  merge_list(ret, &argv[0]);

  // Insert chars of action - each one a seperate index in array
  for (i=0; i<action_len; i++) {
    tmp.tag=tag_char;
    char_tmp[0] = action[i];
    s_content_value(tmp)= (int)zlex_strsave(char_tmp);
    append_to_list(ret,&tmp);
  }

  // append_to_list(ret,&argv[1]);
  merge_list(ret,&argv[1]);

  return 1;
}

s_condecho_and(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "&&", ret);
}

s_condecho_or(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "||", ret);
}

s_condecho_add(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "+", ret);
}

s_condecho_sub(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "-", ret);
}

s_condecho_mult(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "*", ret);
}

s_condecho_div(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "/", ret);
}

s_condecho_eq(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "==", ret);
}

s_condecho_ne(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "!=", ret);
}

s_condecho_ge(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, ">=", ret);
}

s_condecho_gt(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, ">", ret);
}

s_condecho_le(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "<=", ret);
}

s_condecho_lt(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  return s_condecho(argc, argv, "<", ret);
}

/*--------------------------------------------------------------------------*/


int s_do(int argc, struct s_content argv[], struct s_content* ret)
{
  s_do_while_loops(argc,argv,&ret,0);
}

int s_while(int argc, struct s_content argv[], struct s_content* ret)
{
  s_do_while_loops(argc,argv,&ret,1);
}


/**
 * s_while() - while loop functionality
 *  Function must reinterpret the boolean condition of the while
 *  while loop for each pass of the loop (different from 'for' loop
 *  where interpretation is only done at parse of loop).
 */
s_do_while_loops(argc,argv,ret,while_loop)
     int argc;
     struct s_content argv[], *ret;
     int while_loop;
{
  char *param_name, *ctmp, loop_var_name[] = "              ";
  static int loop_var_name_count = 0;
  int subs_result,loop_control_flag;
  char suffix_array[] = "                                                                    ";
  char *suffix;
  struct s_content lst, tmp, blk, cond, loop_var;

  if (while_loop) {
    blk = argv[1];
    cond = argv[0];
  }
  else {             // "do" loop 
    blk = argv[0];
    cond = argv[1];
  }

  // Because we cannot get a return value from the internal "exec()" 
  // function, we need to wrap the boolean condition code of the
  // loop in an 'if' statement that sets a local var, and then test
  // that variable.  Because we could have nested loops, each call to
  // this function needs to have a uniquely named local var:
  sprintf(loop_var_name, "$zz$while_%i", loop_var_name_count++);
  loop_var.tag=tag_ident;
  s_content_ivalue(loop_var)= (int)zlex_strsave(loop_var_name);

  // Need to test to make sure this new 'temp' var name is not already
  // in use in this scope ...
  while (param_substitute(&loop_var,&ctmp)) {
    sprintf(loop_var_name, "$zz$while_%i", loop_var_name_count++);
    loop_var.tag=tag_ident;
    s_content_ivalue(loop_var)= (int)zlex_strsave(loop_var_name);
  }

  // Make an executable command out of the boolean condition that sets
  // the local temp variable based on the reulsts of the boolean op.
  create_list(&lst,15);

  tmp.tag=tag_char;
  s_content_value(tmp)= (int)zlex_strsave("/");
  append_to_list(&lst,&tmp);

  tmp.tag=tag_ident;
  s_content_value(tmp)= (int)zlex_strsave("if");
  append_to_list(&lst,&tmp);
  
  // printf("Merging\n");
  // fprintz(zz_chanout, "List:%z\n", &lst);
  // fprintz(zz_chanout, "Cond:%z\n", &cond);

  merge_list(&lst, &cond);

  sprintf(suffix_array, "{/%s=1} else {/%s=0}", loop_var_name, loop_var_name);

  suffix = &suffix_array[0];

  while(*suffix) {
    zlex(&suffix,&tmp);
    append_to_list(&lst,&tmp);
  }

  // Execute the boolean condition...
  if (while_loop) {
    s_exec(1, &lst, NULL);

    // Retreive value of boolean test variable
    loop_var.tag=tag_ident;
    s_content_ivalue(loop_var)= (int)zlex_strsave(loop_var_name);
    subs_result = param_substitute(&loop_var,&ctmp);
  }
  else
    subs_result = 1; // "do" loop always gets first pass

  // Only start loop if local tmp var was successfully read - if it wasn't
  // it means there was some compilation/run error in the boolean condition
  if (subs_result) {

    if (while_loop)
      loop_control_flag = s_content_ivalue(loop_var);
    else
      loop_control_flag = 1;     // "do" loop always gets first pass

    // While the test var is true (reinterpret in each pass), do the loop
    while ( loop_control_flag )
      {
	// Execute the code block of the loop
	source_list(&blk);

	// If there is a parse error in the loop body break out of loop
	if (!parse(find_nt("root"))) {
	  //  No need to report error - already done in parse
	  return 0;
	}
	pop_source();

	// Re-execute the boolean test condition
	s_exec(1, &lst, NULL);

	// Determine value of temp variable set in test command
	loop_var.tag=tag_ident;
	s_content_ivalue(loop_var)= (int)zlex_strsave(loop_var_name);
	param_substitute(&loop_var,&ctmp);

	loop_control_flag = s_content_ivalue(loop_var);
      }
  }

  // Remove temp list created for processing boolean condition
  delete_list(&lst);

  // Remove temp variable setup for testing boolean condition
  if (ctmp)
    unset_param(ctmp);

  return 1;
}


/*--------------------------------------------------------------------------*/


int s_if(int argc, struct s_content argv[], struct s_content* ret)
{
  int i,flag;
  char *paramname;
  struct s_content blk;

  blk = argv[1];

  if(s_content_value(argv[0]))
    {
      source_list(&blk);
      parse(find_nt("root"));
      pop_source();
    }
}


/*--------------------------------------------------------------------------*/

int s_ifelse(int argc, struct s_content argv[], struct s_content* ret)
{
  int i,flag;
  char *paramname;
  struct s_content blk;
  zz_assert(argc==3);
  if(s_content_value(argv[0]))
    {
      blk = argv[1];
    }
  else 
    {
      blk = argv[2];
    }
  source_list(&blk);
  parse(find_nt("root"));
  pop_source();
}


/*--------------------------------------------------------------------------*/

char *strcat_filename(name,type)
char *name,*type;
{
char *s;
s=(char*)malloc(strlen(name)+strlen(type)+2);
strcpy(s,name);
strcat(s,".");
strcat(s,type);
return s;
}

/*--------------------------------------------------------------------------*/

int s_include(int argc, struct s_content argv[], struct s_content* ret)
{
  char filename[FILENAME_MAX],type[40];

  zz_assert(zz_includes);

  strcpy(filename,zz_includes);
  if(argc==1) 
    {
      zz_assert(s_content_tag(argv[0]) == tag_qstring);
      strcat(filename,(char *)s_content_value(argv[0]));
    }
  else
    {
      zz_assert(s_content_tag(argv[0]) == tag_qstring);
      zz_assert(s_content_tag(argv[1]) == tag_qstring);
      strcat(filename,(char *)s_content_value(argv[0]));
      strcat(filename,".");
      strcat(filename,(char *)s_content_value(argv[1]));
    }
  /* DR: please review this....
     get_extension(filename,type);
     if(!type[0])
     change_extension(filename,zz_include_default_extension);
  */
  if(!source_file(filename))
    {
      zz_error(ERROR,"File %s not found",filename);
      if(include_fatal)
	{
	  zz_error(FATAL_ERROR,"Compilation aborted");
	  exit(1);
	}
      return 0;
    }
  else
    {
      parse(find_nt("root"));
      pop_source();
    }
  return 1;
}

/*--------------------------------------------------------------------------*/

int s_add_includedir(int argc, struct s_content argv[], struct s_content* ret)
{
  zz_assert(argc==1);
  if (zz_num_includedirs == MAX_INCLUDEDIRS-1) {
    zz_error(ERROR,"reached maximum defualt include directories");return 0;
  }
  zz_includedirs[zz_num_includedirs] = (char*) malloc(strlen(s_content_svalue(argv[0]))+2);
  strcpy(zz_includedirs[zz_num_includedirs], s_content_svalue(argv[0]));
  strcat(zz_includedirs[zz_num_includedirs], "/");
  zz_num_includedirs++;
}

/*--------------------------------------------------------------------------*/

int s_print_includedirs(int argc, struct s_content argv[], struct s_content* ret)
{
  int i;
  fprintf(zz_chanout, "Default Include Directories:\n");
  for (i = 0; i < zz_num_includedirs; i++) {
    fprintf(zz_chanout, "[%d] -> %s\n", i, zz_includedirs[i]);    
  }
}

/*--------------------------------------------------------------------------*/

int s_include_default(int argc, struct s_content argv[], struct s_content* ret)
{
  char filename[512],type[40];
  int i;
  for (i = 0; i < zz_num_includedirs; i++) {
    zz_assert(zz_includedirs[i]);
    strcpy(filename, zz_includedirs[i]);
    if(argc==1)
      strcat(filename,s_content_svalue(argv[0]));
    else
      {
	strcat(filename,s_content_svalue(argv[0]));
	strcat(filename,".");
	strcat(filename,s_content_svalue(argv[1]));
      }
    get_extension(filename,type);

    if(!type[0])
      change_extension(filename,zz_include_default_extension);

    zz_trace("include_default: trying <%s>\n", filename);
    if(source_file(filename))
      {
	parse(find_nt("root"));
	pop_source();
	return 1;
      }
  }  

  zz_error(ERROR,"File %s not found in any default include directory", s_content_svalue(argv[0]));

  if(include_fatal)
    {zz_error(FATAL_ERROR,"Compilation aborted");exit(1);}

  return 1;
}


/*----------------------------------------------------------------------------*/

/*
 * Handler for loading dynamic libraries.
 * There is currently no feature to unload libs:
 *  ie. dlclose(handle);
*/
int s_load_lib(argc,argv,ret)
     int argc;
     struct s_content argv[], *ret;
{
  void *handle;
  void (*init)();
  char *error, *lib_name;

  if (argc != 1) {
    zz_error(ERROR,"/load_lib called with incorrect # of params(), expecting 1.", argc);
    return 0;
  }

  /* zero the return value */
  s_content_tag(*ret)    = tag_int;
  s_content_pvalue(*ret) = 0;

  lib_name = s_content_svalue(argv[0]);

  if (!lib_name || (strlen(lib_name) == 0)) {
    zz_error(ERROR, "Required library name parameter to /load_lib missing.");
    return 0;
  }

  /*
    Try to open the library
    export its symbols to further dyn libs
  */ 
  handle = dlopen (lib_name, RTLD_LAZY|RTLD_GLOBAL);

  if (!handle) {
    zz_error(ERROR, "Error in /load_lib %s while trying to load library (%s).", lib_name, dlerror());
    return 0;
  }

  //zz_error(WARNING, "Library '%s' Loaded", lib_name);

  // The library has been loaded so now try to execute the "init()" 
  // function if it exists:

  init = dlsym(handle, "zz_ext_init");

  if ((error = dlerror()) != NULL) {
      zz_error(ERROR, "Error in /load_lib while trying to execute zz_ext_init() function of '%s': %s.", lib_name, error);
      dlclose(handle);
      return 0;
    }

  // init() function exists, execute it:
  (*init)();

  s_content_pvalue(*ret) = handle;

  return 1;
}


/*----------------------------------------------------------------------------*/

int dump_memory_usage(void)
{
char cmd[256];
strcpy(cmd,"ps -u");
strcat(cmd," | awk '$11==\"zz\" {print \"RSS=\" $6}'");
/*printf("cmd=%s\n",cmd);*/
system(cmd);
return 1;
}


/*---------------------------------------------------------------------------*/


int s_extract(int argc, struct s_content argv[], struct s_content* ret)
{
  struct s_content *cnt;
  struct s_content *list_extract();

  cnt=list_extract(&argv[0],s_content_value(argv[1])-1);

  if(cnt)
    *ret= *cnt;
  else
    ret->tag=tag_none;

  return 1;
}


/*---------------------------------------------------------------------------*/

int s_list_length(int argc, struct s_content argv[], struct s_content* ret)
{
struct s_content *cnt;
struct s_content *list_extract();
ret->tag = tag_int;
s_content_value(*ret) = ((struct s_list*)s_content_value(argv[0]))->n;
return 1;
}

/*----------------------------------------------------------------------------*/

int s_string_length(int argc, struct s_content argv[], struct s_content* ret)
{
  struct s_content *cnt;
  ret->tag = tag_int;
  s_content_value(*ret) =(s_content_value(argv[0]) ? strlen((char*)s_content_value(argv[0])):0);
  return 1;
}


/*----------------------------------------------------------------------------*/

// Implement split function for zz simillar to strtok() c function.
int s_split(int argc, struct s_content argv[], struct s_content* ret)
{
  char *pch;
  char *srcstr;
  const char *delimstr;
  struct s_content tmp;

  zz_assert(argv);
  zz_assert(ret);

  if (argc!=2) {
    zz_error(ERROR,"s_split() received incorrect number of arguments, got %i, expecting 2.", argc);
    exit(1);
  }

  create_list(ret,10);
  tmp.tag=tag_qstring;

  srcstr = strdup(s_content_svalue(argv[0]));
  delimstr = s_content_svalue(argv[1]);

  zz_assert(srcstr);
  zz_assert(delimstr);

  pch = strtok(srcstr, delimstr);

  while (pch != NULL) {
    s_content_svalue(tmp) = pch;

    append_to_list(ret,&tmp); 

    pch = strtok(NULL, delimstr);
  }
  
  /* 
     'tmp' is copied so it can be reused in each
     pass of the loop above, however it's internal
     values are copied by reference, and so the 
     memory where tha values are stored cannot be
     free'd.

     // do not do this =>  free(srcstr);
  */

  return 1;
}


/*----------------------------------------------------------------------------*/


void show_sys_memory(void)
{
PRINTMEM("sys.qstring",sys_qstring_mem)
}

/*---------------------------------------------------------------------------*/

subtag(tagson_name,tagparent_name)
char *tagson_name,*tagparent_name;
{
struct s_tag *tagson,*tagparent;
tagson = find_tag(tagson_name);
tagparent = find_tag(tagparent_name);
tagson->delete = tagparent->delete;
tagson->param_on = tagparent->param_on;
tagson->param_off = tagparent->param_off;
}

/*---------------------------------------------------------------------------*/

void show_memory(void)
{
printf("Memory usage\n");
show_zlex_memory();
show_list_memory();
show_rule_memory();
show_sys_memory();
show_param_memory();
}

/*---------------------------------------------------------------------------*/

static int Start_Time, Stop_Time;

static struct tbuffer
  {
  int proc_user_time,  proc_system_time;
  int child_user_time, child_system_time;
  }
  Time;



void init_time(void)
{
times (&Time);
Start_Time = Time.proc_user_time;
}


int get_time(void)
{
int t;
times (&Time);
Stop_Time = Time.proc_user_time;
t = Stop_Time - Start_Time;
return t; /* centesimi di secondo */
}



int proc_beep(int argc, struct s_content argv[], struct s_content *ret)
{
char *s;
int line_n;
int time;
float sec;
static int count=0;
time = get_time();
sec = (float)time*0.01;
if(argc==1)
  printz("** %z **    ",&argv[0]);
else
  printf("** %d **    ",count++);
s=get_source_name();
line_n=get_source_line();
printf("TIME %4.2fs    FILE %s   LINE %d\n",sec,s,line_n);
return 1;
}


/*---------------------------------------------------------------------------*/
static int bra_ket_defined = 0;
static struct s_content bra,ket;


int s_append_bra(int argc, struct s_content argv[], struct s_content *ret)
{
if(argc!=1 || argv[0].tag!=tag_list) 
  {printf("error - s_append_bra; bad parameters\n");exit(1);}
if(!bra_ket_defined)
  {
   char *s="{}";
   bra_ket_defined=1;
   zlex(&s,&bra);
   zlex(&s,&ket);
  }
*ret = argv[0];
append_to_list(ret,&bra);
return 1;
}


/*---------------------------------------------------------------------------*/

int s_append_ket(int argc, struct s_content argv[], struct s_content *ret)
{
if(argc!=1 || argv[0].tag!=tag_list) 
  {printf("error - s_append_ket; bad parameters\n");exit(1);}
if(!bra_ket_defined)
  {
   char *s="{}";
   bra_ket_defined=1;
   zlex(&s,&bra);
   zlex(&s,&ket);
  }
*ret=argv[0];
append_to_list(ret,&ket);
return 1;
}


/*---------------------------------------------------------------------------*/


char *s_getenv(name)
char *name;
{
int i;
char buffer[256];
char *r;
r = getenv(name);
if(!r) 
  {
   for(i=0;name[i];i++,r++)
     buffer[i]=toupper(name[i]);
   buffer[i]='\0';
   r=getenv(buffer);
   if(!r)r="";
  }
return zlex_strsave(r);
}


/*---------------------------------------------------------------------------*/


void proc_quit()
{
  exit(0);
}


/*---------------------------------------------------------------------*/

int zz_qtoi(char* q)
{ int i;
  i=strtol(q,(char **)0,0);
  return i;
}

/*---------------------------------------------------------------------------*/

int zz_inttohex(int i)
{ 
  char* q=calloc(20,sizeof(char));
  sprintf(q,"0x%x",i);
  return (int)q;
}

/*---------------------------------------------------------------------------*/

int zz_inttostring(int i)
{ 
  char* q=calloc(2,sizeof(char));
  int ret;
  // dr: seems a bug to me (q should get terminated by sprintf) ...
  //ret = sprintf(q,"%c",*(char*)&i);
  q[0]=(char)i;
  q[1]='\0';
  return (int)q;
}

/*---------------------------------------------------------------------------*/

int zz_int64tohex(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{ 
  char* q;
  long long val;
  zz_assert(ret);
  ret->tag=0; /* in case of error */
  if(argc!=1) {zz_error(ERROR,"stringify: bad argument number");return 0;}
  if(argv[0].tag!=tag_int64)
    {
      zz_error(ERROR,"stringify: bad argument number");return 0;
    }
  val = s_content_llvalue(argv[0]);
  q=calloc(22,sizeof(char));
  sprintf(q,"0x%Lx",val);
  
  ret->tag = tag_qstring;
  s_content_svalue(*ret) = q;
  return 1;
}

/*---------------------------------------------------------------------------*/

int s_tag_of(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  char *var_name;

  if (argc != 1) {
    zz_error(ERROR,"s_tag_of: incorrect number of arguments, expecitng 1, got %d.", argc);
    return 0;
  }

  var_name = zlex_strsave(s_content_svalue(argv[0]));

  if (!var_name) {
    zz_error(ERROR,"s_tag_of: unable to resolve var name '%s' - no variable found for literal.", \
             s_content_svalue(argv[0]));
    return 0;
  }

  if (!param_substitute(&argv[0], &var_name)) {
    zz_error(ERROR,"s_tag_of: unable to resolve variable '%s' - no value found for name.", \
             var_name);
    return 0;
  }

  ret->tag=tag_qstring;
  s_content_svalue(*ret) = argv[0].tag->name;
  return 1;
}

/*---------------------------------------------------------------------------*/

int zz_doubletofloat(argc,argv,ret)
     int argc;
     struct s_content argv[],*ret;
{
  zz_assert(ret);

  if(argc!=1) {
    zz_error(ERROR,"mult: bad argument number: %i", argc);
    return 0;
  }

  if(argv[0].tag!=tag_double) {
    zz_error(ERROR,"zz_doubletofloat: bad source argument type:%s", argv[0].tag->name);
    return 0;
  }

  ret->tag = tag_float;
  s_content_fvalue(*ret) = (float)s_content_dvalue(argv[0]);
}

/*---------------------------------------------------------------------------*/

void get_extension(char *fullfilename, char *filetype){ //Max:05.04.01
  char *r, *s, *v;
 
  r = rindex(fullfilename, '/');
 
  if(!r){
    r = rindex(fullfilename, '.');
    if(!r){
      *filetype = '\0';
      return;
    }
    else
      strcpy(filetype, r);
  }
  else{
    s = index(r, '.');
    if(!s){
      *filetype = '\0';
      return;
    }
    v = index(s, ';');
    if(v){
      strncpy(filetype, s, (int)(v-s));
      filetype[(v-s)+1] = '\0';
    }
    else
      strcpy(filetype,s);
  }
}          
/* --- Previous Version ---
get_extension(char *fullfilename,char *filetype) {
  char *r,*s,*v;
  r = rindex(fullfilename,'/');
  if( ! r ) return '\0';

  s = index(r,'.');
  if( ! s ) return '\0';

  v = index(s,';');
  if( v ) { strncpy(filetype,s,(int)(v-s)); filetype[(v-s)+1] = '\0'; }
  else strcpy(filetype,s);
}
*/
/*---------------------------------------------------------------------------*/

int change_extension(char *fullfilename, const char *filetype) {
  char tmp[256],*r,*s,*t;
  int i;
  if(*filetype=='.') filetype++;
  r = rindex(fullfilename,'/');
  if( ! r ) r = &fullfilename[strlen(fullfilename)];

  s = index(r,'.');
  if( ! s ) { strcat(fullfilename,".");strcat(fullfilename,filetype); return 0; }

  tmp[0] = '\0';
  t = index(s,';');
  if( t ) strcpy(tmp,t);

  strcpy(s+1,filetype);
  strcat(s,tmp);
}

/*---------------------------------------------------------------------------*/

void zz_set_trace_mask(int mask)
{
  zztrace=mask;
}

/*---------------------------------------------------------------------------*/

int zz_trace_mask()
{
  return zztrace;
}

/*---------------------------------------------------------------------------*/

void zz_set_default_include_dir(const char* default_include_dir)
{
  zz_assert(default_include_dir);
  zz_trace("zz_set_default_include_dir: oldval=<%s> newval=<%s>\n", zz_includes, default_include_dir);
  zz_includes=default_include_dir;
}

/*---------------------------------------------------------------------------*/



static char rcsid[] = "$Id: sys.c,v 1.33 2002/06/03 11:06:13 kibun Exp $ ";
