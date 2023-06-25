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

  ZLEX.C

  Lexical analyzer
  (by Simone Cabasino)


==============================================================================*/
#define ZLEX

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <stdarg.h>
#include "avl.h"
#include "zlex.h"
#include "mem.h"
#include "err.h"
//#define DEBUG
#include "trace.h"

/*----------------------------------------------------------------------------*/

#define cntlen(CNT)	\
	(*(((struct s_content *)(CNT))->tag->len)) (CNT) 
#define cntcpy(T,S)	\
	(*(((struct s_content*)(S))->tag->copy)) ((T),(S)) 
#define cntsprint(B,CNT) \
	(*(((struct s_content*)(CNT))->tag->sprint)) ((B),(CNT)) 


#define LEFT_ASSOCIATIVE 1
#define RIGHT_ASSOCIATIVE 0

#define ZLEX_PRECEDENCE(T)	\
	(((T)->tag==tag_ident || (T)->tag==tag_char) ? \
		   *(((unsigned char*) ((T)->value))-2)\
		:  100)


#define ZLEX_IS_LEFT_ASSOCIATIVE(T)	\
	(((T)->tag==tag_ident || (T)->tag==tag_char) ? \
		   (*(((char*) ((T)->value))-3) == LEFT_ASSOCIATIVE)\
		:  1)

/*----------------------------------------------------------------------------*/

#define lexical_error(FMT, ARGS...) zz_error(LEXICAL_ERROR, FMT , ## ARGS)

/*----------------------------------------------------------------------------*/

static int zlex_case_insensitive = 1;
static int zlex_realconst_double = 0; /* real const default type */
static int zlex_intconst_int64   = 0;
static int zlex_parse_eol        = 1;

static int zlex_strsaved_mem=0;
static int zlex_qstring_mem=0;
static int zlex_tag_mem=0;
static int ignore_block_flag=0;
static int braket_level=0;

#define MAX_TOKEN_LENGTH	255

/*----------------------------------------------------------------------------*/

extern int fprint_list(),sprint_list(),fprint_list_image(),sprint_list_image();

static int tags_initialized=0;

static int cur_prec = 100;
static int cur_assoc = LEFT_ASSOCIATIVE;

int str_term_n=0;

struct s_term {
	char *name;
	};

TREE *str_term_table=0;


/*----------------------------------------------------------------------------*/

#define isident(a) (isalnum(a) || (a)== '$' || (a) == '_')

#define TERM_MAGIC 123

/*----------------------------------------------------------------------------*/

int sprint_string(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  //zz_assert(cnt && s_content_tag(*cnt)==tag_qstring);
  strcpy(buffer,(char*)s_content_value(*cnt));
}

