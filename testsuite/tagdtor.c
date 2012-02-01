#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <zz.h>
#include <zzbind.h>

#define OPEN(N) zz_bind_open(#N);
#define M(S) zz_bind_keyword(S);
#define GSB(N) zz_bind_match(#N);
#define END zz_bind_close();
#define PASS zz_bind_pass();
#define SFUN(T,P)  {zz_bind_call_fun(P,T);}
#define SFUN2(T,P) {zz_bind_call_fun_tag(P,T);}
#define SPROC(P)   {zz_bind_call(P);}
#define PROC(P)    {zz_bind_call_exe_no_tag(P);}
#define FUN(T,P)   {zz_bind_call_exe_proc(P,T);}


static int foo_sprint(char *s, struct s_content *foo)
{
  sprintf(s, "foo(%p)", foo);
}

static int foo_fprint(FILE *fd, struct s_content *foo)
{
  fprintf(fd, "foo(%p)", foo);
}

static int foo_ctor(struct s_content *foo, const char *pname)
{
  assert(foo);
  //printf("foo param ctor (%p, tag=%s) name=%s invoked\n", foo, zz_scnt_get_tag_name(foo), pname);
  printf("invoked param ctor (tag=%s name=%s)\n", zz_scnt_get_tag_name(foo), pname);
}

static int foo_dtor(struct s_content *foo, const char *pname)
{
  //printf("foo param dtor (%p, tag=%s) name=%s invoked\n", foo, zz_scnt_get_tag_name(foo), pname);
  printf("invoked param dtor (tag=%s name=%s)\n", zz_scnt_get_tag_name(foo), pname);
}

int foo_new()
{
  printf("foo_new called\n");
  //abort();
  return 0;
}

int foo_remove_tag()
{
  printf("foo_remove_tag called\n");
  
  zz_lex_remove_tag("foo");

  // should be useless...
  return 0;
}

void zz_ext_init()
{
  int ret;
  ret = zz_lex_add_new_tag("foo", foo_sprint, foo_fprint, foo_ctor, foo_dtor, 0);

  zz_parse_string("/$arg->foo^$ : pass;");

  OPEN(foo) M("$foo_new") SFUN("foo", foo_new) END;
  OPEN(stat) M("$foo_remove_tag") SPROC(foo_remove_tag) END;

  printf("registered foo tag (ret=%d)\n", ret);
}

#if 0
int main(int argc, char* argv[])
{
  int ret;
  assert(argc==2);
  zz_set_output_stream(stdout);
  /* LAUNCH ZZ */
  zz_init();
  zz_ext_init();
  zz_set_default_extension("zz");
  ret=zz_parse_file(argv[1]);

  fprintf(stderr, "bye (ret=%d)\n",ret);

  /* EXIT */
  if(ret) 
    exit(EXIT_SUCCESS);
  else {
    print_error_count();
    exit(EXIT_FAILURE);
  }
  
  return 0;
}
#endif
