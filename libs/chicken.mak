### makefile.vc - Makefile for building CHICKEN with VC++ 5/6 - felix

CC = cl
CFLAGS = /nologo /O2 /MT /DC_DEFAULT_TARGET_STACK_SIZE=300000 /DC_NO_PIC_NO_DLL /DHAVE_LOADLIBRARY /DHAVE_GETPROCADDRESS /DHAVE_WINDOWS_H
LFLAGS = /nologo /subsystem:console /incremental:no /machine:I386
CHICKEN = chicken
SFLAGS = -quiet -explicit-use -debug-level 0 -optimize-level 2
SXFLAGS = -quiet -no-warnings -debug-level 0 -optimize-level 2
#SFLAGS = -quiet -explicit-use -optimize-level 2
SUFLAGS = -quiet -explicit-use -debug-level 0 -optimize-level 2 -feature unsafe -unsafe
WINLIBS = ws2_32.lib
WINGUILIBS = $(WINLIBS) kernel32.lib user32.lib gdi32.lib

.SUFFIXES : .scm

# Targets:

all : libchicken.lib chicken.dll libchicken-static.lib \
	libchicken-gui.lib chicken-gui.dll libchicken-gui-static.lib \
	libuchicken.lib uchicken.dll libuchicken-static.lib \
	chicken.exe chicken-static.exe \
	csi.exe csi-static.exe csc.exe chicken-profile.exe chicken-setup.exe

clean :
	del *.obj *.obs library.c eval.c lolevel.c tinyclos.c srfi-14.c srfi-1.c srfi-13.c scheduler.c srfi-18.c extras.c \
	  utils.c posixwin.c profiler.c match-support.c pregexp.c srfi-4.c stub.c \
	  ulibrary.c ueval.c ulolevel.c utinyclos.c usrfi-14.c usrfi-1.c usrfi-13.c usrfi-18.c \
	  uextras.c \
	  uutils.c uposixwin.c umatch-support.c upregexp.c usrfi-4.c tcp.c utcp.c \
	  chicken.c compiler.c support.c partition.c easyffi.c c-backend.c c-platform.c optimizer.c \
	  *.lib *.exe *.exp *.dll


# Compiling from sources:

.scm.c : 
	$(CHICKEN) $< -output-file $@ $(SFLAGS)

ulibrary.c : library.scm
	$(CHICKEN) library.scm -output-file $@ $(SUFLAGS)
ueval.c : eval.scm
	$(CHICKEN) eval.scm -output-file $@ $(SUFLAGS)
upregexp.c : pregexp.scm
	$(CHICKEN) pregexp.scm -output-file $@ $(SUFLAGS)
ulolevel.c : lolevel.scm
	$(CHICKEN) lolevel.scm -output-file $@ $(SUFLAGS)
utinyclos.c : tinyclos.scm
	$(CHICKEN) tinyclos.scm -output-file $@ $(SUFLAGS)
usrfi-1.c : srfi-1.scm
	$(CHICKEN) srfi-1.scm -output-file $@ $(SUFLAGS)
usrfi-4.c : srfi-4.scm
	$(CHICKEN) srfi-4.scm -output-file $@ $(SUFLAGS)
usrfi-13.c : srfi-13.scm
	$(CHICKEN) srfi-13.scm -output-file $@ $(SUFLAGS)
usrfi-14.c : srfi-14.scm
	$(CHICKEN) srfi-14.scm -output-file $@ $(SUFLAGS)
usrfi-18.c : srfi-18.scm
	$(CHICKEN) srfi-18.scm -output-file $@ $(SUFLAGS)
umatch-support.c : match-support.scm
	$(CHICKEN) match-support.scm -output-file $@ $(SUFLAGS)
uextras.c : extras.scm
	$(CHICKEN) extras.scm -output-file $@ $(SUFLAGS)
uutils.c : utils.scm
	$(CHICKEN) utils.scm -output-file $@ $(SUFLAGS)
utcp.c : tcp.scm
	$(CHICKEN) tcp.scm -output-file $@ $(SUFLAGS)
uposixwin.c : posixwin.scm
	$(CHICKEN) posixwin.scm -output-file $@ $(SUFLAGS)


# The runtime library:

