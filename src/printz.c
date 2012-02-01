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

	FILE: PRINTZ.C

@	int fprintz (FILE *file_ptr, const char *format_spec, ...);
	int printz  (const char *format_spec, ...);
	int sprintz (char *str, const char *format_spec, ...);
	int printz_aux(FILE *chan);
	int printz_code(char code,int (*fprint_proc),int (*sprint_proc));
	
@       fprintz(),printz(),sprintz() funzionano come fprintf(),...
        
@	fprintz_aux(chan); abilita la scrittura su un file ausiliario da parte
	di fprintz() 

@	printz_aux(chan); abilita la scrittura su un file ausiliario da parte
	di printz() 
	
@	printz_code('z',fprint_z,sprint_z); aggiunge la gestione del
	formato 'z'; le procedure fprint_z e sprint_z devono avere i
	seguenti parametri:

	fprint_z(FILE *chanout, void *argument, char* fmt)
	sprint_z(char *stringout, void *argument, char* fmt)

	fmt contiene gli eventuali modificatori di formato presenti 
	(es.: "%10.3lz")

*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#define PRINTZ
#include "printz.h"

#ifdef PRINTZ_USE_STDARG
#include <stdarg.h>
#else
#include <varargs.h>
#endif

/*----------------------------------------------------------------------------*/
static FILE *printz_aux_chan=0,*fprintz_aux_chan=0;

#define T_LONG 		1
#define T_DOUBLE 	2
#define T_POINTER 	3

#define MAX_PRINTZ_TYPE 100

static struct {
  char code,type;
  int (*fprint_routine)(FILE *chan, ...);
  int (*sprint_routine)(char buffer[], ...);
}
printz_table[MAX_PRINTZ_TYPE+1];
static int printz_table_n=0;


#ifdef PRINTZ_USE_PROTOTYPES
int do_printz(FILE *chanout,FILE *chanout_1,
              char *stringout,char *fmt,va_list *ap);
#else
int do_printz();
#endif


/*---------------------------------------------------------------------------*/


int printz_code(int code,int (*fprint_routine)(FILE *chan,...),int (*sprint_routine)(char buffer[], ...))
{
  int i;
  if(printz_table_n<MAX_PRINTZ_TYPE)
    {
      i=printz_table_n++;
      printz_table[i].code = code;
      printz_table[i].type = T_POINTER;
      printz_table[i].fprint_routine = fprint_routine;
      printz_table[i].sprint_routine = sprint_routine;
      return 1;
    }
  else
    {
      fprintf(stderr,"add printz type: too many printz type\n");
      return 0;
    }
}


/*---------------------------------------------------------------------------*/


int printz_aux(chan)
     FILE *chan;
{
  printz_aux_chan = chan;
  return 1;
}


/*---------------------------------------------------------------------------*/


int fprintz_aux(chan)
     FILE *chan;
{
  fprintz_aux_chan = chan;
  return 1;
}


/*---------------------------------------------------------------------------*/
#ifdef PRINTZ_USE_STDARG
/*---------------------------------------------------------------------------*/


int sprintz(char *out, char *fmt, ...)
{
  int ret=0;
  va_list ap;
  va_start(ap,fmt);
  if(out && fmt) ret=do_printz(0,0,out,fmt,&ap);   
  va_end(ap);
  return ret;
}


/*---------------------------------------------------------------------------*/


int fprintz(FILE *chan, char *fmt, ...)
{
  FILE *chan_1;
  int ret=0;
  va_list ap;
  va_start(ap,fmt);
  chan_1 = chan==stdout?printz_aux_chan:fprintz_aux_chan;
  if(chan && fmt) ret=do_printz(chan,chan_1,0, fmt,&ap);   
  va_end(ap);
  fflush(chan);
  if(chan_1) fflush(chan_1);
  return ret;
}


/*---------------------------------------------------------------------------*/


int printz(char *fmt,...)
{
  int ret=0;
  va_list ap;
  FILE *chan;
  va_start(ap,fmt);
  chan = stdout;
  if(fmt)ret=do_printz(chan,printz_aux_chan,0, fmt,&ap);   
  va_end(ap);
  return ret;
}


/*---------------------------------------------------------------------------*/
#else
/*---------------------------------------------------------------------------*/


int sprintz(va_alist)
     va_dcl
{
  char *out,*fmt;
  int ret=0;
  va_list ap;
  va_start(ap);
  out = va_arg(ap,char*);
  fmt = va_arg(ap,char*);
  if(out && fmt)ret=do_printz(0,0,out,fmt,&ap);   
  va_end(ap);
  return ret;
}

/*---------------------------------------------------------------------------*/


int fprintz(va_alist)
     va_dcl
{
  FILE *chan,*chan_1;
  char *fmt;
  int ret=0;
  va_list ap;
  va_start(ap);
  chan = va_arg(ap,FILE*);
  fmt = va_arg(ap,char*);
  chan_1 = chan==stdout?printz_aux_chan:fprintz_aux_chan;
  if(chan && fmt) ret=do_printz(chan,chan_1,0, fmt,&ap);   
  va_end(ap);
  fflush(chan);
  if(chan_1) fflush(chan_1);
  return ret;
}


