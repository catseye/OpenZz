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

#ifndef RULE_DEFINED
#define RULE_DEFINED

#include "avl.h"
#include "zlex.h"


struct s_param_value {
	char *name;
	struct s_content value;
	struct s_nt *nt;
	};

struct s_bead {
	struct s_content cnt;
	char *name;
	};
	
struct s_rule {
   	struct s_content cnt_prec;
   	struct s_content action,when_change_action,when_exit_scope;
	int action_type,bead_n,bead_size;	
        struct s_tag *sproc_tag;
	struct s_bead *beads;
	int scope,kernel;
	struct s_rule **table_backptr,*prev_rule,*next_rule;
        int segment_id; /* serve per la scrittura con /write rules */
	};

struct s_nt {
	char *name;
	char *prompt;
	struct s_dot *first_dot;
	};

struct s_term_tran {
	int count;
	struct s_content term;
 	struct s_dot *next;
	};

struct s_nt_tran {
	int count;
	struct s_nt *nt;
 	struct s_dot *next;
	};

struct s_dot {
	int id,rule_count;
        struct s_nt *nt;
	TREE *termtree,*nttree;
        struct s_rule *rule;
	char match_param,match_any;
	long int set;
	};

struct s_dot 
	*create_dot(),
	*find_tran(),
	*insert_tran()
	;

struct s_nt
	*find_nt();

void free_rule(void * rule_param/*, void* dummy_param*/);
void init_rule();
struct s_rule *close_rule();
struct s_dot *find_nt_tran(),*find_term_tran();

#define MAX_RULE_LENGTH	30

#define ACT_T_NONE 		0
#define ACT_T_EXECUTE_LIST 	1
#define ACT_T_EXECUTE_PROC 	2
#define ACT_T_EXECUTE_SPROC 	3
#define ACT_T_RETURN  		4
#define ACT_T_PASS  		5
#define ACT_T_LIST  		6
#define ACT_T_APPEND  		7
#define ACT_T_MERGE_ALL 	8
#define ACT_T_MERGE 		9
#define ACT_T_RRETURN 		10
#define ACT_T_ASSIGN 		11

#endif
