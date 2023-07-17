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

/*
  Public interface for C bindings (zzbind.h)

  $Id: interface.c,v 1.6 2002/01/11 11:52:02 brooks Exp $
*/

#include <stdint.h>
#include "zlex.h"
#include "rule.h"

/*PROTOTYPES*/
void open_rule(char *);
void setaction_exeproc(int (*proc)(), struct s_tag *tag);
void setaction_exesproc(int (*sproc)(), struct s_tag *tag);
void setaction_pass(void);
void insert_rule(char *, struct s_rule *);
void init_zlex(void);
void append_t_bead(struct s_content *cnt);
void append_nt_bead(char *ntname, char *beadname);

/*-------------------------------------------------------------------------*/

int zz_bind_open(const char *s)
{
  if(!init_zlex_done)
    init_zlex();
  open_rule(zlex_strsave(s));
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_match(const char *s)
{
  append_nt_bead(zlex_strsave(s),0);
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_keyword(const char *s)
{
  struct s_content cnt;
  cnt.tag=tag_qstring;
  s_content_value(cnt)=(long)s;
  append_t_bead(&cnt);
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_close(void)
{
  insert_rule(0,close_rule());
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_call(int (*proc)())
{
  setaction_exesproc(proc,tag_none);
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_call_fun(int (*proc)(), const char *tag)
{
  setaction_exesproc(proc,find_tag(tag));
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_call_fun_tag(int (*proc)(), struct s_tag *tag)
{
  setaction_exesproc(proc,tag);
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_call_exe_proc(int (*proc)(), const char *tag)
{
  setaction_exeproc(proc,find_tag(tag));
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_call_exe_no_tag(int (*proc)())
{
  setaction_exeproc(proc,0);
  return 1;
}

/*-------------------------------------------------------------------------*/

int zz_bind_pass(void)
{
  setaction_pass();
  return 1;
}

/*-------------------------------------------------------------------------*/
static const char rcsid[] = "$Id: interface.c,v 1.6 2002/01/11 11:52:02 brooks Exp $ ";