libchicken.lib chicken.dll: runtime.obj library.obj eval.obj profiler.obj scheduler.obj \
	extras.obj match-support.obj lolevel.obj tinyclos.obj pregexp.obj utils.obj \
	tcp.obj posixwin.obj srfi-1.obj srfi-4.obj srfi-13.obj srfi-14.obj srfi-18.obj stub.obj
	link /nologo /out:chicken.dll /implib:libchicken.lib /dll $(WINLIBS) $**

libchicken-static.lib : runtime.obs library.obs eval.obs profiler.obs scheduler.obs \
	extras.obs match-support.obs lolevel.obs tinyclos.obs pregexp.obs utils.obs \
	tcp.obs posixwin.obs srfi-1.obs srfi-4.obs srfi-13.obs srfi-14.obs srfi-18.obs stub.obs
	lib /nologo /out:$@ $(WINLIBS) $**

libuchicken.lib uchicken.dll: uruntime.obj ulibrary.obj ueval.obj profiler.obj scheduler.obj \
	uextras.obj umatch-support.obj ulolevel.obj utinyclos.obj upregexp.obj uutils.obj \
	utcp.obj uposixwin.obj usrfi-1.obj usrfi-4.obj usrfi-13.obj usrfi-14.obj usrfi-18.obj stub.obj
	link /nologo /out:uchicken.dll /implib:libuchicken.lib /dll $(WINLIBS) $**

libuchicken-static.lib : uruntime.obs ulibrary.obs ueval.obs profiler.obs scheduler.obs \
	uextras.obs umatch-support.obs ulolevel.obs utinyclos.obs upregexp.obs uutils.obs \
	utcp.obs uposixwin.obs usrfi-1.obs usrfi-4.obs usrfi-13.obs usrfi-14.obs usrfi-18.obs stub.obs
	lib /nologo /out:$@ $(WINLIBS) $**

libchicken-gui.lib chicken-gui.dll: gui-runtime.obj gui-library.obj eval.obj profiler.obj scheduler.obj \
	extras.obj match-support.obj lolevel.obj tinyclos.obj pregexp.obj utils.obj \
	tcp.obj posixwin.obj srfi-1.obj srfi-4.obj srfi-13.obj srfi-14.obj srfi-18.obj stub.obj
	link /nologo /out:chicken-gui.dll /implib:libchicken-gui.lib /dll $(WINGUILIBS) $**

libchicken-gui-static.lib : gui-runtime.obs gui-library.obs eval.obs profiler.obs scheduler.obs \
	extras.obs match-support.obs lolevel.obs tinyclos.obs pregexp.obs utils.obs \
	tcp.obs posixwin.obs srfi-1.obs srfi-4.obs srfi-13.obs srfi-14.obs srfi-18.obs stub.obs
	lib /nologo /out:$@ $(WINGUILIBS) $**

.c.obj:
	$(CC) $(CFLAGS) /c $< /DPIC

.c.obs:
	$(CC) $(CFLAGS) /c $< /Fo$@

runtime.obj : runtime.c chicken.h
	$(CC) $(CFLAGS) /c runtime.c /DC_BUILDING_LIBCHICKEN /DPIC

runtime.obs : runtime.c chicken.h
	$(CC) $(CFLAGS) /c runtime.c /DC_BUILDING_LIBCHICKEN /Fo$@

uruntime.obj : runtime.c chicken.h
	$(CC) $(CFLAGS) /c runtime.c /DC_BUILDING_LIBCHICKEN /DC_UNSAFE_RUNTIME /DPIC /Fo$@

uruntime.obs : runtime.c chicken.h
	$(CC) $(CFLAGS) /c runtime.c /DC_BUILDING_LIBCHICKEN /DC_UNSAFE_RUNTIME /Fo$@

gui-runtime.obj : runtime.c
	$(CC) $(CFLAGS) /c /DC_WINDOWS_GUI runtime.c /Fo$@ /DC_BUILDING_LIBCHICKEN /DPIC

gui-runtime.obs : runtime.c
	$(CC) $(CFLAGS) /c /DC_WINDOWS_GUI runtime.c /Fo$@ /DC_BUILDING_LIBCHICKEN /Fo$@

eval.obj : eval.c chicken.h
	$(CC) $(CFLAGS) /c eval.c /DC_BUILDING_LIBCHICKEN /DPIC
