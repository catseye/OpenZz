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


#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//--------------------------------------------------------------------
//  Modified version of defopen for opening INPUT files
//  
//  Tries to open an input-file with name derived from *infile 
//  and extension *ext. The leading '.' in ext is optional. 
//  NULL is returned if no file can be opend.
//  
//  The name of the searched file is derived as follows:
//  If the basename of *infile (i.e. *infile without path) 
//  contains no '.', *ext is appended as extension. Otherwise,
//  if the basename of *infile contains a '.', only the file
//  with name identical to the basename is opened (i.e. the 
//  default extension *ext is ignored).
//
//  The file is searched in the following directories:
//  If *infile contains a path specification (i.e. a '/'), 
//  the file is only searched in the specified path.
//  Otherwise, if *infile contains no path specification,
//  all directories in the environment variable *ext are
//  searched. The variable is interpreted as a colon (':') 
//  separated list. The current working directory is automatically
//  appended to the list in *env (note: './' should be prepended
//  to the variable in *env only when the local files should be 
//  considered really before the ones in the other *env
//  directories)
//
//  hs 27/4/98
//
//  $Id: defopen.c,v 1.3 2002/01/11 11:52:02 brooks Exp $
//--------------------------------------------------------------------

FILE *defin(char *infile, char *ext, char *env, char *mode/*="r"*/) {
#define MAXLENG 250
  FILE *fp;
  char *sp;
  int i, l, p_name, p_dot, p_dir;
  char name[MAXLENG], dirs[MAXLENG], buffer[MAXLENG];

  // search infile for basename and extension
  p_name = 0;
  p_dot  = -1;

  for( i=0; infile[i] != '\0'; i++) {           
    if(infile[i]=='/') p_name = i+1;
    if(infile[i]=='.') p_dot = i;
  }

  strcpy(name,&infile[p_name]);

  if ( p_dot < p_name ) {    // infile contains no extension
    if ( *ext != '.' && strlen(ext) > 0 ) strcat(name,".");
    strcat(name,ext);
  }

  if ( p_name > 0 ) {        // infile contains a path specification
    strcpy(dirs,infile);
    dirs[p_name] = 0;
  }
  else 
    if( (sp=getenv(env))!=NULL ) 
      {
	strcpy(dirs,sp);
	strcat(dirs,":.");    // append '.' to directories
      }
    else                      // default directory '.'
      strcpy(dirs,".");
  
  
  l=strlen(dirs);
  for (p_dir=0, i=0 ; i<=l ; i++ ) {
    if (dirs[i]==':' || dirs[i]=='\0')	{
      if ( dirs[i-1] == '/' ) dirs[i-1] = 0;
      else dirs[i]=0;

      sprintf(buffer, "%s/%s", &dirs[p_dir], name); 
      // printf("try %s: \n",buffer);
      if ( (fp=fopen(buffer,mode))!=NULL ) return (fp);
      
      p_dir=i+1;
    }
  }
  return NULL;
}

//--------------------------------------------------------------------
//  Modified version of defopen for opening OUTPUT files
//  
//  Opens an output-file with name `base.ext' where in base
//  the path and extension of infile is stripped off.
//  The leading '.' in ext is optional. 
//
//  hs 27/4/98
//---------------------------------------------------------------------

FILE *defout(char *infile, char *ext, char *mode/*="w"*/) {
#define MAXLENG 250
  int i, p_dot, p_name;
  char buffer[MAXLENG];

  // search infile for path and extension
  p_name = 0;
  p_dot  = -1;

  for( i=0; infile[i] != '\0'; i++) {           
    if(infile[i]=='/') p_name = i+1;
    if(infile[i]=='.') p_dot = i;
  }

  // strip off path:
  strcpy(buffer,&infile[p_name]);
  
  // strip off extension:
  if ( p_dot > -1 ) buffer[p_dot-p_name] = 0;

  // append new extension:
  if ( *ext != '.' && strlen(ext)>0 ) strcat(buffer,".");
  strcat(buffer,ext);

  return fopen(buffer,mode);
}

//---------------------------------------------------------------------

int get_path_length(char *fullfilename)
{
  int i,l;
  i = strlen(fullfilename)-1;   /* la funzione strlen conta il numero 
                                   di caratteri di fullfilname, escluso 
                                   il carattere nullo di fine stringa */
  while(i>=0 && fullfilename[i]!='/') 
    i--;
  return i+1;
}
//---------------------------------------------------------------------

void get_filetype(char *fullfilename,char *filetype) {
  char *s;
  int i;
  i = get_path_length(fullfilename);
  s = fullfilename+i;
  while(*s && *s!='.' && *s!=';')s++;
  if(*s=='.') 
    {
      s++;
      while(*s && *s!=';')
	*filetype++ = *s++;
    }
  *filetype = '\0';
}

//--------------------------------------------------------------------

void change_filetype(char *fullfilename,char *filetype) {
  char tmp[256];
  char *s,*t;
  int i;
  if(*filetype=='.') filetype++;
  i = get_path_length(fullfilename);
  s = fullfilename+i;
  while(*s && *s!='.' && *s!=';')s++;
  t = s;
  while(*t && *t!=';') t++;
  if(*t==';') strcpy(tmp,t);
  else tmp[0]='\0';
  *s++ = '.';
  while(*filetype) 
    *s++ = *filetype++;
  strcpy(s,tmp);
}
//--------------------- end of file --------------------------------




