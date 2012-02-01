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
#include "rule.h"
#include "err.h"
#include "mem.h"
#include "param.h"
#include "trace.h"

#define PARAM_SCOPE_STACK_SIZE 50

struct s_param_pair {
	char *name;
	char global;
	struct s_content cnt;
	struct s_param_pair *next;
	};

static struct s_param_pair *param_first_free=0;

static struct s_param_pair *param_scope_stack[PARAM_SCOPE_STACK_SIZE];
static int param_level=1; /* per provare a risolvere il /inutile=maserve */
static int param_mem=0;

/*------------------------------------------------------------------------*/

static struct s_param_pair *new_param_pair()
{
struct s_param_pair *pair;
if(param_first_free)
  {
   pair=param_first_free;
   param_first_free=pair->next;
  }
else
  {
   pair = (struct s_param_pair*)malloc(sizeof(struct s_param_pair));
   param_mem += sizeof(struct s_param_pair);
  }
pair->global=0;
pair->next=0;
pair->name=0;
pair->cnt.tag=tag_none;
return pair;
}

/*------------------------------------------------------------------------*/

static free_param_pair(pair)
struct s_param_pair *pair;
{
pair->next=param_first_free;
pair->name=0;
pair->cnt.tag=tag_none;
param_first_free = pair;
}

/*------------------------------------------------------------------------*/

push_param_scope()
{
if(param_level>=PARAM_SCOPE_STACK_SIZE-1)
  zz_error(ERROR,"too many Zz variable scopes");
else
  {
   param_scope_stack[param_level++] = 0;
  }
}

/*------------------------------------------------------------------------*/

pop_param_scope()
{
struct s_param_pair *pair,*tmp;
if(param_level>0)
  {
   param_level--;
   pair = param_scope_stack[param_level];
   while(pair)
     {      
      tmp=pair;
      pair=pair->next;
      if(tmp->cnt.tag->param_off)
        (*tmp->cnt.tag->param_off)(&(tmp->cnt),tmp->name);
      free_param_pair(tmp);
     }
  }
}

/*------------------------------------------------------------------------*/
/*
  returns 1 if a new zz variable was crated, 0 otherwise
 */

int set_param(name,cnt)
     char *name;
     struct s_content *cnt;
{
  struct s_param_pair *pair;
  struct s_content *old;
  int i,ret;
  zz_assert(name);
  zz_trace("set_param(%s,%z)\n", name, cnt);

  if(strcmp(name,"$")==0) 
    return 1;

  if(param_level<=0) 
    push_param_scope();

  i    = param_level-1;
  pair = param_scope_stack[i];
  while(pair && pair->name!=name)pair=pair->next;
  if(pair)
    {
      if(pair->cnt.tag->param_off)
	(*pair->cnt.tag->param_off)(&(pair->cnt),pair->name);
      ret=0;
    }
  else
  {   
    pair = new_param_pair();
    pair->name = name;
    pair->next = param_scope_stack[i];
    param_scope_stack[i]=pair;
    ret=1;
  }
  if(cnt->tag->param_on)
    (*cnt->tag->param_on)(cnt,name);
  pair->cnt = *cnt;
  pair->global=0;
  return ret;
}

/*------------------------------------------------------------------------*/

gset_param(char *name, struct s_content *cnt)
{
  struct s_param_pair *pair;
  struct s_content *old;
  int i;
  if(strcmp(name,"$")==0) return 1;
  if(param_level<=0) push_param_scope();
  i=0;
  pair = param_scope_stack[i];
  while(pair && pair->name!=name)pair=pair->next;
  if(pair)
    {
      if(pair->cnt.tag->param_off)
	(*pair->cnt.tag->param_off)(&(pair->cnt),pair->name);
    }
  else
    {   
      pair = new_param_pair();
      pair->name = name;
      pair->next = param_scope_stack[i];
      param_scope_stack[i]=pair;
    }
  if(cnt->tag->param_on)
    (*cnt->tag->param_on)(cnt,name);
  pair->cnt = *cnt;
  pair->global=1;
  return 1;
}


/*------------------------------------------------------------------------*/

