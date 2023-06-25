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

#ifndef __KERNEL_H__
#define __KERNEL_H__

/*PROTOTYPES*/
void init_zlex(void);
void setaction_exesproc(int (*sproc)(), struct s_tag *tag);
void setaction_exeproc(int (*proc)(), struct s_tag *tag);
void setaction_pass(void);
void setaction_merge();
void setaction_merge_all();
void setaction_list();
void setaction_append();
void setaction_return(struct s_content *, char *);
void insert_rule();  /* actual signature is: void insert_rule(char *, struct s_rule *); */
void append_t_bead(struct s_content *);
void append_nt_bead(char *, char *);
void set_recovery(char *, char *);
void set_nt_prompt(char *, const char *);

#define OPEN(S) {INIT_ZLEX open_rule(zlex_strsave( #S ));}
#define GSB(S)  append_nt_bead(zlex_strsave( #S),0);
#define M(S)    {struct s_content cnt;cnt.tag=tag_qstring;\
		s_content_value(cnt)=(long)S;append_t_bead(&cnt);}
#define M_EOF   {struct s_content cnt;cnt.tag=tag_eof;\
		s_content_value(cnt)=0;append_t_bead(&cnt);}
#define END     insert_rule(0,close_rule());

/*
#define ACTION(A) {int A();k_action(A);}
#define RETURN_PROCEDURE(A) {int A();k_return_procedure(A);}
#define RETURN_STRING(S) k_return_string(S);
*/

#define RETURN_IDENT(S) {struct s_content tmp;char *s=S;zlex(&s,&tmp);\
			  setaction_return(&tmp,tmp.tag->name);}

#define PASS setaction_pass();
#define LIST setaction_list();
#define APPEND setaction_append();
#define MERGE setaction_merge();
#define MERGE_ALL setaction_merge_all();

#define PROC(P)   {int P(); setaction_exeproc(P,0);}
#define FUN(T,P)  {int P(); setaction_exeproc(P,T);}
#define SPROC(P)  {int P(); setaction_exesproc(P,tag_none);}
#define SFUN(T,P) {int P(); setaction_exesproc(P,T);}

#endif // __KERNEL_H__
