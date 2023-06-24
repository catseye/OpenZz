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

#include "zz.h"
#include "zlex.h"
#include "rule.h"
#include "err.h"
#include "avl.h"
#include "trace.h"

/*PROTOTYPES*/
int next_token(struct s_content *);
void syntax_error(int (*info_routine)());

/*---------------------------------------------------------------------------*/

extern struct s_nt *nt_any,*nt_param,*nt_gparam;
static char *first_prompt="";
static int max_dot=0,tot_dot=0,ndot=0,max_sp=0;
static int reduction_count=0;
#define EXPECTED_SIZE 30
static struct s_content expected[EXPECTED_SIZE];
static int expected_n;

/*---------------------------------------------------------------------------*/

/*   2 macros to search the AVL trees  */

#define AVL_LOCATE_TERM_TRAN(TR,VA,NO) {\
	struct avl_node *TMPnode=TR->root;\
	struct s_term_tran *TMPtran;NO=0;\
    	while (TMPnode) {\
    	  TMPtran = (struct s_term_tran*) TMPnode->key.ptr;\
      	  if      ( (long)TMPtran->term.tag < (long)(VA)->tag) \
		TMPnode = TMPnode->right;\
          else if ( (long)TMPtran->term.tag > (long)(VA)->tag) \
		TMPnode = TMPnode->left;\
          else {\
          if      ( s_content_value(TMPtran->term) < s_content_value(*(VA))) \
		TMPnode = TMPnode->right;\
	  else if ( s_content_value(TMPtran->term) > s_content_value(*(VA))) \
		TMPnode = TMPnode->left;\
	  else {NO=TMPnode->data;break;}}}}

#define AVL_LOCATE_LONG(TR,VA,NO) {\
	struct avl_node *node = TR->root;NO=0;\
    	while (node)\
      	  if      ( node->key.val < (long)VA) node = node->right;\
          else if ( node->key.val > (long)VA) node = node->left;\
          else {NO=node->data;break;}}        


/*----------------------------------------------------------------------*/

struct s_cur_token {
   struct s_content cnt;
   char *param_name,is_eof,is_param;
   struct s_nt *nt;
  } cur_token;

/*----------------------------------------------------------------------*/

#define LR_NextToken {\
   cur_token.is_eof= !next_token(&(cur_token.cnt));\
   cur_token.is_param=\
     param_substitute(&(cur_token.cnt),&(cur_token.param_name));\
   cur_token.nt=find_nt(cur_token.cnt.tag->name);\
  }

/*---------------------------------------------------------------------------*/
#define RECOVERY_SIZE 100
static struct {struct s_nt *nt; char *termlist;} recovery_array[RECOVERY_SIZE];
static int recovery_n=0;

/*---------------------------------------------------------------------------*/

#define WORKAREA_SIZE 100

#define A_REDUCE 1
#define A_SHIFT_KEYWORD 2
#define A_SHIFT_MATCHED_TAG 3
#define A_SHIFT_PARAM 4
#define A_SHIFT_ANY 5

#define NO_PARAM 0
#define PARAM 1
#define GPARAM 2


static struct {
  int type,set,is_param;
  struct s_content keyword;
  struct s_rule *rule;
 } workarea[WORKAREA_SIZE];
static int workarea_n=0;

/*---------------------------------------------------------------------------*/

/* storage for the dot */
#define DOT_POOL_SIZE 8000
static struct s_dot *dots[DOT_POOL_SIZE];
static int dots_n=0;

/* LR parser stack */
#define LRSTACK_SIZE 500
static struct {
   int a,b,prev;
  } lrstack[LRSTACK_SIZE];
struct s_content valuestack[LRSTACK_SIZE];

typedef struct {
 int sp,a,b;
 } LRENV;
static LRENV cur_lrenv = {0,0,-1};

/*----------------------------------------------------------------------*/

/*----------------------------------------------------------------------*/

/* Initialize the structure cur_lrenv (at the start of parse()) */
#define LR_InitStack {\
  cur_lrenv.a=cur_lrenv.b+1;}

/*----------------------------------------------------------------------*/

/* add a DOT to the working set */
#define LR_AddDot(DOT) {\
  if(cur_lrenv.b>=DOT_POOL_SIZE-1)\
    {zz_error(INTERNAL_ERROR,"dot_pool overflow");exit(1);}\
  else dots[++cur_lrenv.b]=DOT;}

