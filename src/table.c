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

	file: TABLE.C

	dot = create_dot(nt);
	remove_dot(dot);

	dot = [find|insert|add|sub]_[term|nt]_tran(dot,[term|nt]);

	link_rule(rule)
	unlink_rule(rule)


==============================================================================*/
#include <stdint.h>
#include "zlex.h"
#include "rule.h"
#include "err.h"
#include "mem.h"
#include "table.h"

static int table_mem=0;
extern struct s_nt *nt_any,*nt_param,*nt_gparam;

/*----------------------------------------------------------------------------*/


/* CONFRONTO DI T-TRAN */

ttrancmp(p1,p2)
struct s_term_tran *p1,*p2;
{
struct s_content *t1,*t2;
t1 = &(p1->term);
t2 = &(p2->term);
if(t1->tag<t2->tag) return -1;
else if(t1->tag>t2->tag) return 1;
else
  {
   if(s_content_value(*t1)<s_content_value(*t2)) return -1;
   else if(s_content_value(*t1)>s_content_value(*t2)) return 1;
   else return 0;
  }
}




/*----------------------------------------------------------------------------*/

struct s_dot *create_dot(nt)
struct s_nt *nt;
{
static int dot_count=0;
struct s_dot *dot;
dot = (struct s_dot *)calloc(1,sizeof(struct s_dot));
table_mem+=sizeof(struct s_dot);
dot->nt = nt;
dot->id = dot_count++;
dot->termtree = avl_tree_nodup(ttrancmp);
dot->nttree = avl_tree_nodup_long(struct s_nt_tran,nt);
dot->match_param=dot->match_any=0;
return dot;
}


/*----------------------------------------------------------------------------*/

/* 
  chiamata da avl_release in remove_dot
 */
void remove_nt_tran(tran)
struct s_nt_tran *tran;
{
remove_dot(tran->next);
free(tran);
}

/*----------------------------------------------------------------------------*/


/* 
  chiamata da avl_release in remove_dot
 */
void remove_term_tran(tran)
struct s_term_tran *tran;
{
remove_dot(tran->next);
free(tran);
}

/*----------------------------------------------------------------------------*/

remove_dot(dot)
struct s_dot *dot;
{
if(!dot)return;
avl_release(dot->termtree,remove_term_tran);
avl_release(dot->nttree,remove_nt_tran);
avl_close(dot->termtree);dot->termtree=0;
avl_close(dot->nttree);dot->nttree=0;
free(dot);
}

/*----------------------------------------------------------------------------*/


struct s_dot *find_term_tran(dot,token)
struct s_dot *dot;
struct s_content *token;
{
struct s_term_tran *tran,key;
key.term = *token;
tran = avl_locate(dot->termtree,&key);
if(!tran) return 0;
else 
  return tran->next;
}



/*----------------------------------------------------------------------------*/

struct s_dot *insert_term_tran(dot,token)
struct s_dot *dot;
struct s_content *token;
{
struct s_term_tran *tran;
tran = (struct s_term_tran *)calloc(1,sizeof(struct s_term_tran));
table_mem+=sizeof(struct s_term_tran);
tran->term = *token;
tran->next = create_dot(0);
tran->count = 1;
avl_insert(dot->termtree,tran);
return tran->next;
}

/*----------------------------------------------------------------------------*/

struct s_dot *add_term_tran(dot,token)
struct s_dot *dot;
struct s_content *token;
{
struct s_term_tran *tran,key;
key.term = *token;
tran = avl_locate(dot->termtree,&key);
if(tran)
  {
   tran->count++;
   return tran->next;
  }
else
  return insert_term_tran(dot,token);
}

/*----------------------------------------------------------------------------*/

static void check_dummy_dot(dot,sid)
struct s_dot *dot;
char *sid;
{
int n;
n = avl_nodes(dot->nttree) + avl_nodes(dot->termtree) + (dot->rule?1:0);
if(n==0)
  {
   if(!dot->nt)
     zz_error(INTERNAL_ERROR,"%s: dot%d  is empty",sid,dot->id);
  }
}

/*----------------------------------------------------------------------------*/

static struct s_dot *sub_term_tran(dot,token)
struct s_dot *dot;
struct s_content *token;
{
struct s_term_tran *tran,key;
key.term = *token;
tran = avl_locate(dot->termtree,&key);
if(!tran)
  {
   zz_error(INTERNAL_ERROR,"sub_term_tran: tran not found");return 0;
  }
if(--tran->count) return tran->next;
remove_dot(tran->next);
avl_remove(dot->termtree,&key);
check_dummy_dot(dot,"sub_term_tran");
return 0;
}


