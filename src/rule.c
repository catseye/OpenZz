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

/*==============================================================================

	rule.C

	() gestione non-terminali:

		struct s_nt *start_symbol;
		struct s_nt *find_nt();


	() per creare una regola:	
	
		open_rule(headname);
		append_bead(cnt,name);
		append_t_bead(cnt);
		append_nt_bead(ntname,name);

		setaction_exelist(list);
		setaction_exeproc(proc);
		setaction_exesproc(proc);
		setaction_return(value);
		setaction_pass(proc);
		setaction_list(proc);
		setaction_merge_all(proc);
		setaction_merge(proc);
		setaction_append(proc);

	   rule=close_rule();	


	() per distruggere una regola:
		free_rule(rule);



==============================================================================*/


#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "avl.h"
#include "rule.h"
#include "list.h"
#include "err.h"
#include "mem.h"
#include "trace.h"
#include "table.h"
#include "printz.h"

/*PROTOTYPES*/
int param_substitute();  /* signature int param_substitute(struct s_content *token,char **paramname) in param.h is incompatible */
int parse(struct s_nt *);
int source_list();  /* signature int source_list(struct s_content *, void *); is incompatible */
void pop_source(void);

static int nt_mem=0;
static int rule_mem=0;
struct s_nt *nt_any=0,*nt_param=0,*nt_gparam;

/* forwards, now exported in rule.h */
/*void init_rule();*/
/*void free_rule(struct s_rule *);*/
/*----------------------------------------------------------------------------*/

/*
extern struct s_dot *create_dot();
*/
/* now in table.h
  extern int link_rule(),unlink_rule();
*/

struct s_nt *start_symbol=0;

/*----------------------------------------------------------------------------*/

static TREE *nttree=0;

static struct s_rule *cur_rule=0,*last_rule=0;

#define RULES_SIZE 100
static TREE *rules[RULES_SIZE];
static int rules_sp=1;
static int init_rule_done=0;
int kernel_flag=1;

struct s_rule_class {
	char *name;
	TREE *rules;
	};

TREE *rule_classes=0;


/*----------------------------------------------------------------------------*/

void zz_set_kernel_flag(int bool_value)
{
  kernel_flag = bool_value;
}

/*----------------------------------------------------------------------------*/

struct s_nt *find_nt(char *name)
{
  struct s_nt *nt;
  zz_assert(name);
  if(!nttree)
    {
      nttree = avl_tree_nodup_str (struct s_nt, name);
      nt_param=find_nt("param");
      nt_gparam=find_nt("gparam");
      nt_any=find_nt("any");
    }
  nt = avl_locate(nttree,name);
  if(!nt)
    {
      nt = (struct s_nt*)calloc(1,sizeof(struct s_nt));
      nt->name = (char*)malloc(strlen(name)+1);
      nt_mem+=sizeof(struct s_nt)+strlen(name)+1;
      strcpy(nt->name,name);
      nt->prompt=0;
      avl_insert(nttree,nt);
      nt->first_dot = 0;
    }
  return nt;
}


/*----------------------------------------------------------------------------*/

int rulecmp(struct s_rule *r1,struct s_rule *r2)
{
  int i;
  zz_assert(r1 && r2);
  for(i=0;i<r1->bead_n && i<r2->bead_n;i++)
    {
      if(r1->beads[i].cnt.tag < r2->beads[i].cnt.tag)
	return -1;
      else if(r1->beads[i].cnt.tag > r2->beads[i].cnt.tag) 
	return 1;
      else if(s_content_value(r1->beads[i].cnt) < s_content_value(r2->beads[i].cnt))
	return -1;
      else if(s_content_value(r1->beads[i].cnt) > s_content_value(r2->beads[i].cnt))
	return 1;     
    }
  if(i<r1->bead_n) return -1;
  else if(i<r2->bead_n) return 1;
  else return 0;
}


/*----------------------------------------------------------------------------*/