/*----------------------------------------------------------------------*/

/* cancel all the dot(s) of the working set */
#define LR_ClearDots {\
  cur_lrenv.a=cur_lrenv.b-1;}

/*----------------------------------------------------------------------*/

/* mette il working set sullo stack (PREV e' il set precedente)
   apre il working set successivo */

#define LR_PushSet(PREV) {int sp=cur_lrenv.sp++;\
  if(cur_lrenv.sp>LRSTACK_SIZE)\
     {zz_error(INTERNAL_ERROR,"lrstack overflow");exit(1);}\
  else {\
   lrstack[sp].a=cur_lrenv.a;\
   lrstack[sp].b=cur_lrenv.b;\
   lrstack[sp].prev=(PREV);\
   cur_lrenv.a=cur_lrenv.b+1;}}

/*----------------------------------------------------------------------*/

/* 'SET' diventa il set sul top dello stack. il workset e' vuoto */
#define LR_RestoreSet(SET) { \
   cur_lrenv.sp = SET+1;cur_lrenv.a=lrstack[SET].b+1;\
   cur_lrenv.b=cur_lrenv.a-1;}

/*----------------------------------------------------------------------*/

static void compute_expected_from_set(int set_index);
/*------------------------------------------------=-------------------*/

static long int setid=0;

/* calcola la chiusura del work set */
make_closure()
{
  void lr_add_nt();
  int i,a=cur_lrenv.a,b=cur_lrenv.b;
  setid++;
  for(i=a;i<=b;i++)dots[i]->set = setid;
  for(i=a;i<=cur_lrenv.b;i++)
    avl_scan(dots[i]->nttree,lr_add_nt);
}

/*----------------------------------------------------------------------*/


void lr_add_nt(tran)
struct s_nt_tran *tran;
{
int i;
struct s_dot *dot;
if((dot = tran->nt->first_dot) && dot->set!=setid)
  {
/*
   for(i=cur_lrenv.a;i<=cur_lrenv.b;i++)
     if(dots[i]==dot) return ;
*/
   dot->set=setid;
   LR_AddDot(dot);
  }
}

/*----------------------------------------------------------------------*/



/* 
   crea il nuovo kernel (set senza chiuso)  con tutti gli shift
   che ammettono il token corrente.
   il set di partenza e' 'set_index' (vale sempre il tos, ma non
   e' necessario).
   set ci riesce aggiunge una voce a workarea e mette il nuovo set
   sullo stack. N.B. in questo caso si aggiorna lo stack pointer!!
   
 */

try_shift(set_index)
int set_index;
{
struct s_term_tran *ttran;
struct s_nt_tran *nttran;
int i,a,b,lev,count,zero,paramtype;
struct s_dot *dot;
a=lrstack[set_index].a;
b=lrstack[set_index].b;
lev=0;count=zero=cur_lrenv.b;
paramtype=PARAM;
for(i=a;i<=b;i++)
  {
   dot=dots[i];
   if(cur_token.is_param)
     {
      AVL_LOCATE_LONG(dot->nttree,nt_param,nttran)
      if(nttran) 
         {
	  if(lev<4) {lev=4;count=zero;} cur_lrenv.b=count++;
          LR_AddDot(nttran->next)
	  continue;
         }
      AVL_LOCATE_LONG(dot->nttree,nt_gparam,nttran)
      if(nttran) 
         {
	  if(lev<4) {lev=4;count=zero;} cur_lrenv.b=count++;
          LR_AddDot(nttran->next)
	  paramtype=GPARAM;
	  continue;
         }
     }
   AVL_LOCATE_TERM_TRAN(dot->termtree,&(cur_token.cnt),ttran)
   if(ttran) 
     {
      if(lev>3) continue;if(lev<3) {lev=3;count=zero;} cur_lrenv.b=count++;
      LR_AddDot(ttran->next)
     }
   AVL_LOCATE_LONG(dot->nttree,cur_token.nt,nttran)
   if(nttran) 
     {
      if(lev>2) continue;if(lev<2) {lev=2;count=zero;} cur_lrenv.b=count++;
      LR_AddDot(nttran->next)
     }
   if(!cur_token.is_eof)
     {
      AVL_LOCATE_LONG(dot->nttree,nt_any,nttran)
      if(nttran) 
        {
         if(lev>1)continue;if(lev<1){lev=1;count=zero;}cur_lrenv.b=count++;
         LR_AddDot(nttran->next)
	}
     }
  }
if(cur_lrenv.b>=cur_lrenv.a)
  {
   workarea[workarea_n].set = cur_lrenv.sp;
   workarea[workarea_n].rule=0;
   workarea[workarea_n].is_param = lev==4?paramtype:0;
   workarea_n++;
   LR_PushSet(set_index)
  }
}

