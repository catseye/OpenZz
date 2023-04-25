OpenZz 1.0.4-4ce2
=================

This is OpenZz 1.0.4-4ce2, a fork of the original OpenZz distribution
based on version 1.0.4-4.  The original README can be found at the
end of this Markdown file, encased in a preformatted text block.

This fork was made because development on OpenZz was stalled, I have a
project which requires it ([Zzrk](http://catseye.tc/node/Zzrk.html)), and I
needed to make changes to the sources to get them to build on the latest
version of `gcc`.

Shortly thereafter, upgrades to (apparently) `libtool`(?) caused `automake`
to generate Makefiles which went into infinite loops at the shell level
(`pstree` showed what looked like an infinite tree of `sed` and `bash`)
which would take down my laptop quite readily.

So, I added a simple script `build.sh` to build the `ozz` executable
directly, without messing around with libraries or anything.  It works
on Ubuntu and NetBSD, and will (I'm sure) work on other flavours of Linux
and other BSDs with only minor modifications.

Running `build.sh` is recommended.  Run `autogen.sh` and `configure` at
your own risk.

Much later, I discovered that `ozz` had problems when compiled on a
64-bit system.  I made some changes to the build script to force
compilation as a 32-bit executable.

Also note: several of the tests now fail.  The core semantics seem alright,
so these tests have been skipped.  The specific failures are:
    
*   `deref_param.zz`: failure of the test suite; it shouldn't be caring about
    an internal address, which isn't stable from run to run
*   list_in_cond.zz: the syntax "a.b" seems to be a syntax error
*   numerictypes.zz: 64-bit values wrap to 32 bits on a 32-bit archiecture
*   tagdtor.xx: can't open the specified .so file

At one point, `double.zz` also did not pass; but now it does.

The verdict is that this version of OpenZz does not support:

*   64-bit integer values
*   the "a.b" syntax (whatever that means exactly)
*   loading ".so" files

Which is fine by me so long as it can still play Zzrk.

-Chris

```
===============================================================================
                 Zz - Dynamic Lexical Parser README File
===============================================================================

Zz is a dynamic parser which is currently being developed as a front end to
gcc to implement compilation of new languages (ie. TAO and others), for APE
systems.  These are custom build parallel processing computers used for LQCD
physics research.

See:
  http://chimera.roma1.infn.it/ape.html

UPDATES
 See the NEWS and ChangeLog files for information on the latest changes
 Some support is being formulated at the project directory on SourceForge:
  http://openzz.sourceforge.net  

LICENSE
 The Zz library is released under the GNU Lesser GPL (LGPL).
 Parts of Zz are released under the GPL - see the "COPYING" file in the
   project root.

DOCUMENTATION
 See the "doc" subdirectory

APE
 Whence the name "APE"?  "Ape" is the Italian word for "bee", the mascot of
 the APE project. (INFN, the funder of the project is based in Italy and the
 software is primarily developed there).
```
