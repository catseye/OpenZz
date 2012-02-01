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

#ifndef LIST_DEFINED
#define LIST_DEFINED

#include "zlex.h"

struct s_list {
  int size,n,pos; 
  struct s_content *array;
};

struct s_content *next_list_item();
void create_list(struct s_content *list_cnt,int num_items);
void delete_list(struct s_content *list_cnt);
/* append a new item to list */
void append_to_list(struct s_content *dest_list, struct s_content *src_cnt);
/* creates a new list with copied elements from cnt1 + cnt2*/
struct s_content *s_concat_list(struct s_content *cnt1,struct s_content *cnt2);
/* appends elements of src_cnt to dest_list */
void merge_list(struct s_content *dest_list, struct s_content *src_cnt);
int get_list_size(struct s_content *cnt);
int get_list_pos(struct s_content *cnt);
void list_seek(struct s_content *cnt,int pos);
struct s_content *list_extract(struct s_content *cnt,int pos);
int fprint_list(FILE *chan,struct s_content *cnt);
int sprint_list(char *string,struct s_content *cnt);
int fprint_list_image(FILE *chan,struct s_content *cnt);
int sprint_list_image(char *string,struct s_content *cnt);
void print_list(struct s_content *cnt);
void show_list_memory(void);
void dump_list(struct s_content *cnt);

#endif
