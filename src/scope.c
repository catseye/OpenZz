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

#define DEBUG 1

#include <stdio.h>
#include <stdint.h>
#include "zz.h"
#include "avl.h"
#include "rule.h"
#include "list.h"
#include "err.h"
#include "trace.h"

struct s_scope {
        char enabled;
	char *name;
	TREE *rules;
        struct s_scope *previous,*next;
	};
	
	
static struct s_scope *top_scope=0,*cur_scope=0;
static TREE *scope_tree=0;
static int link_flag=0;
extern int kernel_flag;
static struct s_nt *cur_nt;
int list_kernel_flag=0;
static int cur_segment_id=1;


/*---------------------------------------------------------------------------*/


extern int link_rule(),unlink_rule();
static struct s_rule *last_rule=0;
// Sap: int push_rule(),pop_rule();
void push_rule(),pop_rule();

/*---------------------------------------------------------------------------*/

struct s_scope *find_scope(name)
char *name;
{
int rulecmp();
struct s_scope *scope;
if(!scope_tree)
  scope_tree = avl_tree_nodup_str(struct s_scope,name);
scope = avl_locate(scope_tree,name);
if(!scope)
  {
   scope = (struct s_scope *)calloc(1,sizeof(struct s_scope));
   scope->name = name;
   scope->rules = avl_tree_nodup(rulecmp);
   scope->enabled=0;
   scope->previous=scope->next=0;
   if(zz_trace_mask()&TRACE_SCOPE) printf("   @ create scope %s\n",name);
   avl_insert(scope_tree,scope);
  }
return scope;
}

/*---------------------------------------------------------------------------*/

zz_push_scope(scope_name)
char *scope_name;
{
struct s_scope *scope,*new_scope;
new_scope = find_scope(scope_name);
scope = top_scope;
while(scope && scope!=new_scope) scope=scope->previous;
if(scope) {zz_error(ERROR,"duplicate scope");return;}
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ push scope %s\n",scope_name);
if(top_scope) top_scope->next = new_scope;
new_scope->previous = top_scope;
new_scope->next = 0;
top_scope = new_scope;
avl_scan(top_scope->rules,push_rule);
top_scope->enabled=1;
}

/*---------------------------------------------------------------------------*/
delete_and_push_scope(scope_name)
char *scope_name;
{
delete_scope(scope_name);
zz_push_scope(scope_name); 
}

/*---------------------------------------------------------------------------*/

zz_pop_scope()
{
struct s_scope *scope;
if(!top_scope || !top_scope->previous)
  {zz_error(ERROR,"you can't remove the kernel scope");return;}
scope = top_scope;
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ pop scope %s\n",scope->name);
top_scope=top_scope->previous;
top_scope->next = 0;
scope->next=scope->previous=0;
avl_scan(scope->rules,pop_rule);
scope->enabled=0;
}


/*---------------------------------------------------------------------------*/

/* una regola piazzata al top scope oscura le altre 
   eventualmente viene chiamato link_rule
   viene chiamato da zz_push_scope() */
// Sap: push_rule(rule)
void push_rule(rule)
struct s_rule *rule;
{
struct s_rule *oldrule;
struct s_scope *scope;
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ push rule %r\n",rule);
scope = top_scope->previous;
while(scope)
  {
   oldrule = avl_locate(scope->rules,rule);
   if(oldrule) break;
   scope=scope->previous;
  }
rule->prev_rule = oldrule;
rule->next_rule = 0;
if(oldrule)
  {
   if(zz_trace_mask()&TRACE_SCOPE) 
     printz("   @ push rule: obscurated %s::%r\n",scope->name,oldrule);
   *(oldrule->table_backptr)=rule;
   rule->table_backptr = oldrule->table_backptr;
   oldrule->table_backptr = 0;
   oldrule->next_rule = rule;
  }
else
  {
   if(zz_trace_mask()&TRACE_SCOPE) printz("   @ link %r\n",rule);
   link_rule(rule);
  }
}

/*---------------------------------------------------------------------------*/

