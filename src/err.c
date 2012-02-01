/* 
    Zz Dynamic Parser Library
    Copyright (C) 1989 - I.N.F.N - S.Cabasino, P.S.Paolucci, G.M.Todesco

    The Zz Library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This Zz Library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


/*============================================================================

 ERR.C

 Management of the errors (cfr. source.c)


==============================================================================*/


#include <stdio.h>
#include <stdint.h>
#include "err.h"
#include "zlex.h"
#include "printz.h"

#define PRINTZ_USE_STDARG

#ifdef PRINTZ_USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

char err_file[256] = "LOG.ERR";

int max_error_n=10;
static FILE *err_chan = 0;
static int 
	info_n=0,
	warning_n=0,
	error_n=0,
	fatal_error_n=0,
	lexical_error_n=0,
	internal_error_n=0,
	unknown_error_n=0,
	total_error_n=0;
extern struct s_content cur_token;


/*---------------------------------------------------------------------------*/

open_err_file()
{
char filename[132];
static int err_file_flag=0;
if(!err_file_flag)
  {
   get_source_file(err_file);
   change_filetype(err_file,".err");
   err_file_flag=1;
   err_chan = fopen(err_file,"w");
   if(!err_chan) 
     {
      fprintf(stderr,"**** ERROR: unable to open error file %s ****\n",
        err_file);
     } 
  }
}


/*----------------------------------------------------------------------------*/
#ifdef PRINTZ_USE_STDARG
/*----------------------------------------------------------------------------*/


int errprintf(char *fmt,...)
{
va_list ap;
open_err_file();
va_start(ap,fmt);
do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
return 1;
}

/*----------------------------------------------------------------------------*/

int zz_error(long type, char *fmt,...)
{
int ret=0;
va_list ap;
error_head(type);
va_start(ap,fmt);
ret=do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
error_tail();
return ret;
}

int zz_error_1(long type, char *fmt,...)
{
int ret=0;
va_list ap;
error_head(type);
va_start(ap,fmt);
ret=do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
error_tail_1();
return ret;
}

/*----------------------------------------------------------------------------*/
#else
/*----------------------------------------------------------------------------*/


int errprintf(va_alist)
va_dcl
{
char *fmt;
int ret=0;
va_list ap;
open_err_file();
va_start(ap);
fmt=va_arg(ap,char*);
ret=do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
return ret;
}

/*----------------------------------------------------------------------------*/

/*

int error(va_alist)
va_dcl
{
char *fmt;
int type,ret=0;
va_list ap;
va_start(ap);
type=va_arg(ap,long);
error_head(type);
fmt=va_arg(ap,char*);
ret=do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
error_tail();
return ret;
}

int error_1(va_alist)
va_dcl
{
char *fmt;
int type,ret=0;
va_list ap;
va_start(ap);
type=va_arg(ap,long);
error_head(type);
fmt=va_arg(ap,char*);
ret=do_printz(stderr,err_chan,0,fmt,&ap);   
va_end(ap);
error_tail_1();
return ret;
}

*/

#endif

/*---------------------------------------------------------------------------*/

error_head(type)
int type;
{
open_err_file();
fprintz(stderr,"+ **** ");
if(err_chan) fprintz(err_chan,"+ **** ");
switch(type)
  {
   case INFO:
     info_n++;
     total_error_n++;
     fprintz(stderr,"info: ");
     if(err_chan) fprintz(err_chan,"info: ");
     break;
   case WARNING:
     warning_n++;
     total_error_n++;
     fprintz(stderr,"warning: ");
     if(err_chan) fprintz(err_chan,"warning: ");
     break;
   case ERROR:
     error_n++;
     total_error_n++;
     fprintz(stderr,"ERROR: ");
     if(err_chan) fprintz(err_chan,"ERROR: ");
     break;
   case FATAL_ERROR:
     fatal_error_n++;
     total_error_n++;
     fprintz(stderr,"FATAL ERROR: ");
     if(err_chan) fprintz(err_chan,"FATAL ERROR: ");
     break;
   case LEXICAL_ERROR:
     lexical_error_n++;
     total_error_n++;
     fprintz(stderr,"LEXICAL ERROR: ");
     if(err_chan) fprintz(err_chan,"LEXICAL ERROR: ");
     break;
   case INTERNAL_ERROR:
     internal_error_n++;
     total_error_n++;
     fprintz(stderr,"INTERNAL ERROR: ");
     if(err_chan) fprintz(err_chan,"INTERNAL ERROR: ");
     break;
   default:
     unknown_error_n++;
     total_error_n++;
     fprintz(stderr,"GENERIC ERROR: ");
     if(err_chan) fprintz(err_chan,"GENERIC ERROR: ");
  }
}