eval.obs : eval.c chicken.h
	$(CC) $(CFLAGS) /c eval.c /DC_BUILDING_LIBCHICKEN /Fo$@
library.obj : library.c chicken.h
	$(CC) $(CFLAGS) /c library.c /DC_BUILDING_LIBCHICKEN /DPIC
library.obs : library.c chicken.h
	$(CC) $(CFLAGS) /c library.c /DC_BUILDING_LIBCHICKEN /Fo$@
profiler.obj : profiler.c chicken.h
	$(CC) $(CFLAGS) /c profiler.c /DC_BUILDING_LIBCHICKEN /DPIC
profiler.obs : profiler.c chicken.h
	$(CC) $(CFLAGS) /c profiler.c /DC_BUILDING_LIBCHICKEN /Fo$@
scheduler.obj : scheduler.c chicken.h
	$(CC) $(CFLAGS) /c scheduler.c /DC_BUILDING_LIBCHICKEN /DPIC
scheduler.obs : scheduler.c chicken.h
	$(CC) $(CFLAGS) /c scheduler.c /DC_BUILDING_LIBCHICKEN /Fo$@

extras.obj : extras.c chicken.h
extras.obs : extras.c chicken.h
srfi-1.obj : srfi-1.c chicken.h
srfi-1.obs : srfi-1.c chicken.h
srfi-4.obj : srfi-4.c chicken.h
srfi-4.obs : srfi-4.c chicken.h
match-support.obj : match-support.c chicken.h
match-support.obs : match-support.c chicken.h
tinyclos.obj : tinyclos.c chicken.h
tinyclos.obs : tinyclos.c chicken.h
srfi-13.obj : srfi-13.c chicken.h
srfi-13.obs : srfi-13.c chicken.h
srfi-14.obj : srfi-14.c chicken.h
srfi-14.obs : srfi-14.c chicken.h
srfi-18.obj : srfi-18.c chicken.h
srfi-18.obs : srfi-18.c chicken.h
lolevel.obj : lolevel.c chicken.h
lolevel.obs : lolevel.c chicken.h
stub.obj : stub.c chicken.h
stub.obs : stub.c chicken.h
pregexp.obj : pregexp.c chicken.h
pregexp.obs : pregexp.c chicken.h
utils.obj : utils.c chicken.h
utils.obs : utils.c chicken.h
tcp.obj : tcp.c chicken.h
tcp.obs : tcp.c chicken.h
posixwin.obj : posixwin.c chicken.h
posixwin.obs : posixwin.c chicken.h

ueval.obj : ueval.c chicken.h
	$(CC) $(CFLAGS) /c ueval.c /DC_BUILDING_LIBCHICKEN /DPIC
ueval.obs : ueval.c chicken.h
	$(CC) $(CFLAGS) /c ueval.c /DC_BUILDING_LIBCHICKEN /Fo$@
ulibrary.obj : ulibrary.c chicken.h
	$(CC) $(CFLAGS) /c ulibrary.c /DC_BUILDING_LIBCHICKEN /DPIC
ulibrary.obs : ulibrary.c chicken.h
	$(CC) $(CFLAGS) /c ulibrary.c /DC_BUILDING_LIBCHICKEN /Fo$@
uextras.obj : uextras.c chicken.h
uextras.obs : uextras.c chicken.h
usrfi-1.obj : usrfi-1.c chicken.h
usrfi-1.obs : usrfi-1.c chicken.h
usrfi-4.obj : usrfi-4.c chicken.h
usrfi-4.obs : usrfi-4.c chicken.h
umatch-support.obj : umatch-support.c chicken.h
umatch-support.obs : umatch-support.c chicken.h
utinyclos.obj : utinyclos.c chicken.h
utinyclos.obs : utinyclos.c chicken.h
usrfi-13.obj : usrfi-13.c chicken.h
usrfi-13.obs : usrfi-13.c chicken.h
usrfi-14.obj : usrfi-14.c chicken.h
usrfi-14.obs : usrfi-14.c chicken.h
usrfi-18.obj : usrfi-18.c chicken.h
usrfi-18.obs : usrfi-18.c chicken.h
ulolevel.obj : ulolevel.c chicken.h
ulolevel.obs : ulolevel.c chicken.h
upregexp.obj : upregexp.c chicken.h
upregexp.obs : upregexp.c chicken.h
uutils.obj : uutils.c chicken.h
uutils.obs : uutils.c chicken.h
utcp.obj : utcp.c chicken.h
utcp.obs : utcp.c chicken.h
uposixwin.obj : uposixwin.c chicken.h
uposixwin.obs : uposixwin.c chicken.h

