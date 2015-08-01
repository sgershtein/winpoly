#
# Borland C++ IDE generated makefile
# Generated 07.03.99 at 21:15:24
#
.AUTODEPEND


#
# Borland C++ tools
#
IMPLIB  = Implib
BCC32   = Bcc32 +BccW32.cfg
BCC32I  = Bcc32i +BccW32.cfg
TLINK32 = TLink32
ILINK32 = Ilink32
TLIB    = TLib
BRC32   = Brc32
TASM32  = Tasm32
#
# IDE macros
#
#
# External tools
#
CompileHLP = hc31.exe  # IDE Command Line: $NOSWAP $CAP MSG(HC312MSG) $EDNAME


#
# Options
#
IDE_LinkFLAGS32 =  -LD:\BC5\LIB
IDE_ResFLAGS32 =
LinkerLocalOptsAtW32_winpolydexe =  -Tpe -aa -V4.0 -c
ResLocalOptsAtW32_winpolydexe =
BLocalOptsAtW32_winpolydexe =
CompInheritOptsAt_winpolydexe = -I.\;D:\BC5\INCLUDE -DSTRICT;_OWLPCH;
LinkerInheritOptsAt_winpolydexe = -x
LinkerOptsAt_winpolydexe = $(LinkerLocalOptsAtW32_winpolydexe)
ResOptsAt_winpolydexe = $(ResLocalOptsAtW32_winpolydexe)
BOptsAt_winpolydexe = $(BLocalOptsAtW32_winpolydexe)
CompLocalOptsAtW32_winpolydhpj =
LinkerLocalOptsAtW32_winpolydhpj =
ResLocalOptsAtW32_winpolydhpj =
BLocalOptsAtW32_winpolydhpj =
CompOptsAt_winpolydhpj = $(CompOptsAt_winpolydexe) $(CompLocalOptsAtW32_winpolydhpj)
CompInheritOptsAt_winpolydhpj = -I.\;D:\BC5\INCLUDE -DSTRICT;_OWLPCH;
LinkerInheritOptsAt_winpolydhpj = -x
LinkerOptsAt_winpolydhpj = $(LinkerOptsAt_winpolydexe) $(LinkerLocalOptsAtW32_winpolydhpj)
ResOptsAt_winpolydhpj = $(ResOptsAt_winpolydexe) $(ResLocalOptsAtW32_winpolydhpj)
BOptsAt_winpolydhpj = $(BOptsAt_winpolydexe) $(BLocalOptsAtW32_winpolydhpj)

#
# Dependency List
#
Dep_Winpoly = \
   winpoly.exe

Winpoly : BccW32.cfg $(Dep_Winpoly)
  echo MakeNode

Dep_winpolydexe = \
   brdata.obj\
   tlist.h\
   tarray.h\
   tdraw.obj\
   texcept.h\
   tfield.obj\
   tpoly.obj\
   polymain.obj\
   apxprint.obj\
   polywin.obj\
   apxprev.obj\
   aboutdlg.obj\
   polymain.res\
   polymain.def\
   winpoly.hlp

winpoly.exe : $(Dep_winpolydexe)
  $(ILINK32) @&&|
 /v $(IDE_LinkFLAGS32) $(LinkerOptsAt_winpolydexe) $(LinkerInheritOptsAt_winpolydexe) +
D:\BC5\LIB\c0w32.obj+
brdata.obj+
tdraw.obj+
tfield.obj+
tpoly.obj+
polymain.obj+
apxprint.obj+
polywin.obj+
apxprev.obj+
aboutdlg.obj
$<,$*
D:\BC5\LIB\owlwf.lib+
D:\BC5\LIB\bidsf.lib+
D:\BC5\LIB\import32.lib+
D:\BC5\LIB\cw32.lib
polymain.def
polymain.res

|
Dep_brdatadobj = \
   brdata.h\
   brdata.cpp

brdata.obj : $(Dep_brdatadobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ brdata.cpp
|

Dep_tdrawdobj = \
   tdraw.h\
   tdraw.cpp

tdraw.obj : $(Dep_tdrawdobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ tdraw.cpp
|

Dep_tfielddobj = \
   intersec.h\
   tfield.h\
   tfield.cpp

tfield.obj : $(Dep_tfielddobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ tfield.cpp
|

Dep_tpolydobj = \
   tpoly.h\
   tpoly.cpp

tpoly.obj : $(Dep_tpolydobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ tpoly.cpp
|

Dep_polymaindobj = \
   polymain.h\
   polymain.cpp

polymain.obj : $(Dep_polymaindobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ polymain.cpp
|

Dep_apxprintdobj = \
   apxprint.h\
   apxprint.cpp

apxprint.obj : $(Dep_apxprintdobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ apxprint.cpp
|

Dep_polywindobj = \
   polywin.h\
   polywin.cpp

polywin.obj : $(Dep_polywindobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ polywin.cpp
|

Dep_apxprevdobj = \
   apxprev.h\
   apxprev.cpp

apxprev.obj : $(Dep_apxprevdobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ apxprev.cpp
|

Dep_aboutdlgdobj = \
   aboutdlg.h\
   aboutdlg.cpp

aboutdlg.obj : $(Dep_aboutdlgdobj)
  $(BCC32) -c @&&|
 $(CompOptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe) -o$@ aboutdlg.cpp
|

Dep_polymaindres = \
   polymain.rh\
   polymain.rc

polymain.res : $(Dep_polymaindres)
  $(BRC32) -R @&&|
 $(IDE_ResFLAGS32) $(ROptsAt_winpolydexe) $(CompInheritOptsAt_winpolydexe)  -FO$@ polymain.rc
|
Dep_winpolydhlp = \
   polymain.rh\
   mainhelp.rtf\
   keys.rtf\
   winpoly.hpj

winpoly.hlp : $(Dep_winpolydhlp)
  $(CompileHLP)   D:\Documents\language\CPP\winpoly\winpoly.hpj

# Compiler configuration file
BccW32.cfg : 
   Copy &&|
-w
-R
-v
-WM-
-vi
-H
-H=Winpoly.csm
-W
-H"owl\pch.h"
| $@