/*---------------------------------------------------------------------------*/

error_tail()
{
fprintz(stderr," ****\n");
if(err_chan) fprintz(err_chan," ****\n");
fprint_source_position(stderr,0);
fprint_param(stderr);
if(err_chan)
  {
   fprint_source_position(err_chan,0);
   fprint_param(err_chan);
  }
check_error_max_number();
}


/*---------------------------------------------------------------------------*/

error_tail_1()
{
fprintz(stderr," ****\n");
if(err_chan) fprintz(err_chan," ****\n");
fprint_source_position(stderr,0);
fprint_param(stderr);
if(err_chan)
  {
   fprint_source_position(err_chan,0);
   fprint_param(err_chan);
  }
check_error_max_number();
}


/*---------------------------------------------------------------------------*/

error_token(cnt)
struct s_content *cnt;
{
fprintz(stderr,"%z ",cnt);
if(err_chan) fprintz(err_chan,"%z ",cnt);
}


/*---------------------------------------------------------------------------*/


print_error_count()
{
if(total_error_n)
  {
   if(info_n) printf("%d info(s) ",info_n);
   if(warning_n) printf("%d warning(s) ",warning_n);
   if(lexical_error_n) printf("%d lexical error(s) ",lexical_error_n);
   if(error_n) printf("%d error(s) ",error_n);
   if(fatal_error_n) printf("%d fatal error(s) ",fatal_error_n);
   if(internal_error_n) printf("%d internal error(s) ",internal_error_n);
   if(unknown_error_n) printf("%d ??? ",unknown_error_n);
   printf("\n");
   printf("listed in %s\n",err_file);
  }
}

/*---------------------------------------------------------------------------*/


get_error_number()
{
return lexical_error_n+error_n+fatal_error_n+unknown_error_n+internal_error_n;
}

/*---------------------------------------------------------------------------*/

syntax_error(info_routine)
int (*info_routine)();
{
open_err_file();
fprintz(stderr,"+ **** SYNTAX ERROR ****\n");
if(err_chan) fprintz(err_chan,"+ **** SYNTAX ERROR ****\n");
error_n++;
total_error_n++;
if(info_routine) (*info_routine)();
fprint_source_position(stderr,1);
fprint_param(stderr);
if(err_chan)
  {
   fprint_source_position(err_chan,1);
   fprint_param(err_chan);
  }
check_error_max_number();
}


/*---------------------------------------------------------------------------*/

check_error_max_number()
{
static int count=0;
char *s;
if(++count>=max_error_n)
  {
   s = "+ **** Too many errors. compilation aborted ****\n";
   fprintz(stderr,s);
   if(err_chan) fprintz(err_chan,s);
   fprint_source_position(stderr,1);
   if(err_chan) fprint_source_position(err_chan,1);
   print_error_count();
   exit(1);
  }
}

set_max_error_n(argc,argv,ret)
int argc;
struct s_content argv[],*ret;
{
  int n;
  if(argc!=1) return;
  n=(int)s_content_value(argv[0]);
  max_error_n=n;
}

int zz_get_error_number()
{
  return total_error_n;
}

static char rcsid[] = "$Id: err.c,v 1.7 2002/06/03 11:06:13 kibun Exp $ ";