/*
  una regola tolta dal top scope scopre le altre
  eventualmente viene chiamato unlink_rule()
*/
// Sap: pop_rule(rule)
void pop_rule(rule)
struct s_rule *rule;
{
struct s_rule *oldrule,*r;
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ pop rule %r\n",rule);
if(rule->next_rule)
  {
   zz_error(INTERNAL_ERROR,"pop_rule: not top_scope rule");
  }
oldrule=rule->prev_rule;
*(rule->table_backptr)=oldrule;
if(oldrule)
  {
   oldrule->table_backptr = rule->table_backptr;
   oldrule->next_rule = 0;
  }
else
  {
   if(zz_trace_mask()&TRACE_SCOPE) printz("   @ unlink %r\n",rule);
   unlink_rule(rule);
  }
rule->table_backptr = 0;
rule->next_rule=rule->prev_rule=0;
}


/*---------------------------------------------------------------------------*/

/* come pop_rule, ma funziona anche per regole non sul top dello stack */

// Sap: remove_rule(rule)
void remove_rule(rule)
struct s_rule *rule;
{
struct s_rule *oldrule,*r;
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ remove rule %r\n",rule);
if(rule->next_rule)
  {
   rule->next_rule->prev_rule = rule->prev_rule;
   if(rule->prev_rule) rule->prev_rule->next_rule = rule->next_rule;
   rule->table_backptr = 0;
   rule->next_rule=rule->prev_rule=0;
  }
else
  pop_rule(rule);
}

/*---------------------------------------------------------------------------*/

insert_rule(scope_name,rule)
char *scope_name;
struct s_rule *rule;
{
struct s_scope *scope,*dst_scope;
struct s_rule *oldrule,*r;
int flag,need_link;
last_rule = rule;
if(kernel_flag==0)
  rule->segment_id = cur_segment_id;
if(scope_name)
  dst_scope = find_scope(scope_name);
else 
  {if(!top_scope) zz_push_scope("kernel");
   dst_scope = top_scope;}

oldrule = avl_locate(dst_scope->rules,rule);
if(oldrule)
  {
   /* La regola e' presente nello stesso scope: chomp */
    if((zz_trace_mask()&TRACE_SCOPE) && (!kernel_flag))
      printz("   @ scope %s: overwrite rule %r\n",dst_scope->name,oldrule);

   if(oldrule->when_change_action.tag==tag_list)
     {
      source_list(&oldrule->when_change_action);  
      parse(find_nt("root"));
      pop_source();
     }
   if(oldrule->beads) free(oldrule->beads);
   if(oldrule->action.tag==tag_list)
     delete_list(&oldrule->action);

   rule->prev_rule = oldrule->prev_rule;
   rule->next_rule = oldrule->next_rule;
   rule->table_backptr = oldrule->table_backptr;
   *oldrule = *rule;
   last_rule = oldrule;
   
   rule->beads=0;
   rule->action.tag=tag_none;
   free(rule);
  }
else
  {
   need_link = 1;
   /* la regola non e' presente in dst_scope */
   avl_insert(dst_scope->rules,rule);
   scope = dst_scope->next;
   while(scope)
      {
       oldrule = avl_locate(scope->rules,rule);
       if(oldrule)break;
       scope=scope->next;
      }
   if(oldrule)
     {
      /* la regola e' oscurata da oldrule*/
      need_link = 0;
      rule->table_backptr = 0;
      rule->next_rule = oldrule;
      rule->prev_rule = oldrule->prev_rule;
      oldrule->prev_rule = rule;
      if(rule->prev_rule) rule->prev_rule->next_rule = rule;
     }
   else
     {
      /* la regola e' il nuovo top_scope */
      scope = dst_scope->previous;
      flag=1;
      while(scope)
        {
	 oldrule = avl_locate(scope->rules,rule);
	 if(oldrule) break;
	 scope=scope->previous;
        }
      if(oldrule)
        {
	 /* rule oscura oldrule */
	 rule->table_backptr=oldrule->table_backptr;
	 *(rule->table_backptr) = rule;
	 oldrule->table_backptr = 0;
	 oldrule->next_rule = rule;
	 rule->next_rule = 0;
	 rule->prev_rule = oldrule;
	}
      else
	{
	 /* rule e' nuova */
	 if(dst_scope->enabled)
	   link_rule(rule);
	}
     }
  }
}

