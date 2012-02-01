## INCLUDES
## LDFLAGS
## LDADD
## EXTRADIST
## SUBDIRS
## check_PROGRAMS
## TESTS

lib_LTLIBRARIES = libozz.la libozzi.la

libozz_la_SOURCES=err.c source.c zlex.c list.c rule.c scope.c table.c dumpnet.c \
	      parse.c param.c action.c lazy.c zkernel.c zsys.c kernel.c \
              sys.c interface.c defopen.c avl.c printz.c util.c \
	      avl.h kernel.h mem.h rule.h sys.h zlex.h \
	      err.h list.h param.h printz.h source.h trace.h table.h
libozz_la_LIBADD = $(LIBM)

libozzi_la_SOURCES= zzi.c

## from libtool documentation
##-version-info current[:revision[:age]]
##   current
##     The most recent interface number that this library implements.
##   revision
##     The implementation number of the current interface.
##   age
##     The difference between the newest and oldest interfaces that this library
##     implements. In other words, the library implements all the interface 
##     numbers in the range from number current - age to current.
##
## If two libraries have identical current and age numbers, then the
## dynamic linker chooses the library with the greater revision
## number.
## If either revision or age are omitted, they default to
## 0. Also note that age must be less than or equal to the current
## interface number.
## Here are a set of rules to help you update your library version
## information:
##    1. Start with version information of 0:0:0 for each libtool library.
##    2. Update the version information only immediately before a public 
##       release of your software. More frequent updates are unnecessary, 
##       and only guarantee that the current interface number gets larger faster.
##    3. If the library source code has changed at all since the last 
##       update, then increment revision (c:r:a becomes c:r+1:a).
##    4. If any interfaces have been added, removed, or changed since the 
##       last update, increment current, and set revision to 0.
##    5. If any interfaces have been added since the last public release, 
##       then increment age.
##    6. If any interfaces have been removed since the last public release, 
##       then set age to 0.
##
## Never try to set the interface numbers so that they correspond to
## the release number of your package. This is an abuse that only
## fosters misunderstanding of the purpose of library
## versions. Instead, use the -release flag (see Release numbers), but
## be warned that every release of your package will not be binary
## compatible with any other release.
libozz_la_LDFLAGS = -version-info 1:4:1 -export-dynamic
libozzi_la_LDFLAGS = -version-info 1:4:1 -export-dynamic

## headers to be installed
openzzincludedir = $(includedir)/ozz
openzzinclude_HEADERS= zz.h zzbind.h


bin_PROGRAMS=ozz
ozz_SOURCES = zz.c
ozz_LDADD = libozzi.la libozz.la

# zz_LDFLAGS = 
# zz_DEPENDENCIES=libzz
