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

#ifndef __SOURCE_H__
#define __SOURCE_H__

#define NO_SOURCE     0
#define SOURCE_FILE   1
#define SOURCE_TT     2
#define SOURCE_LIST   3

// Size of input buffer:
#define MAX_INPUT_LINE_LENGTH 256

// Prototype predeclaration:
struct s_source;

struct s_source_file
{
  char *filename;
  FILE *chan;
  char row[MAX_INPUT_LINE_LENGTH],*s,*old;
};

struct s_source_tt 
{
  const char *prompt;
  char row[MAX_INPUT_LINE_LENGTH],*s,*old;
};

struct s_source_list
{
  void *id;
  struct s_content *list;
  int current_pos,line_n;
};

struct s_source
{
  int type,eof,line_n;
  struct s_content cur_token;
  void (*read_next_token)(struct s_source *src);
  union
  {
    struct s_source_file file;
    struct s_source_tt tt;
    struct s_source_list list;
  } src;
};

#endif // __SOURCE_H__


extern struct s_source *new_source(void (*next_token_function)(struct s_source *src));

extern struct s_content cur_token;

extern int (*source_line_routine)();
extern int (*find_prompt_proc)();