/*----------------------------------------------------------------------*/

/* ritorna 1 se lo shift e' ammissibile */
check_shift(set_index)
int set_index;
{
struct s_term_tran *ttran;
struct s_nt_tran *nttran;
int i,a,b;
struct s_dot *dot;
a=lrstack[set_index].a;
b=lrstack[set_index].b;
for(i=a;i<=b;i++)
  {
   dot=dots[i];
   if(cur_token.is_param && dot->match_param) return 1;
   AVL_LOCATE_TERM_TRAN(dot->termtree,&(cur_token.cnt),ttran) 
     if(ttran) return 1;
   AVL_LOCATE_LONG(dot->nttree,cur_token.nt,nttran) if(nttran) return 1;
   if(!cur_token.is_eof && dot->match_any) return 1;
  }
return 0;
}


/*----------------------------------------------------------------------*/
/*

  crea (se possibile) un nuovo set (gia' chiuso) sul top dello stack riducendo
  da 'set_index' la regola 'rule'
  se ci riesce aggiunge una riga a workarea
  
  */

try_reduce(set_index,rule)
int set_index;
struct s_rule *rule;
{
LRENV oldenv;
int oldset,i,j,k,a,b,curset,ret,length,base;
struct s_nt *nt;
struct s_dot *dot,*dot1;
struct s_rule *rule1;
struct s_nt_tran *tran;

length=rule->bead_n-1;
nt=(struct s_nt *)s_content_value(rule->beads[0].cnt);
/* pop */
i=set_index;
while(i>=0 && length-->0) i=lrstack[i].prev;
if(i<0) {zz_error(INTERNAL_ERROR,"try_reduce: stack empty");exit(1);}
a=lrstack[i].a;
b=lrstack[i].b;
oldset=i; /* il set ''sotto' la regola */
/* creo il set GOTO */
cur_lrenv.b=cur_lrenv.a-1;
oldenv = cur_lrenv;
for(j=a;j<=b;j++)
  {
   dot1=dots[j];
   AVL_LOCATE_LONG(dot1->nttree,nt,tran)
   if(tran) LR_AddDot(tran->next)
  }
if(cur_lrenv.a>cur_lrenv.b)
  {
   zz_error(INTERNAL_ERROR,"try_reduce: GOTO not found reducing %r",rule);
   exit(1);
  }
make_closure();
LR_PushSet(oldset)
curset=cur_lrenv.sp-1;
ret=check_shift(curset);
if(!ret)
  {
   a=lrstack[curset].a;
   b=lrstack[curset].b;
   for(i=a;i<=b;i++)
     if((rule1=dots[i]->rule)!=NULL && check_reduce(curset,rule1)) 
       {ret=1;break;}
  }
if(ret)
  {
   workarea[workarea_n].rule=rule;
   workarea[workarea_n].set=curset;
   workarea[workarea_n].is_param = 0;
   workarea_n++;   
  }
else
  {
   /* la riduzione non e' ammissibile; come non detto */
   cur_lrenv = oldenv;
  }
return ret;
}


/*----------------------------------------------------------------------*/

check_reduce(set_index,rule)
int set_index;
struct s_rule *rule;
{
int oldset,i,j,k,a,b,curset,ret,length,base;
struct s_nt *nt;
struct s_dot *dot,*dot1;
struct s_rule *rule1;
struct s_nt_tran *tran;
LRENV oldenv;
oldenv = cur_lrenv;
length=rule->bead_n-1;
nt=(struct s_nt *)s_content_value(rule->beads[0].cnt);

i=set_index;
while(i>=0 && length-->0) i=lrstack[i].prev;
if(i<0) {zz_error(INTERNAL_ERROR,"check_reduce: stack empty");exit(1);}
a=lrstack[i].a;
b=lrstack[i].b;
oldset=i;
for(j=a;j<=b;j++)
  {
   dot1=dots[j];
   AVL_LOCATE_LONG(dot1->nttree,nt,tran)
   if(tran) LR_AddDot(tran->next)
  }
if(cur_lrenv.a>cur_lrenv.b)
  {
   zz_error(INTERNAL_ERROR,"try_reduce: GOTO not found reducing %r",rule);
   exit(0);
  }
make_closure();
LR_PushSet(oldset)
curset=cur_lrenv.sp-1;
ret=check_shift(curset);
if(!ret)
  {
   a=lrstack[curset].a;
   b=lrstack[curset].b;
   for(i=a;i<=b;i++)
     if((rule1=dots[i]->rule)!=NULL && check_reduce(curset,rule1)) 
       {ret=1;break;}
  }
cur_lrenv=oldenv;
return ret;
}


