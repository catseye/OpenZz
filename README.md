OpenZz 1.0.4-4ce3
=================

This is OpenZz 1.0.4-4ce3, a fork of the original
[OpenZz distribution](https://openzz.sourceforge.net/),
based on OpenZz version 1.0.4-4.  The original README can be found at the
end of this Markdown file, encased in a preformatted text block.

This fork was made because development on OpenZz was stalled, I have a
project ([Zzrk](http://catseye.tc/node/Zzrk.html)) that requires OpenZz,
and, at some point, I was unable to build it on the then-current version
of `gcc`.  Shortly thereafter, upgrades to the build system also caused
problems; it seemed like `libtool` was now causing `automake`
to generate Makefiles which went into infinite loops at the shell level;
`pstree` showed what looked like an infinite tree of `sed` and `bash`.
This would take down my laptop quite readily.

It was at this point I decided to create the first version of this
fork.

### 1.0.4-4ce1

In 1.0.4-4ce1, I added a simple [`build.sh`](build.sh) script to build the `ozz`
executable directly, without messing around with `libtool` or anything.
It works on Ubuntu and NetBSD, and will, I'm sure, work on other flavours
of Linux and other BSDs with little or no modification.

Running `build.sh` is recommended.  Run `autogen.sh` and `configure` at
your own risk.

Note, though, that several of the tests now fail.  The core semantics
of OpenZz seem alright, so these failing tests have been skipped in the
test suite.  The specific failures are:

*   `deref_param.zz`: badly-written test; it shouldn't be caring about
    an internal address, which isn't stable from run to run
*   `list_in_cond.zz`: the syntax `a.b` seems to be a syntax error
*   `numerictypes.zz`: 64-bit values wrap to 32 bits on a 32-bit architecture
*   `tagdtor.xx`: can't open the specified `.so` file

The `double.zz` test also did not pass.

The verdict from those results was that this version of OpenZz does not support:

*   64-bit integer values
*   doubles
*   the `a.b` syntax (whatever that means exactly)
*   loading `.so` files

Which was fine by me, as it can still play [Zzrk](http://catseye.tc/node/Zzrk.html).

Later on, I discovered that `ozz` had problems when compiled on a
64-bit system.  For the longest time I was only thinking about how it runs on
[The Cat's Eye Technologies' Platform](https://catseye.tc/node/The%20Cat%27s%20Eye%20Technologies%20Platform),
where it runs just fine, because it's a 32-bit architecture.
But at some point I noticed it didn't work on 64-bit systems anymore,
and I did eventually care enough to try to fix it.  This led to the
second release of this forked project.

### 1.0.4-4ce2

In 1.0.4-4ce2, I made some changes to the build script to force
compilation as a 32-bit executable.  This is done by passing
the `-m32` flag to `gcc`.  For more information,
see e.g. [this StackOverflow question](https://stackoverflow.com/q/3501878)

For this to be successful, 32-bit headers must be installed on the system.
On Ubuntu, this can be accomplished with:

    sudo apt install gcc-multilib

For more information, see e.g.
[this StackOverflow question](https://stackoverflow.com/q/54082459)

This will also require 32-bit versions of the libraries that `ozz`
links to.  Under Ubuntu, they can be installed with:

    sudo dpkg --add-architecture i386
    sudo apt update
    sudo apt install libc6:i386 libedit-dev:i386

For more information, see e.g.
[this StackOverflow question](https://askubuntu.com/q/454253)

Also, under the recent 32-bit build that I made using these
modifications, for whatever reason, the `double.zz` test now passes.

Which is fine by me, as long as it can still play [Zzrk](http://catseye.tc/node/Zzrk.html).

### 1.0.4-4ce3

Includes many changes to the code to address compiler warnings.
While a few remain, the bulk of them have been eliminated.
(Many were simply due to `gcc` wanting to see an explicit return
type, instead of defaulting to `int`, and missing prototypes.)

Also includes an MS-DOS batchfile for building `ozz.exe` under
DOS using DJGPP.

The goal of both of these enhancements was to build an
instance of `ozz` that runs in a DOS emulator running in
a web browser, such as `em-dosbox`.

Which is fine by me, as now we can all
[play Zzrk on the Internet Archive](https://archive.org/details/zzrk_adventure)!

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