void open_rule(char *ntname)
{
  struct s_content tmp1,tmp2;
  zz_assert(ntname);
  tmp1.tag = tag_ident;
  s_content_value(tmp1) = (long)ntname;
  param_substitute(&tmp1,&tmp2);
  ntname = (char*)s_content_value(tmp1);
  if(!init_rule_done) init_rule();
  if(cur_rule)
    free_rule(cur_rule, NULL);
  cur_rule = (struct s_rule *)calloc(1,sizeof(struct s_rule));
  rule_mem += sizeof(struct s_rule);
  cur_rule->bead_n=1;
  cur_rule->beads = (struct s_bead*)
    calloc(MAX_RULE_LENGTH,sizeof(struct s_bead));
  cur_rule->bead_size = MAX_RULE_LENGTH;
  rule_mem += sizeof(struct s_bead)*cur_rule->bead_size;
  cur_rule->beads[0].cnt.tag=tag_sint;
  s_content_value(cur_rule->beads[0].cnt)=(long)find_nt(ntname);
  cur_rule->beads[0].name=0;
  cur_rule->cnt_prec.tag = tag_sint;
  s_content_value(cur_rule->cnt_prec) = 0;
  cur_rule->when_exit_scope.tag = 0;
  cur_rule->when_change_action.tag = 0;
  cur_rule->kernel=kernel_flag;
}

/*---------------------------------------------------------------------------*/

void setaction_exelist(struct s_content *list)
{
  zz_assert(list);
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  if(list->tag!=tag_list)
    {
      zz_error(INTERNAL_ERROR, "setaction_exelist. argument is not a list\n");
      return;
    }
  cur_rule->action_type = ACT_T_EXECUTE_LIST;
  cur_rule->action = *list;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

void setaction_exeproc(int (*proc)(), struct s_tag *tag)
{
  zz_assert(proc);
  //zz_assert(tag);
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_EXECUTE_PROC;
  cur_rule->action.tag = tag_procedure;
  s_content_value(cur_rule->action) = (long)proc;
  cur_rule->sproc_tag = tag;
}

/*----------------------------------------------------------------------------*/

void setaction_exesproc(int (*sproc)(), struct s_tag *tag)
{
  zz_assert(sproc && tag);
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_EXECUTE_SPROC;
  cur_rule->action.tag = tag_procedure;
  s_content_value(cur_rule->action) = (long)sproc;
  cur_rule->sproc_tag = tag;
}

/*---------------------------------------------------------------------------*/

void setaction_return(struct s_content *value, char *tag_name)
{
  //struct s_tag *find_tag();
  zz_assert(value);

  // Cannot assert tag_name or else statements
  //  like "/float -> foo float^n : return n" fail.
  // zz_assert(tag_name);

  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "rule not open\n");return;}

  cur_rule->action_type = ACT_T_RETURN;
  cur_rule->action = *value;
  cur_rule->sproc_tag = (tag_name)?find_tag(tag_name):0;
}

/*----------------------------------------------------------------------------*/

void setaction_pass()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_PASS;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*---------------------------------------------------------------------------*/

void setaction_rreturn()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_RRETURN;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*---------------------------------------------------------------------------*/

void setaction_assign()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_ASSIGN;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

void setaction_list()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_LIST;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

void setaction_merge_all()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_MERGE_ALL;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

void setaction_merge()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_MERGE;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

void setaction_append()
{
  if(!cur_rule)
    {zz_error(INTERNAL_ERROR, "setaction: rule not open\n");return;}
  cur_rule->action_type = ACT_T_APPEND;
  cur_rule->action.tag = 0;
  s_content_value(cur_rule->action) = 0;
  cur_rule->sproc_tag = 0;
}

/*----------------------------------------------------------------------------*/

struct s_rule *close_rule()
{
  int i;
  struct s_rule *rule;
  struct s_bead *beads;
  if(!cur_rule)
    {
      zz_error(ERROR,"close_rule: rule not open");
      return 0;
    }
  rule = cur_rule;
  cur_rule = 0;
  beads = rule->beads;
  rule->beads = (struct s_bead*)
    calloc(rule->bead_n,sizeof(struct s_bead));
  rule_mem+=sizeof(struct s_bead)*(rule->bead_n-rule->bead_size);
  rule->bead_size=rule->bead_n;
  for(i=0;i<rule->bead_n;i++)
    rule->beads[i]=beads[i];
  free(beads);
  rule->segment_id=0;
  return rule;
}


/*----------------------------------------------------------------------------*/

void append_bead(struct s_content *cnt, char *name)
{
  int i;
  if(!cur_rule)
    {
      zz_error(ERROR,"append_bead: rule not open");
      return;
    }
  if(cur_rule->bead_n>=MAX_RULE_LENGTH)
    {
      zz_error(ERROR,"append_bead: rule too long");
      return;   
    }
  i=cur_rule->bead_n++;
  cur_rule->beads[i].cnt = *cnt;
  cur_rule->beads[i].name = name;
  if(cnt->tag==tag_qstring || cnt->tag==tag_ident || cnt->tag==tag_char)
    cur_rule->cnt_prec = *cnt;
}

/*----------------------------------------------------------------------------*/