/*----------------------------------------------------------------------*/


lr_loop(main_nt)
     struct s_nt *main_nt;
{
  struct s_content ret_token,shift_token;
  int i,j,dst,a,b,newa,newb,olda,oldb,is_param;
  int cur_set,old_set,new_set;
  struct s_rule *rule;

  while(1)
    {
      workarea_n=0;
      cur_set = cur_lrenv.sp-1;
      a=lrstack[cur_set].a;
      b=lrstack[cur_set].b;

      /* lazy_rec(dots+a,b-a+1); */
   
      try_shift(cur_set);

      for(i=a;i<=b;i++)
	if(rule=dots[i]->rule)
	  {
	    if((struct s_nt *)s_content_pvalue(rule->beads[0].cnt)==main_nt)
	      return 1;
	    try_reduce(cur_set,rule);
	  }   

      if(workarea_n==0) 
	return 0;

      if(workarea_n>1)
	{
	  zz_error(ERROR,"Ambiguous syntax (%d)",workarea_n);

	  for(i=0;i<workarea_n;i++)
	    if(workarea[i].rule)
	      errprintf("  (%d) reduce %r\n",i,workarea[i].rule);
	    else
	      errprintf("  (%d) shift %z\n",i,&cur_token.cnt);

	  return -1;
	}

      if(workarea_n==1)
	{
	  new_set = workarea[0].set;
	  rule=workarea[0].rule;
	  is_param=workarea[0].is_param;

	  if(rule) 
	    lr_reduce(rule,cur_set,&ret_token);

	  newa = lrstack[new_set].a;
	  newb = lrstack[new_set].b;
	  old_set = lrstack[new_set].prev;
	  oldb = lrstack[old_set].b;
	  dst = newa-(oldb+1);

	  if(dst>0)
	    {
	      for(i=newa;i<=newb;i++)
		dots[i-dst]=dots[i];
	      newa-=dst;
	      newb-=dst;
	    }

	  cur_lrenv.a=newa;
	  cur_lrenv.b=newb;
	  cur_lrenv.sp=old_set+1;

	  if(rule==0) 
	    {
	      make_closure();
	      switch(is_param)
		{
		case PARAM:
		  valuestack[cur_lrenv.sp].tag = tag_ident;
		  s_content_svalue(valuestack[cur_lrenv.sp]) = cur_token.param_name;
		  break;
		case GPARAM:
		  if(cur_token.is_param==GPARAM)
		    {
		      valuestack[cur_lrenv.sp].tag = tag_ident;
		      s_content_svalue(valuestack[cur_lrenv.sp]) = cur_token.param_name;
		    }
		  else
		    valuestack[cur_lrenv.sp] = cur_token.cnt;	        
		  break;
		default:
		  valuestack[cur_lrenv.sp] = cur_token.cnt;
		}
	      shift_token = cur_token.cnt;

	      LR_NextToken;
	    }
	  else
	    {
	      valuestack[cur_lrenv.sp] = ret_token;
	    }

	  LR_PushSet(old_set)

	    if(zz_trace_mask()&TRACE_LRSTACK)
	      {
		if(rule)
		  {printz("  @ REDUCE %r\n",rule);dump_stack();}
		else
		  {printz("  @ SHIFT %z\n",&shift_token);dump_stack();}
	      }
	}
      else break;
    }
  return 0;
}


/*----------------------------------------------------------------------*/

lr_reduce(rule,set_index,ret_token)
struct s_rule *rule;
int set_index;
struct s_content *ret_token;
{
int i,length;
reduction_count++;
length = rule->bead_n-1;
set_index-=length-1;
if(zz_trace_mask()&TRACE_REDUCE)
  {
   printz("   @ reduce %r  args:",rule,length);
   for(i=0;i<length;i++)
     printz(" %z",&valuestack[set_index+i]);
   printz("\n");
  }
action(rule,valuestack+set_index,ret_token);
if(zz_trace_mask()&TRACE_REDUCE)
   printz("   @ action ret: %z\n",ret_token);
}



