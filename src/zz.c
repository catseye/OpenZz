/* 
    Zz Dynamic Parser Library
    Copyright (C) 1989 - I.N.F.N - S.Cabasino, P.S.Paolucci, G.M.Todesco

    As opposed to the majority of the files in the Zz library,
    this file is released under the GPL.

    This file(zz.c) is free software; you can redistribute it
    and/or modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version 2, or
    (at your option) any later version.

    This file and the Zz library are distributed in the hope that they
    will be useful, but WITHOUT ANY WARRANTY; without even the implied 
    warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
    See the GNU Lesser General Public License for more details.

    The GNU General Public License is available online or
    can be requested from the Free Software Foundation,
    59 Temple Place, Suite 330, Boston, MA 02111 USA. 
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "zz.h"

/*PROTOYPES*/
void print_error_count(void);
void print_usage(void);

int main(argc,argv,env)
     int argc;
     char *argv[];
     char ** env;
{
  int pipe_flag;
  char *filein,*fileout,*def_incl_file;
  char *s;
  char *include_dir;
  int verbose;
  int i,arg_n;
  int ret;
  int debug_flag;
  /* /k/kit/lib/ */

  /* DEFAULT VALUES */
  pipe_flag=0;
  debug_flag=0;
  arg_n=0;
  filein=0;
  fileout=0;
  verbose=0;
  zz_set_output_stream(stdout);

  /* READ COMMAND LINE */
  for(i=1;i<argc;i++)
    {
      if(argv[i][0]=='-') 
	{
	  s=argv[i]+1;
	  if(!*s) {
	    verbose=0;pipe_flag=1;
	  }
	  else while(*s)
	    {
	      switch(*s++)
		{
		case 'h':
		  print_usage();
		  exit(0);
		  break;
		case 'v':
		  verbose=1;
		  break;
		case 'q':
		  verbose=0;
		  break;
		case 'p':
		  pipe_flag=1;
		  break;
		case 'd':
		  debug_flag=1;
		  break;
		default: printf("Unrecognized qualifier -%c\n",*s);
		}
	    }
	}
      else
	{
	  switch(arg_n++)
	    {
	    case 0: filein=argv[i];break;
	    case 1: fileout=argv[i];break;
	    }
	}
    }
  if(arg_n>2)
    {fprintf(stderr, "error: too many arguments\n");print_usage();exit(1);}

  if(verbose)
    {
      fprintf(stderr, "OpenZz "VERSION"\n");

      if(filein) 
	{
	  fprintf(stderr, "input: %s\n",filein);
	  if(fileout) fprintf(stderr, "output: %s\n",fileout);
	}
      else fprintf(stderr, "interactive session\n");
    }

  if(debug_flag==1)
    zz_set_trace_mask(TRACE_ALL);

  /* LAUNCH ZZ */
  zz_init();


  if(fileout)
    zz_set_output(fileout);
  else
    zz_set_output(0);

  zz_set_default_extension("zz");
  include_dir = getenv("ZZ_INCLUDES");

  if(!include_dir)
    include_dir = "./";
  else
    if(verbose) {
      fprintf(stderr, "Including files from %s\n",include_dir);
    }

  zz_set_default_include_dir(include_dir);

  if(def_incl_file=getenv("ZZ_DEFAULT_INCLUDE_FILE")) {
    if(verbose)
      fprintf(stderr, "Including default file: %s\n",def_incl_file);

    ret=zz_parse_file(def_incl_file);
  }

  if(pipe_flag)
    ret=zz_parse_pipe();
  else if(!filein)
    ret=zz_parse_tt();
  else
    ret=zz_parse_file(filein);

  if(verbose)
    fprintf(stderr, "bye (ret=%d)\n",ret);

  /* EXIT */
  if(ret) 
    exit(EXIT_SUCCESS);
  else {
    print_error_count();
    exit(EXIT_FAILURE);
  }
}              /* main() */


/*---------------------------------------------------------------------*/


void print_usage(void) {
  fprintf(stderr, 
	  "usage: ozz [params] [filein [fileout]]\n"
	  "  -q        quiet\n"
	  "  -v        verbose\n"
	  "  -p        get input from stdin\n"
	  "  -d        turns on debug prints\n");
}