void append_t_bead(struct s_content *cnt)
{
  char *s;
  struct s_content tmp;
  if(cnt->tag==tag_qstring)
    {
      s = (char*)s_content_value(*cnt);
      while(*s==' ' || *s=='\t')s++;
      while(*s)
	{
	  zlex(&s,&tmp);
	  append_t_bead(&tmp);
	  while(*s==' ' || *s=='\t')s++;
	}
    }
  else
    append_bead(cnt,0);
}

/*----------------------------------------------------------------------------*/

void append_nt_bead(char *ntname, char *beadname)
{
struct s_content tmp;
static char *dollar=0;
if(!beadname)
  {
   if(!dollar) dollar = zlex_strsave("$");
   beadname = dollar;   
  }
tmp.tag = tag_sint;
s_content_value(tmp) = (long)find_nt(ntname);
append_bead(&tmp,beadname);
}


/*============================================================================*/


static int fprint_rule(FILE *chan, void *_rule)
{
  int i;
  struct s_nt *nt;
  struct s_rule *rule = (struct s_rule *)_rule;
  if(!rule) 
    fprintz(chan,"(nil)");
  else if(rule->bead_n<0)
    fprintz(chan,"(nil)-> ");
  else
    { 
      nt = (struct s_nt*) s_content_value(rule->beads[0].cnt);
      fprintz(chan,"%s ->",nt->name);
      for(i=1;i<rule->bead_n;i++)
	{
	  if(rule->beads[i].cnt.tag==tag_sint)
	    fprintz(chan," %s^%s",
		    ((struct s_nt*)s_content_value(rule->beads[i].cnt))->name,
		    rule->beads[i].name);
	  else
	    fprintz(chan," %z",&rule->beads[i].cnt);
	}
    }
  return 1;
}

/*----------------------------------------------------------------------------*/

static int sprint_rule(char* buffer, void *_rule)
{
  struct s_nt *nt;
  int i;
  char *s;
  struct s_rule *rule=(struct s_rule *)_rule;

  if(!rule) 
    strcpy(buffer,"(nil)");
  else if(rule->bead_n<0)
    strcpy(buffer,"(nil)-> ");
  else
    { 
      nt = (struct s_nt*) s_content_value(rule->beads[0].cnt);
      strcpy(buffer,nt->name);
      strcat(buffer,"  ->");
      s = buffer+strlen(buffer);
      for(i=1;i<rule->bead_n;i++)
	{
	  if(rule->beads[i].cnt.tag==tag_sint)
	    sprintz(s," %s^%s",
		    ((struct s_nt*)s_content_value(rule->beads[i].cnt))->name,
		    rule->beads[i].name);
	  else
	    sprintz(s," %z",&rule->beads[i].cnt);
	  while(*s)s++;
	}
    }
  return 1;
}

/*----------------------------------------------------------------------------*/

void print_rule(struct s_rule *rule)
{
  printz("  %r\n",rule);
}

/*----------------------------------------------------------------------------*/

void init_rule()
{
  if(init_rule_done) return;
  init_rule_done=1;
  printz_code('r',fprint_rule,sprint_rule);
}

/*----------------------------------------------------------------------------*/

static int do_delete_scope_action(rule)
struct s_rule *rule;
{
   zz_trace("do_delete_scope_action()\n");

if(rule->when_exit_scope.tag==tag_list)
  {
   source_list(&rule->when_exit_scope);
   parse(find_nt("root"));
   pop_source();
   zz_trace("do_delete_scope_action() exit scope (%r): %z\n",rule,&rule->when_exit_scope);
  }
}

void free_rule(void *_rule, void *dummy_param)
{
  struct s_rule *rule = (struct s_rule *)_rule;
  do_delete_scope_action(rule);
  rule_mem-=sizeof(struct s_rule)-sizeof(struct s_bead)*rule->bead_size;
  free(rule->beads);
  free(rule);
}

/*============================================================================*/

void show_rule_memory()
{
  PRINTMEM("rule",rule_mem);
  PRINTMEM("non-terminal",nt_mem);
}

/*============================================================================*/

/*

azioni:

		  sproc_tag    action_type          action.tag 
	action	:              ACT_T_EXECUTE_PROC   tag_procedure
	saction : tag_xxx      ACT_T_EXECUTE_SPROC  tag_procedure
	pass    :              ACT_T_PASS
	list    :              ACT_T_LIST
	append  :              ACT_T_APPEND
        ret/proc:	       ACT_T_RETURN	    tag_procedure

*/

static char rcsid[] = "$Id: rule.c,v 1.18 2002/06/03 11:06:13 kibun Exp $ ";
