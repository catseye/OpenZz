/* 
    Zz Dynamic Parser Library
    Copyright (C) 1989 - I.N.F.N - S.Cabasino, P.S.Paolucci, G.M.Todesco

    As opposed to the majority of the files in the Zz library,
    this file is released under the GPL.

    This file(zzi.c) is free software; you can redistribute it
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

#ifdef USE_READLINE
  #include <readline/readline.h>
  #include <readline/history.h>
#endif

#include "zz.h"
#include "zlex.h"
#include "trace.h"
#include "source.h"

void next_token_tt(struct s_source *src);

/*--------------------------------------------------------------------*/

/**
 * Initialization function for using zz with interactive tty interface:
 */
source_tt()
{
  struct s_source *src = new_source(next_token_tt);

  src->type = SOURCE_TT;
  src->src.tt.prompt = zz_get_prompt();
  src->src.tt.old = src->src.tt.s = 0;
  src->src.tt.row[0]='\0';

  return 1;
}


/*--------------------------------------------------------------------*/



zz_parse_tt()
{
  /* really necessary... hope it doesn't come first in the calling sequence..
     if(!zz_chanout)
     zz_set_output(0);
  */
  source_tt();

  return parse(find_nt("root"));
}




/*---------------------------------------------------------------------*/


void next_token_tt(cur_source)
     struct s_source *cur_source;
{
  static char *line_read = (char *)NULL;       // Tmp ptr for gnu libreadline
  char *s;

#ifndef USE_READLINE
  line_read = malloc(300);
#endif

  if(!cur_source->src.tt.s) {                  /* NEED TO READ A NEW LINE OF DATA */
    zz_trace("reading new line...\n");
   
    cur_source->src.tt.prompt = ".. ";

    if(find_prompt_proc)
      (*find_prompt_proc)(&(cur_source->src.tt.prompt));

    s = cur_source->src.tt.row;

#ifdef USE_READLINE
    // Read a line from tty input using gnu libreadline
    line_read = readline(cur_source->src.tt.prompt);
#else
    (void)fgets(line_read, 250, stdin);
#endif

    // If some non-empty input was read, store it in the history
    if (line_read) {
      if (*line_read) {

	if (strlen(line_read) >= MAX_INPUT_LINE_LENGTH) {
	  printf("ERROR: Input line (len=%i) exceeded max length, truncated at %i(max) chars.\n", 
		 strlen(line_read), MAX_INPUT_LINE_LENGTH);
	  line_read[MAX_INPUT_LINE_LENGTH]='\0';
	}

#ifdef USE_READLINE
	add_history (line_read);
#endif

	if (strlen(line_read) >= 250) {
	  exit(0);
	}

	// Doesn't seem to be used anywhere -
	// It would force exposure of 'source_sp' so I don't like it either.
	// if(source_line_routine && source_sp==1)
	//  (*source_line_routine)(s, cur_source->line_n, "stdin", 8);

	strcpy(cur_source->src.tt.row, line_read);
      }
      else {
	// Have read a blank line - set a default value
	strcpy(cur_source->src.tt.row, "");
      }

      cur_source->line_n ++;

      cur_source->src.tt.old = cur_source->src.tt.s = cur_source->src.tt.row;
    
      zlex(&(cur_source->src.tt.s), &cur_token);

      // Free the memory used in the temporary line buffer
      free (line_read);
      line_read = (char *)NULL;
    }
    else {                        /* NULL received from readline - signifies EOF */
      cur_source->eof = 1;
      cur_token.tag = tag_eof;
    }
  }
  else  {                         /* HAVE DATA - DO NOT NEED TO READ A NEW LINE */
    cur_source->src.tt.old = cur_source->src.tt.s;
    zlex(&(cur_source->src.tt.s),&cur_token);
  }

  if(cur_token.tag==tag_eol) {
    cur_source->src.tt.s=0;
    zz_trace("tag_eol... s=0\n");
  }
}


/*--------------------------------------------------------------------*/

static char rcsid[] = "$Id: zzi.c,v 1.8 2002/06/03 11:06:13 kibun Exp $ ";
