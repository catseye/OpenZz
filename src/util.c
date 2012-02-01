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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "zz.h"
#include "zlex.h"
#include "list.h"
#include "rule.h"
#include "err.h"
#include "mem.h"
#include "param.h"
#include "trace.h"
#include "sys.h"

static const char *std_prompt = "ozz> ";                // Default prompt

static const char* ozz_version=VERSION;

/*---------------------------------------------------------------------------*/


const char* zz_version(void)
{
  return ozz_version;
}

/*---------------------------------------------------------------------------*/


void zz_init(void)
{
  zz_set_output_stream(stdout);
  kernel();
  zkernel();
  zz_set_kernel_flag(0);
}

/*---------------------------------------------------------------------------*/


void zz_set_prompt(prompt)
     const char *prompt;
{
  zz_assert(prompt);
  std_prompt=prompt;
}

const char* zz_get_prompt()
{
  zz_assert(std_prompt);
  return std_prompt;
}

/*---------------------------------------------------------------------*/

/*
 * zz_parse_string(s) - Executes a string holding valid zz source code.
 */
int zz_parse_string(const char *s)
{
  int ret;
  struct s_content lst,tmp;
  
  create_list(&lst,0);
  while(*s)
    {
      zlex((char**)&s,&tmp);
      append_to_list(&lst,&tmp);
    }
  source_list(&lst,0);
  ret = parse(find_nt("root"));
  pop_source();
  delete_list(&lst);
  return ret;
}

/*----------------------------------------------------------------------------*/

#define SCNT_ACCESSOR(TYPE, PREFIX)				\
TYPE zz_scnt_get_##PREFIX##value(struct s_content *pscnt)	\
{								\
  zz_assert(pscnt);						\
  return s_content_##PREFIX##value(*pscnt);			\
}								\
TYPE zz_scnt_getv_##PREFIX##value(struct s_content *pscnt, int idx)\
{								\
  zz_assert(pscnt);						\
  return s_content_##PREFIX##value(pscnt[idx]);			\
}								\
void zz_scnt_set_##PREFIX##value(struct s_content *pscnt, TYPE v)\
{								\
  zz_assert(pscnt);						\
  s_content_##PREFIX##value(*pscnt) = v;			\
}

SCNT_ACCESSOR(long,l)
SCNT_ACCESSOR(int32_t,i)
SCNT_ACCESSOR(float,f)
SCNT_ACCESSOR(void*,p)
SCNT_ACCESSOR(char*,s)
SCNT_ACCESSOR(double,d)
SCNT_ACCESSOR(int64_t,ll)

const char* zz_scnt_get_tag_name(struct s_content *cnt)
{
  zz_assert(cnt);
  zz_assert(s_content_tag(*cnt));
  return s_content_tag(*cnt)->name;
}

/*----------------------------------------------------------------------------*/

static const char* rcsid="$Revision";

/*----------------------------------------------------------------------------*/
