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
#include <stdlib.h>
#include <string.h>

#include "rule.h"


struct s_l_node {
   struct s_l_node *next;
   int dot_n;
   struct s_dot **dots;
   int id,ref_count;
  };


#define MAX_N 100
static struct s_l_node *sets[MAX_N+1];
static int id=1000;

/*----------------------------------------------------------------*/

struct s_l_node *lazy_search(dots,n)
int n;
struct s_dot *dots[];
{
struct s_dot *dot;
struct s_l_node **pnode,*node;
int i,j,k;
if(n>MAX_N) i = MAX_N;
else i=n;
pnode = &sets[i];
while(node= *pnode)
  {
   if(node->dot_n!=n) continue;
   for(j=0;j<n;j++)
     {
      dot=dots[j];
      for(k=0;k<n && node->dots[k]!=dot;k++);
      if(k>=n) break;
     }
   if(j>=n)
     {
      node->ref_count++;
      return node;
     }
   else
     {
      pnode = &(node->next);
     }
  }
node = (struct s_l_node *)malloc(sizeof(struct s_l_node));
node->next= *pnode;
*pnode = node;
node->dot_n=n;
node->dots=(struct s_dot**)calloc(n,sizeof(struct s_dot *));
memcpy(node->dots,dots,n*sizeof(struct s_dot*));
node->id=id++;
node->ref_count = 1;
return node;
}

/*----------------------------------------------------------------*/


void lazy_rec(dots,len)
struct s_dot *dots[];
int len;
{
int i;
struct s_l_node *node;
node = lazy_search(dots,len);
/*
printf("--> set%d\n",node->id);
*/
}



/*----------------------------------------------------------------*/


int print_lazy_report()
{
struct s_l_node *node;
int k,i,count,flag;
printf("lazy report:\n");
flag= -1;
for(i=0;i<MAX_N;i++)
  if(sets[i])
    {
     printf("(%d dots)\n",i);
     node=sets[i];
     while(node)
       {
	printf("  set%d  (%d ref)\n",node->id,node->ref_count);
	node=node->next;
       }
    }

i=MAX_N;
if(sets[i])
  {
   printf("(>%d dots)\n",i);
   node=sets[i];
   while(node)
     {
      printf("  set%d (%d dots) (%d ref)\n",
        node->id,node->dot_n,node->ref_count);
      node=node->next;
     }
  }
  return 0;
}
static char sccsid[]="@(#)lazy.c	6.1\t9/7/94";
static char rcsid[] = "$Id: lazy.c,v 1.3 2002/01/11 11:52:02 brooks Exp $ ";