/*----------------------------------------------------------------------*/

static add_expected(tag,value)
struct s_tag *tag;
long value;
{
char *name,*s;
int i;
if(expected_n>=EXPECTED_SIZE) return 0;
if(tag==tag_sint)
  {
   name=((struct s_nt *)value)->name;
   for(s=name;*s && *s!='$';s++);
   if(*s=='$') return 1;
  }
else if(tag==tag_ident)
  {
   name=(char*)value;
   for(s=name;*s && *s!='$';s++);
   if(*s=='$') return 1;
  }
for(i=0;i<expected_n;i++)
  if(expected[i].tag==tag && s_content_value(expected[i])==value)
    return 1;
expected[expected_n].tag=tag;
s_content_value(expected[expected_n])=value;
expected_n++;
return 1;
}

/*----------------------------------------------------------------------*/

static compute_expected_from_reduction(set_index,rule)
int set_index;
struct s_rule *rule;
{
int i,j,k,a,b,curset;
LRENV oldenv;
struct s_dot *dot;
struct s_term_tran *ttran;
struct s_nt_tran *nttran;
int length;
struct s_nt *nt;
length=rule->bead_n-1;
nt=(struct s_nt*)s_content_value(rule->beads[0].cnt);
oldenv=cur_lrenv;
while(set_index>=0 && length-->0)
  set_index=lrstack[set_index].prev;
if(set_index<0) {printf("\n*** Internal error. stackempty ***\n");return 0;}
a=lrstack[set_index].a;
b=lrstack[set_index].b;
/* creo il set GOTO */
cur_lrenv.b=cur_lrenv.a-1;
for(i=a;i<=b;i++)
  {
   dot=dots[i];
   AVL_LOCATE_LONG(dot->nttree,nt,nttran)
   if(nttran) LR_AddDot(nttran->next)
  }
if(cur_lrenv.a>cur_lrenv.b)
  {
   printf("\n*** Internal error. GOTO not found ***\n");
   cur_lrenv = oldenv;return 0;
  }
make_closure();
LR_PushSet(set_index)
compute_expected_from_set(cur_lrenv.sp-1);
cur_lrenv = oldenv;
}

/*----------------------------------------------------------------------*/


static void compute_expected_from_set(int set_index)
{
struct s_dot *dot;
int i,a,b;
struct s_term_tran *ttran;
struct s_nt_tran *nttran;
a=lrstack[set_index].a;
b=lrstack[set_index].b;
for(i=a;i<=b;i++)
  {
   dot=dots[i];
   for(ttran=avl_first(dot->termtree);ttran;ttran=avl_next(dot->termtree))
     if(!add_expected(ttran->term.tag,s_content_value(ttran->term))) return;
   for(nttran=avl_first(dot->nttree);nttran;nttran=avl_next(dot->nttree))
     if(!add_expected(tag_sint,nttran->nt)) return;
  }
for(i=a;i<=b;i++)
  {
   dot=dots[i];
   if(dot->rule)
     compute_expected_from_reduction(set_index,dot->rule);
  }
}

/*----------------------------------------------------------------------*/

static void print_expected()
{
char buffer[256];
int i,j,k;
expected_n=0;
compute_expected_from_set(cur_lrenv.sp-1);
if(expected_n==0)
  errprintf("| no token accessible here\n");
else
  {
   sprintz(buffer,"got: '%z'\n| ", &(cur_token.cnt)); 
   strcat(buffer,"expected one of: ");
   j=strlen(buffer);
   for(i=0;i<expected_n;i++)
     {
      buffer[j++]=' ';
      if(expected[i].tag==tag_sint)
        strcpy(buffer+j,((struct s_nt*)s_content_value(expected[i]))->name);
      else
        sprintz(buffer+j,"'%z'",expected+i);
      if(i>=EXPECTED_SIZE-1) 
	strcat(buffer+j," ....");
      k=j;
      while(buffer[j]) j++;
      if(j>70) {
        buffer[k]='\0';
        errprintf("| %s\n",buffer);
        i--;strcpy(buffer,"   ");j=strlen(buffer);
       }
     }
   
   if(j>0) errprintf("| %s\n",buffer);
  }
}

/*----------------------------------------------------------------------*/

