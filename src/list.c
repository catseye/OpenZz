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

LIST.C

List token type management.
(uses ZLEX)

by Simone Cabasino 

==============================================================================*/

#include <stdio.h>
#include <stdint.h>
#include "zlex.h"
#include "list.h"
#include "mem.h"
#include "err.h"
#include "trace.h"

int list_mem = 0;

/*---------------------------------------------------------------------------*/

void create_list(cnt,size)
struct s_content *cnt;
int size;
{
  struct s_list *lst;
  if(size<=0) size = 10;
  lst = (struct s_list *) malloc(sizeof(struct s_list));
  zz_assert(lst);
  list_mem += sizeof(*lst);
  lst->size=size;
  lst->n=0;
  lst->pos = 0;
  lst->array = (struct s_content *) calloc(lst->size,sizeof(struct s_content));
  zz_assert(lst->array);
  list_mem += lst->size*sizeof(struct s_content);
  cnt->tag = tag_list;
  // Sap:  s_content_value(*cnt) = lst;
  s_content_pvalue(*cnt) = lst;
}

/*---------------------------------------------------------------------------*/

void delete_list(cnt)
struct s_content *cnt;
{
  struct s_list *lst;
  if(cnt->tag!=tag_list)
    {printf("Internal error - delete_list; argument must be LIST\n");exit(1);}
  lst = (struct s_list *) s_content_value(*cnt);
  list_mem -= sizeof(*lst) +lst->size*sizeof(struct s_content);
  cfree(lst->array);
  cfree(lst);
  cnt->tag=0;
  s_content_value(*cnt)=0;
}

/*---------------------------------------------------------------------------*/


static 
void copy_list(target,source)
     struct s_content *target,*source;
{
int i,j;
struct s_list *target_lst,*source_lst;
if(source->tag!=tag_list)
  {
    zz_error(FATAL_ERROR, 
	     "ERROR: copy_list argument must be a list\n"
	     "argument: /%z/\n",source);
    exit(1);
  }
source_lst = (struct s_list *) s_content_value(*source);
create_list(target,source_lst->size);
target_lst = (struct s_list *) s_content_value(*target);
target_lst->n = source_lst->n;
for(i=j=0;i<source_lst->n;i++)
  if(source_lst->array[i].tag==tag_list)
    copy_list(& (target_lst->array[j++]), & (source_lst->array[i]));
  else if(source_lst->array[i].tag!=tag_none)
    target_lst->array[j++] = source_lst->array[i];
}

/*---------------------------------------------------------------------------*/

void append_to_list(cnt1,cnt2)
struct s_content *cnt1,*cnt2;
{
struct s_list *lst;
 zz_trace("append_to_list\n");
if(cnt1->tag!=tag_list)
  {printz("Error - append_to_list; first argument must be a list\n");
   printz("first arg: /%z/  second arg: /%z/\n",cnt1,cnt2);
   exit(1);}
if(cnt2->tag==tag_none)return;
lst = (struct s_list *) s_content_value(*cnt1);
if(lst->n>=lst->size)
  {
   list_mem -= lst->size*sizeof(struct s_content);
   lst->size += 100;
   lst->array = (struct s_content *) 
      realloc(lst->array,sizeof(struct s_content)*lst->size);   
   list_mem += lst->size*sizeof(struct s_content);
  }
if(cnt2->tag==tag_list)
  copy_list(lst->array+lst->n++,cnt2);
else
  lst->array[lst->n++] = *cnt2;
}

/*---------------------------------------------------------------------------*/

struct s_content *s_concat_list(struct s_content *cnt1,struct s_content *cnt2)
{
 int i,j,n;
 struct s_list *lst,*lst1,*lst2;
 struct s_content *cnt;
 zz_trace("s_concat_list\n"); 
 cnt=(struct s_content *)calloc(1,sizeof(struct s_content));
 /* printf("s_concat_list: sono qui!!\n"); */
 if(cnt1->tag!=tag_list || cnt2->tag!=tag_list)
   {printz("Error - s_concat_list; arguments must be lists\n");
    printz("first arg: /%z/  second arg: /%z/\n",cnt1,cnt2);
    exit(1);}
 lst1 = (struct s_list *) s_content_value(*cnt1);
 lst2 = (struct s_list *) s_content_value(*cnt2);
 n=lst1->n+lst2->n;
 create_list(cnt,n);
 lst  = (struct s_list *) s_content_value(*cnt);
 for(i=0;i<lst1->n;i++)
   if(lst1->array[i].tag!=tag_none)
     lst->array[i] = lst1->array[i];
 for(i=lst1->n,j=0;i<n;j++)
   if(lst2->array[j].tag!=tag_none)
     lst->array[i++] = lst2->array[j];
 return cnt;
}

/*---------------------------------------------------------------------------*/