/*---------------------------------------------------------------------------*/

delete_scope(name)
char *name;
{
int i,j;
/*void free_rule();*/
struct s_scope *scope,*tmpscope;
if(!scope_tree)
  {
/*   zz_error(ERROR,"delete scope: scope tree is empty"); */
   return;
  }
if(strcmp(name,"kernel")==0)
  {zz_error(ERROR,"you can't remove the kernel scope");return;}
scope = avl_remove(scope_tree,name); /*******/
if(!scope)
  {/*zz_error(ERROR,"scope %s not found",name);*/ return;}
if(zz_trace_mask()&TRACE_SCOPE) printz("   @ delete scope %s\n",scope->name);

if(scope->next || scope->previous)
  {
   if(scope==top_scope) 
     {
      if(!scope->previous)
         {zz_error(ERROR,"you can't remove the last scope");return;}
      top_scope = scope->previous;
     }
   if(scope->next) scope->next->previous = scope->previous;
   if(scope->previous) scope->previous->next = scope->next;
   avl_scan(scope->rules,remove_rule);
   avl_release(scope->rules,free_rule);
   avl_close(scope->rules);
   free(scope);      
  }
else /* scope non sullo stack */
  {
   avl_release(scope->rules,free_rule);
   avl_close(scope->rules);
   free(scope);   
  }
}

/*---------------------------------------------------------------------------*/

struct s_rule *get_last_rule()
{
return last_rule;
}

/*---------------------------------------------------------------------------*/

// Sap: do_list_rule(rule)
void do_list_rule(rule)
struct s_rule *rule;
{
struct s_nt *nt;
if(cur_nt)
  {
   nt = (struct s_nt*) s_content_value(rule->beads[0].cnt);
   if(nt!=cur_nt) return;
  }
if(rule->kernel && list_kernel_flag==0) return;
printz("  %r\n",rule);
}

/*---------------------------------------------------------------------------*/


list_all_rules() {do_list_rules(0,0);}
list_all_krules() {do_list_rules(0,1);}
list_rules(s)char*s; {do_list_rules(s,0);}
list_krules(s)char*s; {do_list_rules(s,1);}

/*---------------------------------------------------------------------------*/

do_list_rules(sintname,kflag)
char *sintname;
int kflag;
{
struct s_scope *scope;
int i;
list_kernel_flag=kflag;
if(sintname)
  cur_nt = find_nt(sintname);
else
  cur_nt = 0;
printf("RULES");
if(kflag) printf("+KRULES");
if(sintname) printf(" of sintagma %s",sintname);
printf("\n");
scope = top_scope;
while(scope)
  {
   printf(" Scope %s \n",scope->name);
   if(scope->rules)
     avl_scan(scope->rules,do_list_rule); 
   printf("\n");
   scope=scope->previous;
  }
printf("\n");
}

/*---------------------------------------------------------------------------*/
static FILE *Uchan=0;


/*---------------------------------------------------------------------------*/

// Sap: do_write_rule(rule)
void do_write_rule(rule)
struct s_rule *rule;
{
if(rule->segment_id!=cur_segment_id) return;
if(Uchan)
  fprintz(Uchan,"/%r %w\n\n",rule,rule->action);
}

/*---------------------------------------------------------------------------*/


write_rules(filename)
char *filename;
{
struct s_scope *scope;
int i;
Uchan = fopen(filename,"a");
if(!Uchan) {zz_error(ERROR,"Unable to write %s\n",filename);return;}
printf("RULES segment %d -> (%s)\n",cur_segment_id,filename);
scope = top_scope;
while(scope)
  {
   fprintf(Uchan,"!! Scope %s \n",scope->name);
   if(scope->rules)
     avl_scan(scope->rules,do_write_rule); 
   fprintf(Uchan,"\n");
   scope=scope->previous;
  }
fprintf(Uchan,"\n");
fclose(Uchan);
Uchan=0;
cur_segment_id++;
}

/*---------------------------------------------------------------------------*/


static char rcsid[] = "$Id: scope.c,v 1.9 2002/06/03 11:06:13 kibun Exp $ ";
