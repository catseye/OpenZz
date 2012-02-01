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

#ifndef __PARAM_H__
#define __PARAM_H__

int set_param(char *name,struct s_content *cnt);
int gset_param(char *name, struct s_content *cnt);
int gnset_param(char *name,struct s_content *cnt,int delta);
int unset_param(char *name);
int param_substitute(struct s_content *token,char **paramname);
int local_param_substitute(struct s_content *token,struct s_content *paramname);
int s_param_filter(int argc, struct s_content argv[], struct s_content *ret);
int list_params();
int show_param_memory();
int push_param_scope();
int pop_param_scope();

#endif // __PARAM_H__

