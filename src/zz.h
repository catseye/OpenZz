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
  Zz library exported functions

  $Id: zz.h,v 1.9 2002/06/03 11:06:13 kibun Exp $
 */

#ifndef __ZZ_H__
#define __ZZ_H__

//#include "zlex.h"
//#include "trace.h"

#include <sys/types.h> /* for int32_t, ... */
#include <stdio.h> /* for FILE ... */

#ifdef  __cplusplus
extern "C" {
#endif

const char* zz_version(void);

void zz_init(void);

/* "kernel" scope open/close */
void zz_set_kernel_flag(int bool_value);

void zz_set_output(const char* output_filename);
void zz_set_output_stream(FILE* output_stream);

int  zz_parse_string(const char *s);
int  zz_parse_file(const char* input_source_filename);
int  zz_parse_pipe(void);
  /* this one is really in libozzi.xx */
int  zz_parse_tt(void); 

void zz_set_default_extension(const char* include_file_def_extension);
void zz_set_default_include_dir(const char* default_include_dir);

void zz_set_prompt(const char *prompt);
const char* zz_get_prompt(void);

/* error reporting related stuff */
int zz_get_param_stack_depth();
int zz_get_error_number();

/* scanner extension */
struct s_content;
struct s_tag;
/*   struct s_content accessors */
long    zz_scnt_get_lvalue (struct s_content *);
int32_t zz_scnt_get_ivalue (struct s_content *);
float   zz_scnt_get_fvalue (struct s_content *);
void*   zz_scnt_get_pvalue (struct s_content *);
char*   zz_scnt_get_svalue (struct s_content *);
double  zz_scnt_get_dvalue (struct s_content *);
int64_t zz_scnt_get_llvalue(struct s_content *);

long    zz_scnt_getv_lvalue (struct s_content *, int);
int32_t zz_scnt_getv_ivalue (struct s_content *, int);
float   zz_scnt_getv_fvalue (struct s_content *, int);
void*   zz_scnt_getv_pvalue (struct s_content *, int);
char*   zz_scnt_getv_svalue (struct s_content *, int);
double  zz_scnt_getv_dvalue (struct s_content *, int);
int64_t zz_scnt_getv_llvalue(struct s_content *, int);

void zz_scnt_set_lvalue (struct s_content *, long   );
void zz_scnt_set_ivalue (struct s_content *, int32_t);
void zz_scnt_set_fvalue (struct s_content *, float  );
void zz_scnt_set_pvalue (struct s_content *, void*  );
void zz_scnt_set_svalue (struct s_content *, char*  );
void zz_scnt_set_dvalue (struct s_content *, double );
void zz_scnt_set_llvalue(struct s_content *, int64_t);

const char* zz_scnt_get_tag_name(struct s_content *);

typedef int (*zz_tag_sprint_cb)(char *, struct s_content *);
typedef int (*zz_tag_fprint_cb)(FILE *, struct s_content *);
typedef int (*zz_tag_cdtor)(struct s_content *cnt, const char *param_name);
typedef struct s_content * (*zz_tag_cast)(struct s_content* src, struct s_tag* tgt_type_tag, struct s_content* tgt);

int zz_lex_add_new_tag(const char* tag_name, 
		       zz_tag_sprint_cb, 
		       zz_tag_fprint_cb, 
		       zz_tag_cdtor pctor, 
		       zz_tag_cdtor pdtor, 
		       zz_tag_cast cst);

#define zz_lex_add_new_tag2(TAG_NAME, TAG_SP, TAG_FP) zz_lex_add_new_tag(TAG_NAME, TAG_SP, TAG_FP, 0, 0)

int zz_lex_remove_tag(const char* tag_name);

/* tracing stuff */

#define TRACE_REDUCE   0x1
#define TRACE_ZZACTION 0x2
#define TRACE_SCOPE    0x4
#define TRACE_LRSTACK  0x8
#define TRACE_ALL (TRACE_REDUCE|TRACE_ZZACTION|TRACE_SCOPE|TRACE_LRSTACK)

void zz_set_trace_mask(int mask);

#ifdef  __cplusplus
}
#endif

#endif /* __ZZ_H__ */
