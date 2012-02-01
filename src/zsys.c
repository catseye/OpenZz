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

//#define DEBUG 1

#include <stdint.h>
#include "zlex.h"
#include "rule.h"
#include "err.h"
#include "trace.h"

static struct {struct s_content bead;char *name;} beads[MAX_RULE_LENGTH];
static int bead_n=0;
static struct {struct s_content cnt;char *tag_name,is_return;} 
   cur_action = {{0,0},0,0};

/*---------------------------------------------------------------------------*/

/* aggiungi una bead alla regola corrente */

z_bead(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
if(bead_n>=MAX_RULE_LENGTH)
  {
   zz_error(ERROR,"rule too long");
   return;
  }
beads[bead_n].bead=argv[1];
beads[bead_n].name=(argc==3)?(char*)s_content_value(argv[2]):0;
bead_n++;
return 1;
}

/*---------------------------------------------------------------------------*/

/* l'azione della regola corrente (zz-action o special) */

z_set_action(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
cur_action.cnt=argv[0];
cur_action.is_return=0;
return 1;
}

/*---------------------------------------------------------------------------*/

/* l'azione della regola corrente (return [as]) */

z_set_return_action(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
cur_action.cnt=argv[0];
cur_action.is_return=1;
if(argc==2)
  cur_action.tag_name = (char*)s_content_value(argv[1]);
else
  cur_action.tag_name = 0;
return 1;
}

/*---------------------------------------------------------------------------*/

/* inserisce una regola nello scope di default */

z_link_rule_default(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
do_z_link_rule(s_content_value(argv[0]),0);
return 1;
}

/*---------------------------------------------------------------------------*/

/* inserisce una regola in uno scope */

z_link_rule(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
do_z_link_rule(s_content_value(argv[1]),s_content_value(argv[0]));
return 1;
}

/*---------------------------------------------------------------------------*/

do_z_link_rule(sint_name,scope_name)
char *sint_name,*scope_name;
{
int i;
struct s_rule *rule;
open_rule(zlex_strsave(sint_name));
for(i=0;i<bead_n;i++)
  if(beads[i].name)
    append_nt_bead(s_content_value(beads[i].bead),beads[i].name);
  else
    append_t_bead(&beads[i].bead);
bead_n=0;

if(cur_action.is_return)
  {
   setaction_return(&cur_action.cnt,cur_action.tag_name);
  }
else
  {
   if(cur_action.cnt.tag==tag_list)
     setaction_exelist(&cur_action.cnt);   
   else if(cur_action.cnt.tag==tag_ident)
     {
      if(strcmp((char*)s_content_value(cur_action.cnt),"pass")==0)
        setaction_pass();
      else if(strcmp((char*)s_content_value(cur_action.cnt),"rreturn")==0)
        setaction_rreturn();
      else if(strcmp((char*)s_content_value(cur_action.cnt),"assign")==0)
        setaction_assign();
      else
        zz_error(WARNING,"bad type. action ignored.");
     }
  }
rule=close_rule();
insert_rule(scope_name,rule);
cur_action.cnt.tag=tag_none;
s_content_value(cur_action.cnt)=0;
cur_action.is_return=0;
return 1;
}

/*---------------------------------------------------------------------------*/

#ifdef CICCIONE
z_open_rule(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
char *s;
s = (char*)s_content_value(argv[0]);
open_rule(zlex_strsave(s));
return 1;
}


z_close_rule(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
int i;
char *scopename;
struct s_content action;
struct s_rule *rule;
if(argc==3)
  {
   action=argv[2];
   scopename=0;
  }
else
 {
  action=argv[3];
  scopename=(char*)s_content_value(argv[0]);
 }
if(action.tag==tag_list)
   setaction_exelist(&action);   
else if(action.tag==tag_ident)
  {
   if(strcmp(s_content_value(action),"pass")==0)
     setaction_pass();
   else if(strcmp(s_content_value(action),"rreturn")==0)
     setaction_rreturn();
   else if(strcmp(s_content_value(action),"assign")==0)
     setaction_assign();
   else
     zz_error(WARNING,"bad action type. action ignored");
  }
rule=close_rule();
insert_rule(scopename,rule);
return 1;
}


z_append_term(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
append_t_bead(&argv[0]);
return 1;
}

z_append_nt(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
append_nt_bead(s_content_value(argv[0]),s_content_value(argv[1]));
return 1;
}

#endif

/*---------------------------------------------------------------------------*/

z_set_when_change_action(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
struct s_rule *rule,*get_last_rule();
rule = get_last_rule();
if(rule && argc>0)
  {
   rule->when_change_action = argv[0];
  }
return 1;
}


/*---------------------------------------------------------------------------*/

z_set_when_exit_scope(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
struct s_rule *rule,*get_last_rule();
rule = get_last_rule();
if(rule && argc>0)
  {
    zz_trace("set_when_exit_scope: %z\n", &argv[0]);
   rule->when_exit_scope = argv[0];
  }
return 1;
}


/*----------------------------------------------------------------------------*/

static char sccsid[]="@(#)zsys.c	6.1\t9/7/94";
static char rcsid[] = "$Id: zsys.c,v 1.6 2002/06/03 11:06:13 kibun Exp $ ";