recovery()
{
struct {struct s_content cnt; struct s_nt *nt;int lrset;} 
  tmpterm,termlist[1000];
int a,b,tmp,termlist_n,found;
struct s_nt_tran *nttran;
struct s_nt *nt;
struct s_dot *dot;
struct s_content cnt;
char *s;
int pop,i,j,k,newpop;
int set_index;

/* guardo i set sullo stack e la lista delle coppie di
   recovery. Mi costruisco l'insieme dei terminali su cui si puo'
   fare recovery */

set_index = cur_lrenv.sp-1;
termlist_n=0;
while(set_index>=0)
  {
   a=lrstack[set_index].a;
   b=lrstack[set_index].b;
   for(i=a;i<=b;i++)
     {
      dot=dots[i];
      nttran=0;
      for(k=0;nttran==0 && k<recovery_n;k++)
        {
         nt = recovery_array[k].nt;
         AVL_LOCATE_LONG(dot->nttree,nt,nttran)
         if(nttran) 
	   {
             s=recovery_array[k].termlist;
             while(*s)
               {
		 zlex(&s,&cnt);
		 termlist[termlist_n].cnt = cnt;
		 termlist[termlist_n].nt = nt;
		 termlist[termlist_n].lrset = set_index;
		 termlist_n++;
	       }
	   }
	}
     }
   set_index=lrstack[set_index].prev;   
  }

for(i=termlist_n-1;i>0;i--)
  for(j=0;j<i;j++)
    if(termlist[j].lrset<termlist[j+1].lrset)
       {
	tmpterm=termlist[j];termlist[j]=termlist[j+1];termlist[j+1]=tmpterm;
       }

/*
printf("recovery term:\n");
for(k=0;k<termlist_n;k++)
  {
   printz("%z on nt %s (setstack %d)\n",
      &(termlist[k].cnt),
      termlist[k].nt->name,
      termlist[k].lrset);
  }
*/

/* scandisco il file finche' non trovo un terminale di recovery
   (o un EOF) */
found=0;
while(!cur_token.is_eof)
  {
   for(i=0;i<termlist_n;i++)
     if(cur_token.cnt.tag==termlist[i].cnt.tag &&
        s_content_value(cur_token.cnt)==s_content_value(termlist[i].cnt))
        break;
   if(i<termlist_n) {found=1;break;}
   LR_NextToken;
  }

/* se ho trovato un terminale ammissibile: */
if(found)
  {
   /* svuoto lo stack fino al nt corrispondente */
   j = termlist[i].lrset;
   nt = termlist[i].nt;
   LR_RestoreSet(j)
   a=lrstack[j].a;
   b=lrstack[j].b;
   for(i=a;i<=b;i++)
     {
      dot=dots[i];
      for(nttran=avl_first(dot->nttree);nttran;nttran=avl_next(dot->nttree))
	LR_AddDot(nttran->next)
     }
   make_closure();
   LR_PushSet(j)   
   return 1;
  }
else
  return 0;
}

/*------------------------------------------------------------------*/

/*

  Associa una lista di termini di recovery ad un non-terminale

*/
set_recovery(ntname,termlist)
char *ntname,*termlist;
{
struct s_nt *nt;
int i;
nt = find_nt(ntname);
for(i=0;i<recovery_n && recovery_array[i].nt!=nt;i++);
if(i>=recovery_n)
  {
   if(recovery_n>=RECOVERY_SIZE)
     {
      printf("set_recovery: too many recovery pairs\n");return;
     }
   i=recovery_n++;
   recovery_array[i].nt=nt;
  }
else
  {
   if(recovery_array[i].termlist) 
     free(recovery_array[i].termlist);
  }
recovery_array[i].termlist = (char*)malloc(strlen(termlist)+1);
strcpy(recovery_array[i].termlist,termlist);
}

/*------------------------------------------------------------------*/

static int find_prompt(prompt)
     const char **prompt;
{
struct s_nt_tran *nttran;
struct s_dot *dot;
struct s_nt *nt;
int i,set;
set=cur_lrenv.sp-1;
if(lrstack[set].prev<0)
  {
/*
   ep & gmt -- 9/11/94
   *prompt=first_prompt;
*/
    *prompt = zz_get_prompt();
    return 1;
  }
for(i=cur_lrenv.a;i<=cur_lrenv.b;i++)
  {
   dot = dots[i];
   if(dot->rule)
     {
      nt=(struct s_nt*)s_content_value(dot->rule->beads[0].cnt);
      if(nt->prompt)
        {
         *prompt=nt->prompt;
         return 1;
        }
     }
  }
return 0;
}