int fprint_string(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  //zz_assert(cnt && s_content_tag(*cnt)==tag_qstring);
  fprintf(chan,"%s",s_content_value(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_int(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_int);
  sprintf(buffer,"%d",s_content_ivalue(*cnt));
}

int fprint_int(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_int);
  fprintf(chan,"%d",s_content_ivalue(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_int64(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_int64);
  sprintf(buffer,"%Ld",s_content_llvalue(*cnt));
}

int fprint_int64(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_int64);
  fprintf(chan,"%Ld",s_content_llvalue(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_hex(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_hex);
  sprintf(buffer,"0x%X",s_content_value(*cnt));
}

int fprint_hex(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_hex);
  fprintf(chan,"0x%X",s_content_value(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_float(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_float);
  sprintf(buffer,"%g",s_content_fvalue(*cnt));
}

int fprint_float(chan,cnt)
FILE *chan;
struct s_content *cnt;
{ 
 zz_assert(cnt && s_content_tag(*cnt)==tag_float);
 fprintf(chan,"%g",s_content_fvalue(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_double(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_double);
  sprintf(buffer,"%g",s_content_dvalue(*cnt));
}

int fprint_double(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_double);
  fprintf(chan,"%g",s_content_dvalue(*cnt));
}

/*----------------------------------------------------------------------------*/

int sprint_eol(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_eol);
  strcpy(buffer,"EOL");
}

int fprint_eol(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_eol);
  fprintf(chan,"EOL");
}

/*----------------------------------------------------------------------------*/

int sprint_eof(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_eof);
  strcpy(buffer,"EOF");
}

int fprint_eof(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
  zz_assert(cnt && s_content_tag(*cnt)==tag_eof);
  fprintf(chan,"EOF");
}


/*----------------------------------------------------------------------------*/

int sprint_none(buffer,cnt)
char *buffer;
struct s_content *cnt;
{strcpy(buffer,"NONE");}

int fprint_none(chan,cnt)
FILE *chan;
struct s_content *cnt;
{fprintf(chan,"NONE");}


/*------------------------------------------------------------------------*/
/* 
   ???_cast() casting operations for numerical types.
   Returned structure is the 'tgt' param with results. 
*/
static 
struct s_content *int_cast(src, tgt_tag, tgt)
     struct s_content *src, *tgt;
     struct s_tag *tgt_tag;
{
  if (src->tag != tag_int) {
    printz("Error: int_cast(), source tag type not int: %s\n", src->tag->name);
    return 0;
  }

  if (tgt_tag == tag_int)
    s_content_ivalue(*tgt) = (int32_t)s_content_ivalue(*src);
  else if (tgt_tag == tag_int64)
    s_content_llvalue(*tgt) = (int64_t)s_content_ivalue(*src);
  else if (tgt_tag == tag_float)
    s_content_fvalue(*tgt) = (float)s_content_ivalue(*src);
  else if (tgt_tag == tag_double)
    s_content_dvalue(*tgt) = (double)s_content_ivalue(*src);
  else {
    printz("Error: int_cast(), bad type for arithmetic target: %s\n", tgt_tag->name);
    return 0;
  }

  tgt->tag == tgt_tag;
  return tgt;
}

static 
struct s_content *int64_cast(src, tgt_tag, tgt)
     struct s_content *src, *tgt;
     struct s_tag *tgt_tag;
{
  if (src->tag != tag_int64) {
    printz("Error: int64_cast(), source tag type not int64: %s\n", src->tag->name);
    return 0;
  }

  if (tgt_tag == tag_int)
    s_content_ivalue(*tgt) = (int32_t)s_content_llvalue(*src);
  else if (tgt_tag == tag_int64)
    s_content_llvalue(*tgt) = (int64_t)s_content_llvalue(*src);
  else if (tgt_tag == tag_float)
    s_content_fvalue(*tgt) = (float)s_content_llvalue(*src);
  else if (tgt_tag == tag_double)
    s_content_dvalue(*tgt) = (double)s_content_llvalue(*src);
  else {
    printz("Error: int64_cast(), bad type for arithmetic target: %s\n", tgt_tag->name);
    return 0;
  }

  tgt->tag == tgt_tag;
  return tgt;
}

static 
struct s_content *float_cast(src, tgt_tag, tgt)
     struct s_content *src, *tgt;
     struct s_tag *tgt_tag;
{
  if (src->tag != tag_float) {
    printz("Error: float_cast(), source tag type not float: %s\n", src->tag->name);
    return 0;
  }

  if (tgt_tag == tag_int)
    s_content_ivalue(*tgt) = (int32_t)s_content_fvalue(*src);
  else if (tgt_tag == tag_int64)
    s_content_llvalue(*tgt) = (int64_t)s_content_fvalue(*src);
  else if (tgt_tag == tag_float)
    s_content_fvalue(*tgt) = s_content_fvalue(*src);
  else if (tgt_tag == tag_double)
    s_content_dvalue(*tgt) = (double)s_content_fvalue(*src);
  else {
    printz("Error: float_cast(), bad type for arithmetic target: %s\n", tgt_tag->name);
    return 0;
  }

  tgt->tag == tgt_tag;
  return tgt;
}


static 
struct s_content *double_cast(src, tgt_tag, tgt)
     struct s_content *src, *tgt;
     struct s_tag *tgt_tag;
{
  if (src->tag != tag_double) {
    printz("Error: double_cast(), source tag type not double: %s\n", src->tag->name);
    return 0;
  }

  if (tgt_tag == tag_int)
    s_content_ivalue(*tgt) = (int32_t)s_content_dvalue(*src);
  else if (tgt_tag == tag_int64)
    s_content_llvalue(*tgt) = (int64_t)s_content_dvalue(*src);
  else if (tgt_tag == tag_float)
    s_content_fvalue(*tgt) = (float)s_content_dvalue(*src);
  else if (tgt_tag == tag_double)
    s_content_dvalue(*tgt) = s_content_dvalue(*src);
  else {
    printz("Error: double_cast(), bad type for arithmetic target: %s\n", tgt_tag->name);
    return 0;
  }

  tgt->tag == tgt_tag;
  return tgt;
}


static 
struct s_content *invalid_cast(src, tgt_tag, tgt)
     struct s_content *src;
     struct s_tag *tgt_tag;
     struct s_content *tgt;
{
  printf("Error - non numerical type - casting not allowed");
  return 0;
}

/*----------------------------------------------------------------------------*/

static int fprint_zlex(FILE *chan, void *cnt);
static int sprint_zlex(char *stringout, void *cnt);
static int fprint_zlex_image(FILE *chan, void *cnt);
static int sprint_zlex_image(char *stringout, void *cnt);

#if 0

/*
  there is a bug in GNU LibC 2.1.3 (RH6.2) and %z is not overloadable
 */

static int 
print_scontent (FILE *stream,
		const struct printf_info *info,
		const void *const *args)
{
  struct s_content *w;
  char buffer[4096];
  int len;
  w = *((struct s_content **) (args[0]));
  if(info->spec=='z')
    sprint_zlex(buffer, w);
  else if(info->spec=='w')
    sprint_zlex_image(buffer, w);
  else
    assert(!"should never happen");
  len = strlen(buffer);
  assert(len < sizeof(buffer));
  len = fprintf (stream, "%s",buffer);
  return len;
}
     
     
static int 
print_scontent_arginfo (const struct printf_info *info, 
			size_t n,
			int *argtypes)
{
  /* We always take exactly one argument and this is a pointer to the
     structure.. */
  assert(0);
  if (n > 0)
    argtypes[0] = PA_POINTER;
  return 1;
}
     

static void init_print_zlex()
{
  register_printf_function ('z', print_scontent, print_scontent_arginfo);
  register_printf_function ('w', print_scontent, print_scontent_arginfo);
}
#endif

void init_zlex(void)
{
if(init_zlex_done) return;
init_zlex_done=1;

printz_code('z',fprint_zlex,sprint_zlex);
printz_code('w',fprint_zlex_image,sprint_zlex_image);
/* init_print_zlex(); */

tag_int = find_tag("int");
tag_int->sprint=sprint_int;
tag_int->fprint=fprint_int;
tag_int->cast=int_cast;

tag_int64 = find_tag("int64");
tag_int64->sprint=sprint_int64;
tag_int64->fprint=fprint_int64;
tag_int64->cast=int64_cast;

tag_hex = find_tag("hex");
tag_hex->sprint=sprint_hex;
tag_hex->fprint=fprint_hex;
tag_hex->cast=invalid_cast;

tag_float = find_tag("float");
tag_float ->sprint=sprint_float ;
tag_float ->fprint=fprint_float ;
tag_float->cast=float_cast;

tag_double = find_tag("double");
tag_double ->sprint=sprint_double ;
tag_double ->fprint=fprint_double ;
tag_double->cast=double_cast;

tag_qstring = find_tag("qstring");
tag_qstring ->sprint=sprint_string ;
tag_qstring ->fprint=fprint_string ;
tag_qstring->cast=invalid_cast;

tag_ident = find_tag("ident");
tag_ident ->sprint=sprint_string ;
tag_ident ->fprint=fprint_string ;
tag_ident->cast=invalid_cast;

tag_eol = find_tag("eol");
tag_eol ->sprint=sprint_eol ;
tag_eol ->fprint=fprint_eol ;
tag_eol->cast=invalid_cast;

tag_eof = find_tag("eof");
tag_eof ->sprint=sprint_eof ;
tag_eof ->fprint=fprint_eof ;
tag_eof->cast=invalid_cast;

tag_char = find_tag("char");
tag_char ->sprint=sprint_string ;
tag_char ->fprint=fprint_string ;
tag_char->cast=invalid_cast;

tag_none = find_tag("none");
tag_none ->sprint=sprint_none ;
tag_none ->fprint=fprint_none ;
tag_none->cast=invalid_cast;

tag_address = find_tag("address");
tag_address->cast=invalid_cast;

tag_procedure = find_tag("procedure");
tag_procedure->cast=invalid_cast;

tag_qprocedure = find_tag("qprocedure");
tag_qprocedure->cast=invalid_cast;

tag_list = find_tag("list");
tag_list->sprint=sprint_list;
tag_list->fprint=fprint_list;
tag_list->cast=invalid_cast;

tag_cont = find_tag("cont");
tag_cont->cast=invalid_cast;

tag_special = find_tag("special");
tag_special->cast=invalid_cast;

tag_sint = find_tag("sint");
tag_sint->cast=invalid_cast;

tag_param = find_tag("param");
tag_param->cast=invalid_cast;

tag_bead =find_tag("bead");
tag_bead->cast=invalid_cast;

tags_initialized=1;
}

/*----------------------------------------------------------------------------*/

static int sprint_zlex(char *stringout, void *_cnt)
{
  struct s_content *cnt= (struct s_content *)_cnt;
  if(cnt->tag)
    (*cnt->tag->sprint)(stringout,cnt);
  else
    strcpy(stringout,"NULL");
  return 1;
}

/*----------------------------------------------------------------------------*/

static int fprint_zlex(FILE *chan, void *_cnt)
{
  struct s_content *cnt= (struct s_content *)_cnt;
  if(cnt->tag)
    (*cnt->tag->fprint)(chan,cnt);
  else
    fprintf(chan,"NULL");
  return 1;
}

/*----------------------------------------------------------------------------*/

static int sprint_zlex_image(char *stringout, void *_cnt)
{
  struct s_content *cnt= (struct s_content *)_cnt;
  if(cnt->tag)
    {
      if(cnt->tag==tag_eol)
	sprintf(stringout,"\n");
      else if(cnt->tag==tag_list)
	{
	  sprint_list_image(stringout,cnt);
	}
      else
	(*cnt->tag->sprint)(stringout,cnt);
    }
  return 1;
}

/*----------------------------------------------------------------------------*/

static int fprint_zlex_image(FILE *chan, void *_cnt)
{
  struct s_content *cnt= (struct s_content *)_cnt;
  if(cnt->tag)
    {
      if(cnt->tag==tag_eol)
	fprintf(chan,"\n");
      else if(cnt->tag==tag_list)
	{
	  fprint_list_image(chan,cnt);
	}
      else
	(*cnt->tag->fprint)(chan,cnt);
    }
  return 1;
}
/*----------------------------------------------------------------------------*/


zlex_set_precedence(cnt,prec,left_assoc)
struct s_content *cnt;
int prec,left_assoc;
{
char *s;
if(cnt->tag!=tag_ident && cnt->tag!=tag_char)
  {
   printz("zlex_set_precedence: unable to change prec of token /%z/\n",cnt);
   return;
  }
if(prec<0 || prec>255 || (left_assoc!=1 && left_assoc!=0))
  {
   printz("zlex_set_precedence: bad prec/assoc values (%d %d) for token /%z/\n",
	prec,left_assoc,cnt);
   return;
  }
s = (char*)s_content_value(*cnt);
s--;
if(*s!=TERM_MAGIC)
  {
   lexical_error("zlex_set_precedence: bad magic");
   return;
  }
*--s = prec;
*--s = left_assoc? LEFT_ASSOCIATIVE:RIGHT_ASSOCIATIVE;
}


/*----------------------------------------------------------------------------*/

char *zlex_strsave(name)
const char *name;
{
char *tmp;
struct s_term *term;
const char *cs;
char *s;
char *t;
struct symbol **p;
struct s_string_hd *hd;
int dsc,len;
tmp=0;
if(zlex_case_insensitive)
  {
   tmp = (char*)malloc(strlen(name)+1);
   cs = name;
   t = tmp;
   while(*cs)
     {
      if(*cs>='A' && *cs<='Z') *t= *cs+'a'-'A';
      else *t= *cs;
      cs++;
      t++;
     }
   *t='\0';
   name = tmp;
  }
/* one shot init */
if(!str_term_table)
  str_term_table = avl_tree_nodup_str(struct s_term,name);

term = avl_locate(str_term_table,name);
if(!term)
  {
   term = (struct s_term *)calloc(1,sizeof(struct s_term));
   len = strlen(name);
   s = (char*) malloc(len+1+3);
   *s++ = cur_assoc;
   *s++ = cur_prec;
   *s++ = TERM_MAGIC; 
   term->name = s;
   strcpy(term->name,name);
   zlex_strsaved_mem += sizeof(struct s_term) + len+1+3;
   str_term_n++;
   avl_insert(str_term_table,term);
  }
if(tmp) free(tmp);
return term->name;
}



/*----------------------------------------------------------------------------*/

std_len(cnt)
struct s_content *cnt;
{
  //return strlen(cnt->tag->name)+9;
  zz_assert(!"any use????");
}

/*----------------------------------------------------------------------------*/

int std_sprint(buffer,cnt)
char *buffer;
struct s_content *cnt;
{
sprintf(buffer,"%s:%08X",cnt->tag->name,s_content_value(*cnt));
}

/*----------------------------------------------------------------------------*/

int std_fprint(chan,cnt)
FILE *chan;
struct s_content *cnt;
{
fprintf(chan,"%s:%08X",cnt->tag->name,s_content_value(*cnt));
}

/*----------------------------------------------------------------------------*/

int std_copy(tar,src)
struct s_content *tar,*src;
{
*tar = *src;
}

/*----------------------------------------------------------------------------*/

static TREE *tag_tree=0;

/*----------------------------------------------------------------------------*/

static struct s_tag* create_tag(const char* name)
{
  struct s_tag *tag;
  tag = (struct s_tag*)calloc(1,sizeof(struct s_tag));
  zz_assert(tag);
  tag->name      = zlex_strsave(name);
  tag->sprint    = std_sprint;
  tag->fprint    = std_fprint;
  tag->copy      = std_copy;
  tag->len       = std_len;
  tag->delete    = 0;
  tag->cast      = invalid_cast;
  tag->param_on  = 0;
  tag->param_off = 0;
  avl_insert(tag_tree,tag);
  zlex_tag_mem+=sizeof(struct s_tag);
  return tag;
}

/*----------------------------------------------------------------------------*/

static struct s_tag* 
search_tag(const char* name)
{
  struct s_tag *tag;

  zz_assert(name);

  if(!tag_tree)
    tag_tree=avl_tree_nodup_str(struct s_tag,name);

  tag = avl_locate(tag_tree,name);
  return tag;
}

/*----------------------------------------------------------------------------*/

struct s_tag* 
find_tag(const char* name)
{
  struct s_tag *tag;

  tag = search_tag(name);
  if(!tag) 
    tag = create_tag(name);
  return tag;
}

/*----------------------------------------------------------------------------*/

int
zz_lex_add_new_tag(const char*        this_tag_name, 
		   zz_tag_sprint_cb   this_tag_sprint, 
		   zz_tag_fprint_cb   this_tag_fprint,
		   zz_tag_cdtor       this_tag_ctor,
		   zz_tag_cdtor       this_tag_dtor,
		   zz_tag_cast        this_tag_cast)
{
  struct s_tag *newtag, *oldtag;

  if(!this_tag_name) {
    zz_error(FATAL_ERROR, "%s: can't add new tag with null name\n", __FUNCTION__);
    return 0;    
  }
  oldtag = search_tag(this_tag_name);
  if(oldtag) {
    zz_error(FATAL_ERROR, "%s: can't add new tag '%s' (already exists)\n", __FUNCTION__, this_tag_name);
    return 0;
  }

  newtag = create_tag(this_tag_name);
  if(this_tag_sprint)
    newtag->sprint=this_tag_sprint;
  if(this_tag_fprint)
    newtag->fprint=this_tag_fprint;
  if(this_tag_cast)
    newtag->cast=this_tag_cast;
  if(this_tag_ctor)
    newtag->param_on=this_tag_ctor;
  if(this_tag_dtor)
    newtag->param_off=this_tag_dtor;
  return 1;
}

/*----------------------------------------------------------------------------*/

int
zz_lex_remove_tag(const char* tag_name)
{
  struct s_tag *tag;

  if(!tag_name) {
    zz_error(FATAL_ERROR, "%s: null tag\n", __FUNCTION__);
    return 0;    
  }
  tag = search_tag(tag_name);
  if(!tag) {
    zz_error(FATAL_ERROR, "%s: can't find tag '%s'\n", __FUNCTION__, tag_name);
    return 0;
  }  
  if(!avl_remove(tag_tree, tag_name)) {
    zz_error(FATAL_ERROR, "%s: can't remove tag '%s'\n", __FUNCTION__, tag_name);
    return 0;
  }
  return 1;
}

/*----------------------------------------------------------------------------*/

static void for_all_str_term(void (*action)())
{
if(str_term_table)
  avl_scan(str_term_table,action);
}

/*----------------------------------------------------------------------------*/

static int esc_tran(c)
char c;
{
switch(c)
  {
   case 'r':  return '\r';
   case 'n':  return '\n';
   case '7':  return '\7';
   case '?':  return '\033';
   case 't':  return '\t';
   case '\"': return '\"';
   case '\'': return '\'';
   case '\\': return '\\';
   default:   return c;
  }
}



/*---------------------------------------------------------------------------*/


void zlex(ptr, cnt)
char **ptr;
struct s_content *cnt;
{
double df;
char buffer[MAX_TOKEN_LENGTH+1];
int i;
if(!init_zlex_done) init_zlex();
while (**ptr==' ' || **ptr=='\t') (*ptr)++;

switch(**ptr)
  {
   /* IDENTIFICATORE */
    //   CASE_IDENT_START:
  case 'a' ... 'z': 
  case 'A' ... 'Z': 
  case '$': 
  case '_':
     i=0;
     buffer[i++] = *((*ptr) ++);
     while( i<MAX_TOKEN_LENGTH && isident(**ptr))
       buffer[i++] = *((*ptr) ++);
     if (i >= MAX_TOKEN_LENGTH)
       {
        lexical_error("Identifier too long. truncated.");
        i=MAX_TOKEN_LENGTH;
       }
     buffer[i]=0;
// Sap:     cnt->value = zlex_strsave(buffer);
     s_content_value(*cnt) = (int) zlex_strsave(buffer);
     cnt->tag   = tag_ident;
     break;

   /* COSTANTE NUMERICA HEX/INT/FLOAT/DOUBLE */
   case '0' ... '9':
     if(**ptr=='0' 
         && (*(*ptr+1)=='x' || *(*ptr+1)=='X') 
         && isxdigit(*(*ptr+2)) )
       {
        /* HEX */
        (*ptr)+=2;
        i=0;
        buffer[i++] = *((*ptr) ++);
        while( i<=16 && isxdigit(**ptr))
          buffer[i++] = *((*ptr) ++);
	buffer[i]='\0';
	if(**ptr=='l' || zlex_intconst_int64) {
	  if (i > 16) {
	    lexical_error("too long hex constant truncated /%s/",buffer);
	    buffer[16]='\0';
	  }
	  if(**ptr=='l') (*ptr) ++;
	  sscanf(buffer,"%Lx",&(s_content_llvalue(*cnt)));
	  cnt->tag   = tag_int64;
	} else if (**ptr=='i' || !zlex_intconst_int64) {
	  if (i > 8) {
	    lexical_error("too long hex constant truncated /%s/",buffer);
	    buffer[8]='\0';
	  }
	  if(**ptr=='i') (*ptr) ++;
	  sscanf(buffer,"%x",&(s_content_value(*cnt)));
	  cnt->tag   = tag_int;
	}
       }
     else
       {
        /* INT/FLOAT/DOUBLE */
	 i=0;
	 while( i<255 && isdigit(**ptr))
	   buffer[i++] = *((*ptr) ++);

	 if(**ptr=='.')
	   {
	     /* FLOAT||DOUBLE */
	     buffer[i++] = *((*ptr) ++);
	     while( i<39 && isdigit(**ptr))
	       buffer[i++] = *((*ptr) ++);
	     
	     if ((**ptr=='e' || **ptr=='E') &&
		 ( (*(*ptr+1)=='-' && isdigit(*(*ptr+2)) ) ||
		   (*(*ptr+1)=='+' && isdigit(*(*ptr+2)) ) ||
		   isdigit(*(*ptr+1))
		   )
		 )
	       {
		 buffer[i++] = *((*ptr) ++);
		 
		 if(**ptr=='-' || **ptr=='+')
		   buffer[i++] = *((*ptr) ++);
		 
		 while(isdigit(**ptr))
		   buffer[i++] = *((*ptr) ++);
	       }
	     
	   zz_assert(i<MAX_TOKEN_LENGTH);
           buffer[i]='\0';
           sscanf(buffer,"%lf",&df);
	   if(**ptr=='d' || (zlex_realconst_double && (**ptr!='f'))) {
	     if(**ptr=='d') (*ptr) ++;
	     s_content_dvalue(*cnt) = df;
	     cnt->tag   = tag_double;
	   } else if(**ptr=='f' || !zlex_realconst_double) {
	     if(**ptr=='f') (*ptr) ++;
	     s_content_fvalue(*cnt) = (float)df;
	     cnt->tag   = tag_float;
	   } else {
	     zz_assert(!"should never happen");
	   }
          }
        else
          {
           /* INT */
           buffer[i]='\0';
	   if(**ptr=='l' || (zlex_intconst_int64 && **ptr!='i')) {
	     if (i > 30) {
	       lexical_error("too long integer constant /%s/ truncated",buffer);
	       buffer[30]='\0';
	     }
	     if(**ptr=='l') (*ptr) ++;
	     sscanf(buffer,"%Li",&(s_content_llvalue(*cnt)));
	     zz_trace("got int64 = (%s=%Li)\n", buffer, s_content_llvalue(*cnt));
	     cnt->tag   = tag_int64;
	   } else if (**ptr=='i' || !zlex_intconst_int64){
	     int scret=0;
	     if (i > (10+(((**ptr=='-') || (**ptr=='+'))?1:0))) {
	       lexical_error("too long integer constant /%s/ truncated",buffer);
	       buffer[10]='\0';
	     }
	     if(**ptr=='i') (*ptr) ++;
	     scret=sscanf(buffer,"%d",&(s_content_value(*cnt)));
	     cnt->tag   = tag_int;
	     if(scret!=1) {
	       lexical_error("can't parse integer constant /%s/", buffer);
	     }
	   } else {
	     zz_assert(!"should never happen");
	   }
          }
       }
     break;

  /* END OF FILE */
  case '\255':
    zz_trace("got EOF\n");
    cnt->tag = tag_eof;
    s_content_value(*cnt)=0;
    break;

  /* END OF LINE */
  case '\0':
    zz_trace("got EOL\n");
#if 1
    if(zlex_parse_eol) {
      cnt->tag = tag_eol;
    }else{
      cnt->tag = tag_cont;
      *ptr = 0;
      //(*ptr)++;
    }
#else
    cnt->tag = tag_eol;
#endif
    s_content_value(*cnt)=0;
    break;

  case '\n':
    zz_trace("got linefeed\n");
    (*ptr)++;
#if 1
    if(zlex_parse_eol) {
      cnt->tag = tag_eol;
    }else{
      cnt->tag = tag_cont;
    }
#else
    cnt->tag = tag_eol;
#endif
    s_content_value(*cnt)=0;
    break;

  /* ESCAPED SEQUENCE */
/*
  case '\\':
    (*ptr)++;
    buffer[0] = esc_tran(*(*ptr)++);
    buffer[1] = '\0';
    cnt->value= (long)zlex_strsave(buffer);
    cnt->tag=tag_char;
    break;
*/
     
  /* QSTRING */
  case '\"':
    i=0;
    (*ptr) ++; /* schippo il primo " */
    while(i<MAX_TOKEN_LENGTH && **ptr && **ptr != '\"')
      {
       if (**ptr == '\\')
         {
          (*ptr)++;
          buffer[i++] = esc_tran(*(*ptr)++);
	 }
       else
         buffer[i++] = *(*ptr)++;
      }
    buffer[i]='\0';
    if(!**ptr || i>=MAX_TOKEN_LENGTH)
       lexical_error("qstring not terminated or too long /%s/",buffer);
    if(**ptr=='\"') (*ptr) ++;
// Sap:    cnt->value = (char*)malloc(strlen(buffer)+1);

    //s_content_value(*cnt) = (int)malloc(strlen(buffer)+1);
    //strcpy((char*)s_content_value(*cnt),buffer);
    s_content_svalue(*cnt) = (char*)strdup(buffer);
    zz_assert(s_content_svalue(*cnt));

    cnt->tag=tag_qstring;
    zlex_qstring_mem+=strlen(buffer)+1;
    break;

  /* COMMENTI */
  case '!':
    zz_trace("got !\n");
    if (*((*ptr)+1)=='!')
      {
       while(*(*ptr)++);
       s_content_value(*cnt)=0;
       cnt->tag=tag_eol;
      }
    else
      {
       (*ptr)++;
// Sap:       cnt->value= zlex_strsave("!");
       s_content_value(*cnt)= (int)zlex_strsave("!");
       cnt->tag=tag_char;
      }
    break;

  /* COMMENTI C++ STYLE*/
  case '/':
    if (*((*ptr)+1)=='/')
      {
       while(*(*ptr)++);
       s_content_value(*cnt)=0;
       cnt->tag=tag_eol;
      }
    else
      {
       (*ptr)++;
// Sap:       cnt->value= zlex_strsave("/");
       s_content_value(*cnt)= (int)zlex_strsave("/");
       cnt->tag=tag_char;
      }
    break;

  /* CONT. MARK */
  case '.':
    if (zlex_parse_eol && (*((*ptr)+1)=='.'&& *((*ptr)+2)=='.'))
      {
       (*ptr)+=3;
       while(**ptr) (*ptr)++;
       s_content_value(*cnt)=0;
       cnt->tag=tag_cont;
      }
    else
      {
       (*ptr)++;
// Sap:       cnt->value = zlex_strsave(".");
       s_content_svalue(*cnt) =  zlex_strsave(".");
       cnt->tag=tag_char;
      }
    break;
  default:
    buffer[0]= *(*ptr)++;
    buffer[1]= '\0';
// Sap:    cnt->value = zlex_strsave(buffer);
    s_content_svalue(*cnt) = zlex_strsave(buffer);
    cnt->tag=tag_char;
  }
 zz_trace("token is '%z'\n", cnt); 
} /* end zlex() */


/*----------------------------------------------------------------------------*/

show_zlex_memory()
{
PRINTMEM("zlex.qstring.mex",zlex_qstring_mem)
PRINTMEM("zlex.strsaved.mem",zlex_strsaved_mem)
PRINTMEM("zlex.tag.mem",zlex_tag_mem)
}

/*----------------------------------------------------------------------------*/

int zlex_set_case_sensitive(cs)
     int cs;
{
  zlex_case_insensitive=!cs;
  return 1;
}

int zlex_set_parse_eol(peol)
     int peol;
{
  zz_trace("zlex_set_parse_eol(%d)\n", peol);
  zlex_parse_eol=peol;
  return 1;
}

int zlex_set_default_real_as_double(peol)
     int peol;
{
  zz_trace("zlex_set_default_real_as_double(%d)\n", peol);
  zlex_realconst_double=peol;
  return 1;
}

int zlex_set_default_integer_as_int64(peol)
     int peol;
{
  zz_trace("zlex_set_default_real_as_double(%d)\n", peol);
  zlex_intconst_int64=peol;
  return 1;
}

/*------------------------------------------------------------------------*/


ignore_block()
{
ignore_block_flag = 1;
braket_level = 0;
}


/*---------------------------------------------------------------------------*/

static char rcsid[] = "$Id: zlex.c,v 1.34 2002/06/03 11:06:13 kibun Exp $ ";
