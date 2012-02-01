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
#include "avl.h"
#include "err.h"

static struct s_dot *hd_dot=0;

#define DOT_LIST_SIZE 1000
static struct s_dot *dot_list[DOT_LIST_SIZE];
static int dot_n=0;

#define NT_LIST_SIZE 1000
static struct s_nt *nt_list[NT_LIST_SIZE];
static int nt_n=0;

/*----------------------------------------------------------------------------*/

static dump_tran(string)
char *string;
{
int i,tab;
char buffer[256];
tab = 10;
if(hd_dot)
  {
   sprintf(buffer,"dot%d",hd_dot->id);
   if(hd_dot->nt)
     {
      strcat(buffer,"(");
      strcat(buffer,hd_dot->nt->name);
      strcat(buffer,")");      
     }
   for(i=0;buffer[i];i++);
   if(i<tab)
     for(;i<tab;i++)buffer[i]=' ';
   else
     buffer[i++]=' ';
   buffer[i]='\0';
   hd_dot=0;
  }
else
  {
   for(i=0;i<tab;i++) buffer[i]=' ';
   buffer[tab]='\0';
  }
strcat(buffer,string);
printf("%s\n",buffer);
}

/*----------------------------------------------------------------------------*/

static void dump_add_dot(dot)
struct s_dot *dot;
{
int i;
for(i=0;i<dot_n;i++)
  if(dot_list[i]==dot) break;
if(i<dot_n) return;
if(dot_n>=DOT_LIST_SIZE-1)
  {
   zz_error(ERROR,"dump: dot list overflow");
   return;
  }
dot_list[dot_n++]=dot;
}


/*----------------------------------------------------------------------------*/

static void dump_add_nt(nt)
struct s_nt *nt;
{
int i;
for(i=0;i<nt_n;i++)
  if(nt_list[i]==nt) break;
if(i<nt_n) return;
if(nt_n>=NT_LIST_SIZE-1)
  {
   zz_error(ERROR,"dump: nt list overflow");
   return;
  }
nt_list[nt_n++]=nt;
}


/*----------------------------------------------------------------------------*/

static void dump_term_tran(termtran)
struct s_term_tran *termtran;
{
struct s_content *term;
struct s_dot *next;
char buffer[256],tmp[40];
term = &(termtran->term);
next = termtran->next;
sprintz(buffer,"'%z' --> ",term);
if(next) 
  {
   sprintf(tmp,"dot%d",next->id);
   dump_add_dot(next);
  }
else strcpy(tmp,"nil");
strcat(buffer,tmp);
dump_tran(buffer);
}


/*----------------------------------------------------------------------------*/

static void dump_nt_tran(nttran)
struct s_nt_tran *nttran;
{
struct s_nt *nt;
struct s_dot *next;
char buffer[256],tmp[40];
nt = nttran->nt;
dump_add_nt(nt);
next = nttran->next;
strcpy(buffer,nt->name);
strcat(buffer," --> ");
if(next) 
  {
   sprintf(tmp,"dot%d",next->id);
   dump_add_dot(next);
  }
else strcpy(tmp,"nil");
strcat(buffer,tmp);
dump_tran(buffer);
}

/*----------------------------------------------------------------------------*/

static dump_rule(rule)
struct s_rule *rule;
{
  char buffer[256];
  printz(buffer,"reduce: %r", rule);
  dump_tran(buffer);
}

/*----------------------------------------------------------------------------*/

dumpnet(ntname)
char *ntname;
{
int i;
struct s_nt *nt;
struct s_dot *dot;
nt = find_nt(ntname);

nt_n = 1;
nt_list[0] = nt;

for(i=0;i<nt_n;i++)
  do_dumpnet(nt_list[i]);
}

/*----------------------------------------------------------------------------*/

do_dumpnet(nt)
struct s_nt *nt;
{
int i;
struct s_dot *dot;
dot = nt->first_dot;
printf("[%s]\n",nt->name);
if(!dot)
   return 0;

dot_n=1;
dot_list[0] = dot;

for(i=0;i<dot_n;i++)
  {
   dot = dot_list[i];
   hd_dot = dot;
   avl_scan(dot->termtree,dump_term_tran);
   avl_scan(dot->nttree,dump_nt_tran);
   if(dot->rule)
     dump_rule(dot->rule);
   if(hd_dot)
     dump_tran("empty");
  }
printf("\n");
}

static char sccsid[]="@(#)dumpnet.c	6.1\t9/7/94";
static char rcsid[] = "$Id: dumpnet.c,v 1.5 2002/06/03 11:06:13 kibun Exp $ ";