/*------------------------------------------------------------------*/
/*

  associa un prompt ad un non terminale

*/

set_nt_prompt(ntname,prompt)
char *ntname,*prompt;
{
struct s_nt *nt;
if(ntname)
  {
   nt = find_nt(ntname);
   nt->prompt=prompt;
  }
else
  first_prompt = prompt;
}


/*----------------------------------------------------------------------*/


dump_dot(dot,off)
struct s_dot *dot;
int off;
{
int length;
struct s_term_tran *ttran;
struct s_nt_tran *nttran;
struct s_rule *rule;
rule=dot->rule;
if(rule)
  {
   length=rule->bead_n-1;
   printz("  (%d) %r\n",length-off,rule);
  }
for(nttran=avl_first(dot->nttree);nttran;nttran=avl_next(dot->nttree))
   dump_dot(nttran->next,off+1);
for(ttran=avl_first(dot->termtree);ttran;ttran=avl_next(dot->termtree))
   dump_dot(ttran->next,off+1);
}



/*----------------------------------------------------------------------*/

dump_stack()
{
#define IMAGE_SIZE 10
int image[IMAGE_SIZE],image_n;
struct s_dot *dot;
int i,j,a,b;
i = cur_lrenv.sp-1;
image_n=0;
while(i>=0 && image_n<IMAGE_SIZE)
  {
   image[image_n++]=i;
   i=lrstack[i].prev;
  }
printf("  @ lrstack[]= %s",image_n>=IMAGE_SIZE?" ...":"   |");
while(--image_n>=0)
  {
   a=lrstack[image[image_n]].a;
   b=lrstack[image[image_n]].b;
   for(j=a;j<=b;j++) {printf("%s%d ",dots[j]->rule?"'":"",dots[j]->id);}
   printf("| ");
  }
printf("\n");
}



/*----------------------------------------------------------------------*/

dump_set(set_index)
int set_index;
{
int i,a,b;
a=lrstack[set_index].a;
b=lrstack[set_index].b;
printf("set %d (prev=%d)\n",set_index,lrstack[set_index].prev);
for(i=a;i<=b;i++)
  {
   if(i>a) printf("  ----\n");
   dump_dot(dots[i],0);
  }
printf("\n");
}

/*----------------------------------------------------------------------*/

write_dot_stat()
{
printf("dot n.: max=%d, mean=%d\n",max_dot,ndot==0?0:tot_dot/ndot);
printf("lr sp: max=%d\n",max_sp);
}

/*------------------------------------------------------------------*/

print_report()
{
static int old_reduction_count=0;
printf("%d reductions done (%+d)\n",
       reduction_count,reduction_count-old_reduction_count);
old_reduction_count=reduction_count;
printf("%d dots used\n",cur_lrenv.b); 
}

/*------------------------------------------------------------------*/

void fprint_param(chan)
FILE *chan;
{
if(cur_token.is_param)
  {
   fprintz(chan,"| %s == %z\n",
     cur_token.param_name,&cur_token.cnt);
  }
}



/*------------------------------------------------------------------*/

int parse(struct s_nt *start_nt)
{
  int ret;
  struct s_cur_token old_token;
  LRENV old_lrenv;
  //extern int print_expected();
  extern int (*find_prompt_proc)();

  find_prompt_proc=find_prompt;

  /* Save the current configuration */
  old_lrenv = cur_lrenv;
  old_token = cur_token;

  /* Initialize the stack */
  LR_InitStack;
  LR_AddDot(start_nt->first_dot);
  make_closure();
  LR_PushSet(-1);
  LR_NextToken;

  while(1)
    {
      ret=lr_loop(start_nt);
      if(ret>0) break;
      if(ret==0) syntax_error(print_expected);
      if(!recovery())
        {
          zz_error(FATAL_ERROR,"unrecoverable error");
          break;
        }
    }

  cur_lrenv = old_lrenv;
  cur_token = old_token;

  if(get_error_number())
    return 0;
  else 
    return 1;
}


static char sccsid[]="@(#)parse.c	6.2\t11/9/94";
static char rcsid[] = "$Id: parse.c,v 1.12 2002/01/24 18:28:28 ckjamesb Exp $ ";