/* merge_list(dest, src) - dest is expanded */
void merge_list(struct s_content *cnt1, struct s_content*cnt2)
{
  int i,j,n;
  struct s_list *lst1,*lst2;
  struct s_content cnt;
  
  zz_trace("merge_list\n"); 

  if(cnt1->tag!=tag_list || cnt2->tag!=tag_list) {
    printz("Error - merge_list; arguments must be lists\n");
    printz(" List args:\n  first arg : %z\n  second arg: %z\n", cnt1, cnt2);
    exit(1);
  }

  lst1 = (struct s_list *) s_content_value(*cnt1);
  lst2 = (struct s_list *) s_content_value(*cnt2);
  n=lst1->n+lst2->n;

  if(lst1->size<n) {   
    list_mem -= lst1->size*sizeof(struct s_content);
    lst1->size = 100*((n+99)/100);
    lst1->array = (struct s_content *)
      realloc(lst1->array,sizeof(struct s_content)*lst1->size);   
    list_mem += lst1->size*sizeof(struct s_content);
    zz_assert(lst1->array);
  }

  for(i=lst1->n,j=0;i<n;j++) {
    /*
      if(lst2->array[j].tag==tag_list)
      copy_list(&(lst1->array[i++]),&(lst2->array[j]) );
      else */
  
    if(lst2->array[j].tag!=tag_none)
      lst1->array[i++] = lst2->array[j];
  }

  lst1->n=n;
}


/*---------------------------------------------------------------------------*/


int get_list_size(cnt)
struct s_content *cnt;
{
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printz("Error - list_size; argument must be a list. argument: /%z/\n",cnt);
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
return lst->n;
}

/*---------------------------------------------------------------------------*/

int get_list_pos(cnt)
struct s_content *cnt;
{
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printz("Error - get_list_pos; argument must be a list. arg: /%z/\n",cnt);
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
return lst->pos;
}

/*---------------------------------------------------------------------------*/

void list_seek(cnt,pos)
struct s_content *cnt;
int pos;
{
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printz("Error - list_seek; argument must be a list. argument: /%z/\n",cnt);
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
if(lst->n<=0 || pos<0) pos=0;
else if(pos>lst->n) pos=lst->n;
lst->pos=pos;
}

/*---------------------------------------------------------------------------*/


struct s_content *list_extract(cnt,pos)
struct s_content *cnt;
int pos;
{
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printz("Error - list_extract; argument must be a list. argument: /%z/\n",cnt);
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
if(pos<0) return 0;
if(pos>=lst->n) return 0;
return lst->array+pos;
}

/*---------------------------------------------------------------------------*/

struct s_content *next_list_item(cnt)
struct s_content *cnt;
{
struct s_content *itm;
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printz("Error - next_list_item; argument must be a list. arg: /%z/\n",cnt);
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
if(lst->pos>=lst->n)
  return 0;
else return lst->array+lst->pos++;
}


/*---------------------------------------------------------------------------*/

int fprint_list(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
int i;
struct s_list *lst;
lst = (struct s_list*)s_content_pvalue(*cnt);
fprintz(chan,"{ ");
for(i=0;i<lst->n;i++)
   fprintz(chan,"%z ",lst->array+i);
fprintz(chan,"}");
 return 0;
}

/*---------------------------------------------------------------------------*/

int sprint_list(string,cnt)
char *string;
struct s_content *cnt;
{
struct s_list *lst;
char buffer[256];
int i;
lst = (struct s_list *)s_content_pvalue(*cnt);
*string++ = '{';
*string++ = ' ';
for(i=0;i<lst->n;i++)
  {
   sprintz(buffer,"%z ",lst->array+i);
   strcpy(string,buffer);
   while(*string)string++;
  }
*string++ = '}';
*string++ = '\000';
 return 0;
}

/*---------------------------------------------------------------------------*/

int fprint_list_image(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
int i;
struct s_list *lst;
lst = (struct s_list*)s_content_value(*cnt);
fprintz(chan,"{ ");
for(i=0;i<lst->n;i++)
   fprintz(chan,"%w ",lst->array+i);
fprintz(chan,"}");
 return 0;
}

/*---------------------------------------------------------------------------*/

int sprint_list_image(string,cnt)
char *string;
struct s_content *cnt;
{
struct s_list *lst;
char buffer[256];
int i;
lst = (struct s_list *)s_content_value(*cnt);
*string++ = '{';
*string++ = ' ';
for(i=0;i<lst->n;i++)
  {
   sprintz(buffer,"%w ",lst->array+i);
   strcpy(string,buffer);
   while(*string)string++;
  }
*string++ = '}';
*string++ = '\000';
 return 0;
}

/*---------------------------------------------------------------------------*/

void print_list(cnt)
struct s_content *cnt;
{
int i;
struct s_list *lst;
if(cnt->tag!=tag_list)
  {printf("Error - list_size; argument must be a list\n");
   exit(1);}
lst = (struct s_list *) s_content_value(*cnt);
printz("{ ");
for(i=0;i<lst->n;i++)
   printz("%z ",lst->array+i);
printz("}");
}

/*---------------------------------------------------------------------------*/

void show_list_memory()
{
  PRINTMEM("list.mem",list_mem)
}

/*---------------------------------------------------------------------------*/

/*
 * dump_list() - detailed dump of list contents for debuging
 *    Invoke: dump_list(&lst);
 */
void dump_list(cnt)
     struct s_content *cnt;
{
  int i;
  struct s_list *lst;
  struct s_content *item;

  if(cnt->tag!=tag_list) {
    printf("Error - dump_list; argument must be a list\n");
    exit(1);
  }

  lst = (struct s_list *) s_content_value(*cnt);

  printf("List Contents:\n");

  for(i=0; i<lst->n; i++) {
    item = lst->array+i;
    printf(" Item(%i): type=%s,\t", i+1, item->tag->name);
    printz("value=\"%z\"\n", item);
  }

  printf("End List.\n");
}

static char rcsid[] = "$Id: list.c,v 1.10 2002/01/14 14:27:06 rossetti Exp $ ";