/*----------------------------------------------------------------------------*/


struct s_dot *find_nt_tran(dot,nt)
struct s_dot *dot;
struct s_nt *nt;
{
struct s_nt_tran *tran;
tran = avl_locate(dot->nttree,nt);
if(!tran) return 0;
else 
  return tran->next;
}

/*----------------------------------------------------------------------------*/

struct s_dot *insert_nt_tran(dot,nt)
struct s_dot *dot;
struct s_nt *nt;
{
struct s_nt_tran *tran;
tran = (struct s_nt_tran *)calloc(1,sizeof(struct s_nt_tran));
table_mem+=sizeof(struct s_nt_tran);
tran->nt = nt;
tran->next = create_dot(0);
tran->count = 1;
avl_insert(dot->nttree,tran);
if(nt==nt_any) dot->match_any=1;
else if(nt==nt_param || nt==nt_gparam) dot->match_param=1;
return tran->next;
}


/*----------------------------------------------------------------------------*/

struct s_dot *add_nt_tran(dot,nt)
struct s_dot *dot;
struct s_nt *nt;
{
struct s_nt_tran *tran;
tran = avl_locate(dot->nttree,nt);
if(tran)
  {
   tran->count++;
   return tran->next;
  }
else
  return insert_nt_tran(dot,nt);
}


/*----------------------------------------------------------------------------*/

static struct s_dot *sub_nt_tran(dot,nt)
struct s_dot *dot;
struct s_nt *nt;
{
struct s_nt_tran *tran;
tran = avl_locate(dot->nttree,nt);
if(!tran)
  {
   zz_error(INTERNAL_ERROR,"sub_nt_tran: tran not found");return 0;
  }
if(--tran->count) return tran->next;
remove_dot(tran->next);
avl_remove(dot->nttree,nt);
if(nt==nt_any) dot->match_any=0;
else if(nt==nt_param || nt==nt_gparam) dot->match_param=0;
check_dummy_dot(dot,"sub_nt_tran");
return 0;
}


/*---------------------------------------------------------------------------*/

link_rule(rule)
struct s_rule *rule;
{
int bead_n;
struct s_bead *bead;
struct s_nt *nt;
struct s_dot *dot,*next;
bead = rule->beads;
bead_n = rule->bead_n;
nt = (struct s_nt *) s_content_value(bead->cnt);
bead++;
bead_n--;
if(!nt->first_dot)
  nt->first_dot = create_dot(nt);
dot = nt->first_dot;
while(bead_n--)
  {
   if(bead->cnt.tag==tag_sint)
      dot = add_nt_tran(dot,s_content_value(bead->cnt));
   else
      dot = add_term_tran(dot,&(bead->cnt));
   bead++;
  }
if(dot->rule)
  {
   zz_error(INTERNAL_ERROR,"link_rule: overwriting rule ");
  }
dot->rule = rule;
dot->rule_count++;
rule->table_backptr = &(dot->rule);
}


/*----------------------------------------------------------------------------*/

unlink_rule(rule)
struct s_rule *rule;
{
int bead_n;
struct s_bead *bead;
struct s_nt *nt;
struct s_dot *dot,*next;
bead = rule->beads;
bead_n = rule->bead_n;
nt = (struct s_nt *) s_content_pvalue(bead->cnt);
bead++;
bead_n--;
if(!nt->first_dot)
  {
   zz_error(INTERNAL_ERROR,"unlink_rule: first dot not found");return 0;
  }
dot = nt->first_dot;
while(dot && bead_n--)
  {
   if(bead->cnt.tag==tag_sint)
      dot = sub_nt_tran(dot,s_content_value(bead->cnt));
   else
      dot = sub_term_tran(dot,&(bead->cnt));
   bead++;
  }
if(dot)
  {
   if(dot->rule==rule)
     {
      dot->rule = 0;
      check_dummy_dot(dot,"unlink_rule");
     }
   else if(dot->rule != 0) /* work around for popvoidrule bug */
     {                     /* why can dot->rule be null ??? */
      zz_error(INTERNAL_ERROR,"unlink_rule: rule mismatch");
      printf("|  old rule is (0x%x)",dot->rule);print_rule(dot->rule);//printf(" (%p)\n", dot->rule);
      printf("|  unlinking rule is ");print_rule(rule);printf("\n");      
      abort();
     }
  }
}


/*----------------------------------------------------------------------------*/

show_table_mem()
{
PRINTMEM("table",table_mem)
}
static char sccsid[]="@(#)table.c	6.1\t9/7/94";
static char rcsid[] = "$Id: table.c,v 1.6 2002/03/28 16:09:24 kibun Exp $ ";
