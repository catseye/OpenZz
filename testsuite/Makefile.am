## OpenZz testsuite resource script

EXTRA_DIST = zz_test.sh \
 boolean.zz      \
 deref_param.zz  \
 double.zz       \
 foreach.zz      \
 integer.zz      \
 list_in_cond.zz \
 listmerge.zz    \
 listsplit.zz    \
 looptest_do.zz  \
 looptest_while.zz \
 numerictypes.zz \
 setcase.zz      \
 setparseeol.zz  \
 setparseeol2.zz \
 utilities.zz    \
 popvoidrule.zz  \
 tagdtor.zz	 \
 when_del_scope.zz

#all: test

INCLUDES = -I$(top_srcdir)/src

TESTS = libtagdtor.la

lib_LTLIBRARIES = libtagdtor.la
libtagdtor_la_SOURCES = tagdtor.c
libtagdtor_la_LIBADD = ../src/libozz.la

#check_PROGRAMS = zztest

#bin_PROGRAMS=tagdtor
#tagdtor_SOURCES=tagdtor.c
#tagdtor_LDADD=../src/libozz.la

ED=$(EXTRA_DIST:%.zz=$(srcdir)/%.zz)
BUILDPATH=$(PWD)

zztest:
	export PATH="$(BUILDPATH)/../src:$(PATH)"; \
	echo $(PATH);\
	$(srcdir)/zz_test.sh -d $(srcdir)

testold:
	@BUILDPATH=$$PWD;\
	cd $(srcdir);\
	PATH="$$BUILDPATH/../src:$$PATH"; ./zz_test.sh