/*---------------------------------------------------------------------------*/


int printz(va_alist)
     va_dcl
{
  char *fmt;
  int ret=0;
  va_list ap;
  FILE *chan;
  va_start(ap);
  fmt = va_arg(ap,char*);
  chan = stdout;
  if(fmt)ret=do_printz(chan,printz_aux_chan,0, fmt,&ap);   
  va_end(ap);
  return ret;
}


/*---------------------------------------------------------------------------*/
#endif
/*---------------------------------------------------------------------------*/



#define MAX_S_FMT 40
#define MAX_BUFFER 1024
#define OUTPUT(S)\
   if(stringout)\
     {\
      strcpy(stringout,(S));\
      stringout+=strlen(S);\
     }\
   else\
     {\
      fwrite((S),sizeof(char),strlen(S),chanout); \
      if(chanout_1)\
        fwrite((S),sizeof(char),strlen(S),chanout_1);    \
     }
	

/*---------------------------------------------------------------------------*/


int do_printz(FILE *chanout,FILE *chanout_1,char *stringout,char *fmt,va_list *ap)
{
  char sf,*s;
  char buffer[MAX_BUFFER+1],tmp[MAX_S_FMT+1];
  int i,j;
  void *argp;
  while(*fmt)
    { 
      i=0;
      while(*fmt && i<MAX_BUFFER && *fmt!='%')
	{
	  if(*fmt=='\\')
	    {
	      fmt++;
	      if(*fmt>='0' && *fmt<='9')
		{
		  j=0;do {tmp[j++]= *fmt++;} while(j<3 && *fmt>='0' && *fmt<='9');
		  tmp[j]='\0';
		  buffer[i++] = atoi(tmp);
		}
	      else
		{
		  switch(*fmt)
		    {
		    case '\0':break;
		    case 'n':buffer[i++]='\n';break;
		    case 't':buffer[i++]='\t';break;
		    default:buffer[i++]= *fmt;
		    }
		  fmt++;
		}
	    }
	  else
	    {
	      buffer[i++] = *fmt++;
	    }
	}
      buffer[i]='\0';
      OUTPUT(buffer)

	if(*fmt=='%')
	  {
	    tmp[0]= *fmt++;
	    i=1;
	    sf = 0;
	    if(i<MAX_S_FMT  && *fmt=='-') tmp[i++] = *fmt++;
	    while(i<MAX_S_FMT && *fmt>='0' && *fmt<='9') tmp[i++] = *fmt++;
	    if(i<MAX_S_FMT  && *fmt=='.') tmp[i++] = *fmt++;
	    while(i<MAX_S_FMT  && *fmt>='0' && *fmt<='9') tmp[i++] = *fmt++;
	    if(i<MAX_S_FMT  && (*fmt=='l' || *fmt=='L')) tmp[i++] = *fmt++;
	    if(i<MAX_S_FMT  && *fmt) tmp[i++] = sf = *fmt++;
	    tmp[i]='\0';
	    if(sf>='A' && sf<='Z') sf+='a'-'A';
	    switch(sf)
	      {
	      case 's':
		s = va_arg(*ap,char*);
		sprintf(buffer,tmp,s?s:"<nil>");
		OUTPUT(buffer)
		  break;
	      case 'd':
	      case 'o':
	      case 'x':
	      case 'c':
	      case 'u':
		sprintf(buffer,tmp,va_arg(*ap,long));
		OUTPUT(buffer)
		  break;
	      case 'e':
	      case 'f':
	      case 'g':
		sprintf(buffer,tmp,va_arg(*ap,double));
		OUTPUT(buffer)
		  break;
	      case '%':
		OUTPUT("%")
		  break;
	      default:
		for(i=printz_table_n-1;i>=0 && printz_table[i].code!=sf;i--);
		if(i>=0) argp = va_arg(*ap,void*);
		if(stringout)
		  {              
		    if(i<0 || !printz_table[i].sprint_routine)
		      {
			strcpy(stringout,tmp);
			stringout+=strlen(tmp);
		      }
		    else
		      {
			(*printz_table[i].sprint_routine)(stringout,argp,tmp);
			while(*stringout)stringout++;
		      }                 
		  }
		else
		  {
		    if(i<0 || !printz_table[i].fprint_routine)
		      {
			fwrite(tmp,sizeof(char),strlen(tmp),chanout);
			if(chanout_1)
			  fwrite(tmp,sizeof(char),strlen(tmp),chanout_1);
		      }
		    else
		      {
			(*printz_table[i].fprint_routine)(chanout,argp,tmp);
			if(chanout_1)
			  (*printz_table[i].fprint_routine)(chanout_1,argp,tmp);
		      }                               
		  }
	      }
	  }
    }
  return 1;
}

 
static char rcsid[] = "$Id: printz.c,v 1.2 2002/01/11 11:52:02 brooks Exp $";