gnset_param(name,cnt,delta)
char *name;
struct s_content *cnt;
int delta;
{
struct s_param_pair *pair;
struct s_content *old;
int i;
if(strcmp(name,"$")==0) return 1;
if(param_level<=0) push_param_scope();
if(delta<0) 
  {zz_error(INTERNAL_ERROR,"gnset_param - bad delta (%d)",delta);return 0;}
if(param_level<=delta)i=0;
else i=param_level-delta-1;
pair = param_scope_stack[i];
// WARNING: linear scan
while(pair && pair->name!=name)pair=pair->next;
if(pair)
  {
   if(pair->cnt.tag->param_off)
     (*pair->cnt.tag->param_off)(&(pair->cnt),pair->name);
  }
else
  {   
   pair = new_param_pair();
   pair->name = name;
   pair->next = param_scope_stack[i];
   param_scope_stack[i]=pair;
  }
if(cnt->tag->param_on)
  (*cnt->tag->param_on)(cnt,name);
pair->cnt = *cnt;
pair->global=0;
return 1;
}


/*------------------------------------------------------------------------*/

unset_param(name)
char *name;
{
struct s_param_pair *pair,**ptr;
struct s_content *old;
int i;
if(strcmp(name,"$")==0) return 1;
i=param_level-1;
if(i<0) return;
ptr= &(param_scope_stack[i]);
while(*ptr && (*ptr)->name!=name) ptr= &((*ptr)->next);
if(*ptr)
  {
   pair= *ptr;
   *ptr = pair->next;
   if(pair->cnt.tag->param_off)
     (*pair->cnt.tag->param_off)(&(pair->cnt),pair->name);
   free_param_pair(pair);
  }
}


/*------------------------------------------------------------------------*/
/*
  scans param scopes downward, looking for a param
  whose name is token->value. 
  if found, token is overwritten, paramname is written

  returns 0 if unable to substitute (token!=ident or not found)
  returns 1 if local param found
  returns 2 if global param found
 */
param_substitute(token,paramname)
struct s_content *token;
char **paramname;
{
struct s_param_pair *pair;
struct s_content *cnt;
char *name;
int i;
*paramname=0;
if(token->tag!=tag_ident) return 0;
name = (char*)s_content_value(*token);
pair=0;
for(i=param_level-1;pair==0 && i>=0;i--)
  {
   pair = param_scope_stack[i];
   while(pair && pair->name!=name) pair=pair->next;
  }
if(pair)
  {
   *paramname = name;
   cnt = &(pair->cnt);
   *token = *cnt;
   return pair->global?2:1;
  }
else 
  return 0;
}

/*------------------------------------------------------------------------*/

local_param_substitute(token,paramname)
struct s_content *token;
struct s_content *paramname;
{
struct s_param_pair *pair;
struct s_content *cnt;
char *name;
int i;
paramname->tag = tag_none;
if(token->tag!=tag_ident) return;
name = (char*)s_content_value(*token);
if(param_level>0)
  {
   pair = param_scope_stack[param_level-1];
   while(pair && pair->name!=name) pair=pair->next;
   if(pair)
     {
      paramname->tag= tag_ident;
      /*paramname->value = (long) name;   era cosi'*/
      s_content_value(*paramname) = (long)name;   /*ep*/
      cnt = &(pair->cnt);
      *token = *cnt;
      return;
     }
  }
}

/*------------------------------------------------------------------------*/

s_param_filter(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
struct s_param_pair *pair;
int i,lev;
char *name;
name = (char*)s_content_value(argv[0]);
lev = 0;
pair=0;
for(i=param_level-1;pair==0 && i>=0;i--)
  {
   pair = param_scope_stack[i];
   while(pair && pair->name!=name) pair=pair->next;
  }
if(pair && !pair->global)
   *ret=pair->cnt;
else
   *ret=argv[0];
return 1;
}


/*------------------------------------------------------------------------*/


list_params()
{
struct s_param_pair *pair;
int i,lev;
lev=0;
if(param_level<=0)
  {
   printz("param stack is empty\n");
   return 1;
  }
for(i=0;i<param_level;i++)
  {
   pair = param_scope_stack[i];
   while(pair)
     {
      printz("%3d%c ",i,pair->global?'G':'L');
      printz("%-20.20s == %z\n",pair->name,&(pair->cnt));
      pair=pair->next;
     }
  }
return 1;
}

/*---------------------------------------------------------------------------*/

int zz_get_param_stack_depth()
{
  return param_level;
}

/*---------------------------------------------------------------------------*/

show_param_memory()
{
PRINTMEM("param",param_mem)
}



static char sccsid[]="@(#)param.c	6.2\t11/9/94";
static char rcsid[] = "$Id: param.c,v 1.11 2002/06/03 11:06:13 kibun Exp $ ";
