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
#include "zz.h"
#include "zlex.h"
#include "rule.h"
#include "err.h"
#include "avl.h"
#include "list.h"
#include "param.h"
#include "trace.h"

struct s_content zz_ret_value = {0,0};

/*--------------------------------------------------------------------------*/

struct s_content* zz_bind_get_ret_value()
{
  return &zz_ret_value;
}

/*--------------------------------------------------------------------------*/

#define MAX_ARGV 100

action(rule,stack,ret)
struct s_rule *rule;
struct s_content stack[],*ret;
{
  struct s_content cnt,old_zz_ret_value;
  struct s_content argv [MAX_ARGV]; /* allocation issue */
  char *namev[MAX_ARGV];
  int argc;
  int (*caction)();
  int i,j;

  zz_assert(rule);
  zz_assert(stack);
  zz_assert(ret);

  ret->tag=tag_none;
  s_content_value(*ret)=0;
  /* PREPARO I PARAMETRI */
  
  /* rule=rule->top_scope; */
  argc=0;
  for(i=1,j=0;i<rule->bead_n;i++,j++)
    /* push only non-terminals in argv
       to pass it during action execution */
    if(rule->beads[i].cnt.tag==tag_sint)
      {
	zz_assert(argc < MAX_ARGV);
	argv[argc] = stack[j];
	namev[argc] = rule->beads[i].name;
	argc++;
      }
  
  switch(rule->action_type)
    {
    case ACT_T_EXECUTE_PROC:
      caction = (int (*)())s_content_value(rule->action);
      //printf("MIAOOOOOO\n");
      ret->tag = rule->sproc_tag;
      //      if(ret->tag)
      //	printf("zz: EXECUTE_PROC: tag_name=%s\n", ret->tag->name);
      (*caction)(argc,argv,ret);
      break;
    case ACT_T_EXECUTE_SPROC:
      caction = (int (*)())s_content_value(rule->action);
      ret->tag = rule->sproc_tag;
      switch(argc)
	{
        case 0:
	  s_content_value(*ret) = (*caction)();
          break;
        case 1:
	  /*
	    printf("BONBOLO: tag=%p tag_name=%s value=0x%08x\n", 
		 argv[0].tag, argv[0].tag->name, argv[0].value);
	  */
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]));
          break;
        case 2:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]),
					     s_content_value(argv[1]));
          break;
        case 3:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]));
          break;
        case 4:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]));
          break;
        case 5:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]));
          break;
        case 6:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]));
          break;
        case 7:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]),
					     s_content_value(argv[6]));
          break;
        case 8:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]),
					     s_content_value(argv[6]), 
					     s_content_value(argv[7]));
          break;
        case 9:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]),
					     s_content_value(argv[6]), 
					     s_content_value(argv[7]),
					     s_content_value(argv[8]));
          break;
        case 10:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]),
					     s_content_value(argv[6]), 
					     s_content_value(argv[7]),
					     s_content_value(argv[8]), 
					     s_content_value(argv[9]));
          break;
        case 11:
	  s_content_value(*ret) = (*caction)(s_content_value(argv[0]), 
					     s_content_value(argv[1]), 
					     s_content_value(argv[2]), 
					     s_content_value(argv[3]),
					     s_content_value(argv[4]), 
					     s_content_value(argv[5]),
					     s_content_value(argv[6]), 
					     s_content_value(argv[7]),
					     s_content_value(argv[8]), 
					     s_content_value(argv[9]),
					     s_content_value(argv[10]));
          break;
        default:
          zz_error(ERROR,"lr_action: too many argument for s-procedure");
       }
     break;
   case ACT_T_EXECUTE_LIST:
     if(rule->action.tag!=tag_list)
       zz_error(ERROR,"lr_action: action list not found");
     else
       {
        static struct s_nt *root_nt=0;
        old_zz_ret_value=zz_ret_value;
        zz_ret_value = *ret;
        push_param_scope();
        for(i=0;i<argc;i++)
          set_param(namev[i],&argv[i]);        
        source_list(&rule->action);
        if(!root_nt) root_nt=find_nt("root");
	if(zz_trace_mask()&TRACE_ZZACTION)
	  {
	   printz("   @ vvvvvvvvvvvvvvvvvvvv  begin action of %r\n",rule);
	   parse(root_nt);
           printz("   @ ^^^^^^^^^^^^^^^^^^^^  end action of %r\n",rule);
	  }
	else
	  parse(root_nt);
        pop_source();
        pop_param_scope();
        *ret = zz_ret_value;
        zz_ret_value=old_zz_ret_value;
      }
     break;
   case ACT_T_NONE:
     break;
   case ACT_T_RETURN:
     *ret = rule->action;
     if(rule->sproc_tag) ret->tag = rule->sproc_tag;
     break;
   case ACT_T_PASS:
     *ret = argv[0];
     break;
   case ACT_T_RRETURN:
     zz_ret_value = argv[0];
     break;
   case ACT_T_ASSIGN:
     if(argc!=3) zz_error(ERROR,"assign: bad argument number");
     else if(argv[0].tag!=tag_ident) zz_error(ERROR,"assign: bad 1' arg");
     else
       {
	struct s_content value;
	value=argv[1];
	if(argv[2].tag==tag_ident)
	  value.tag = (struct s_tag*)find_tag(s_content_svalue(argv[2]));
        set_param(s_content_svalue(argv[0]),&value);
       }
     break;
   case ACT_T_LIST:
     create_list(ret,10);
     for(i=0;i<argc;i++)
       append_to_list(ret,&argv[i]);
     break;
   case ACT_T_MERGE:
     create_list(ret,10);
     for(i=0;i<argc;i++)
       if(argv[i].tag!=tag_list)
         append_to_list(ret,&argv[i]);
       else 
         merge_list(ret,&argv[i]);
     break;
   case ACT_T_MERGE_ALL:
     create_list(ret,10);
     for(i=0;i<rule->bead_n-1;i++)
       {
        cnt = stack[i];
        if(cnt.tag==tag_list)
          {
           struct s_list *lst;
           lst=(struct s_list*)s_content_value(cnt);
           for(j=0;j<lst->n;j++)
             append_to_list(ret,lst->array+j);
          }
        else       
          append_to_list(ret,&cnt);
       }
     break;
   case ACT_T_APPEND:
     if(argc==0 || argv[0].tag!=tag_list)
       {
        create_list(ret,10);
        for(i=0;i<argc;i++)
          append_to_list(ret,&argv[i]);
       }     
     else
       {
        *ret = argv[0];
        for(i=1;i<argc;i++)
          append_to_list(ret,&argv[i]);
       }
     break;
   default:
     zz_error(WARNING,"lr_action: unknown action type");
     break;  
  }


}

fprint_action(action)
struct content *action;
{



}

static char rcsid[] = "$Id: action.c,v 1.14 2002/02/13 14:26:28 kibun Exp $ ";