gui-library.obj : library.c chicken.h
	$(CC) $(CFLAGS) /c /DC_WINDOWS_GUI library.c /Fo$@ /DC_BUILDING_LIBCHICKEN /DPIC
gui-library.obs : library.c chicken.h
	$(CC) $(CFLAGS) /c /DC_WINDOWS_GUI library.c /Fo$@ /DC_BUILDING_LIBCHICKEN /Fo$@

chicken.res : chicken.rc
	rc /r chicken.rc

# The interpreter:

csi.c: csi.scm build.scm chicken-static.exe
	.\chicken-static.exe csi.scm -optimize-level 2 -debug-level 0 -quiet -output-file csi.c -prologue build.scm
csi.obj: csi.c chicken.h
csi.obs: csi.c chicken.h

csi.exe        : csi.obj libchicken.lib chicken.res
	link $(LFLAGS) $** /out:$@

csi-static.exe : csi.obs libchicken-static.lib chicken.res
	link $(LFLAGS) $** /out:$@


# csc:

csc.c: csc.scm chicken-static.exe
	.\chicken-static.exe csc.scm -optimize-level 2 -debug-level 0 -quiet -output-file csc.c
csc.obj: csc.c chicken.h

csc.scm: csc.scm.in
	copy /Y csc.scm.in csc.scm

csc.exe        : csc.obj libchicken.lib chicken.res
	link $(LFLAGS) $** /out:$@


# chicken-profile:

chicken-profile.c: chicken-profile.scm chicken-static.exe
	.\chicken-static.exe chicken-profile.scm -optimize-level 2 -debug-level 0 -quiet -output-file chicken-profile.c
chicken-profile.obj: chicken-profile.c chicken.h

chicken-profile.exe: chicken-profile.obj libchicken.lib chicken.res
	link $(LFLAGS) $** /out:$@


# chicken-setup:

chicken-setup.c: chicken-setup.scm parameters.scm
	.\chicken-static.exe chicken-setup.scm -optimize-level 2 -debug-level 0 -quiet -output-file chicken-setup.c
chicken-setup.obj: chicken-setup.c chicken.h

chicken-setup.exe: chicken-setup.obj libchicken.lib chicken.res
	link $(LFLAGS) $** /out:$@


# The compiler:

chicken.exe : chicken.obj support.obj partition.obj easyffi.obj compiler.obj optimizer.obj batch-driver.obj \
	  c-platform.obj c-backend.obj chicken.res \
	  libchicken.lib
	link $(LFLAGS) $** /out:$@

chicken-static.exe : chicken.obs support.obs partition.obs easyffi.obs compiler.obs optimizer.obs batch-driver.obs \
	  c-platform.obs c-backend.obs chicken.res \
	  libchicken-static.lib
	link $(LFLAGS) $** /out:$@

chicken.c: chicken.scm tweaks.scm
	$(CHICKEN) chicken.scm $(SXFLAGS) -output-file chicken.c

chicken.obj : chicken.c chicken.h
chicken.obs : chicken.c chicken.h
support.obj : support.c chicken.h
support.obs : support.c chicken.h
partition.obj : partition.c chicken.h
partition.obs : partition.c chicken.h
easyffi.obj : easyffi.c chicken.h
easyffi.obs : easyffi.c chicken.h
c-platform.obj : c-platform.c chicken.h
c-platform.obs : c-platform.c chicken.h
c-backend.obj : c-backend.c chicken.h
c-backend.obs : c-backend.c chicken.h
batch-driver.obj : batch-driver.c chicken.h
batch-driver.obs : batch-driver.c chicken.h
compiler.obj : compiler.c chicken.h
compiler.obs : compiler.c chicken.h
optimizer.obj : optimizer.c chicken.h
optimizer.obs : optimizer.c chicken.h

easyffi.c: easyffi.l.silex

easyffi.l.silex : easyffi.l
	csi -script silex.scm easyffi.l counters none
	move easyffi.l.scm easyffi.l.silex
