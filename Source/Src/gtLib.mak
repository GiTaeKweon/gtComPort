# Microsoft Developer Studio Generated NMAKE File, Based on gtLib.dsp
!IF "$(CFG)" == ""
CFG=gtLib - Win32 Debug
!MESSAGE No configuration specified. Defaulting to gtLib - Win32 Debug.
!ENDIF 

!IF "$(CFG)" != "gtLib - Win32 Release" && "$(CFG)" != "gtLib - Win32 Debug"
!MESSAGE Invalid configuration "$(CFG)" specified.
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gtLib.mak" CFG="gtLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gtLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "gtLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 
!ERROR An invalid configuration is specified.
!ENDIF 

!IF "$(OS)" == "Windows_NT"
NULL=
!ELSE 
NULL=nul
!ENDIF 

!IF  "$(CFG)" == "gtLib - Win32 Release"

OUTDIR=.\../Lib
INTDIR=.\Release

ALL : "..\Bin\gtLib10.dll"


CLEAN :
	-@erase "$(INTDIR)\ChartOptionDlg.obj"
	-@erase "$(INTDIR)\GraphOptionDlg.obj"
	-@erase "$(INTDIR)\gtChart.obj"
	-@erase "$(INTDIR)\gtComPort.obj"
	-@erase "$(INTDIR)\gtGraph.obj"
	-@erase "$(INTDIR)\gtLegend.obj"
	-@erase "$(INTDIR)\gtLib.obj"
	-@erase "$(INTDIR)\gtLib.pch"
	-@erase "$(INTDIR)\gtLib.res"
	-@erase "$(INTDIR)\gtTimer.obj"
	-@erase "$(INTDIR)\gtWaveform.obj"
	-@erase "$(INTDIR)\gtWaveformChart.obj"
	-@erase "$(INTDIR)\gtXYChart.obj"
	-@erase "$(INTDIR)\gtXYGraph.obj"
	-@erase "$(INTDIR)\SelectPoint.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(OUTDIR)\gtLib10.exp"
	-@erase "$(OUTDIR)\gtLib10.lib"
	-@erase "..\Bin\gtLib10.dll"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MD /W3 /GX /O2 /I "../Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /Fp"$(INTDIR)\gtLib.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "NDEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\gtLib.res" /d "NDEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gtLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:no /pdb:"$(OUTDIR)\gtLib10.pdb" /machine:I386 /def:".\gtLibRelease.def" /out:"../Bin/gtLib10.dll" /implib:"$(OUTDIR)\gtLib10.lib" 
LINK32_OBJS= \
	"$(INTDIR)\ChartOptionDlg.obj" \
	"$(INTDIR)\GraphOptionDlg.obj" \
	"$(INTDIR)\gtChart.obj" \
	"$(INTDIR)\gtComPort.obj" \
	"$(INTDIR)\gtGraph.obj" \
	"$(INTDIR)\gtLegend.obj" \
	"$(INTDIR)\gtLib.obj" \
	"$(INTDIR)\gtTimer.obj" \
	"$(INTDIR)\gtWaveform.obj" \
	"$(INTDIR)\gtWaveformChart.obj" \
	"$(INTDIR)\gtXYChart.obj" \
	"$(INTDIR)\gtXYGraph.obj" \
	"$(INTDIR)\SelectPoint.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\gtLib.res"

"..\Bin\gtLib10.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ELSEIF  "$(CFG)" == "gtLib - Win32 Debug"

OUTDIR=.\../Lib
INTDIR=.\Debug

ALL : "..\Bin\gtLib10d.dll"


CLEAN :
	-@erase "$(INTDIR)\ChartOptionDlg.obj"
	-@erase "$(INTDIR)\GraphOptionDlg.obj"
	-@erase "$(INTDIR)\gtChart.obj"
	-@erase "$(INTDIR)\gtComPort.obj"
	-@erase "$(INTDIR)\gtGraph.obj"
	-@erase "$(INTDIR)\gtLegend.obj"
	-@erase "$(INTDIR)\gtLib.obj"
	-@erase "$(INTDIR)\gtLib.pch"
	-@erase "$(INTDIR)\gtLib.res"
	-@erase "$(INTDIR)\gtTimer.obj"
	-@erase "$(INTDIR)\gtWaveform.obj"
	-@erase "$(INTDIR)\gtWaveformChart.obj"
	-@erase "$(INTDIR)\gtXYChart.obj"
	-@erase "$(INTDIR)\gtXYGraph.obj"
	-@erase "$(INTDIR)\SelectPoint.obj"
	-@erase "$(INTDIR)\StdAfx.obj"
	-@erase "$(INTDIR)\vc60.idb"
	-@erase "$(INTDIR)\vc60.pdb"
	-@erase "$(OUTDIR)\gtLib10d.exp"
	-@erase "$(OUTDIR)\gtLib10d.lib"
	-@erase "$(OUTDIR)\gtLib10d.pdb"
	-@erase "..\Bin\gtLib10d.dll"
	-@erase "..\Bin\gtLib10d.ilk"

"$(OUTDIR)" :
    if not exist "$(OUTDIR)/$(NULL)" mkdir "$(OUTDIR)"

"$(INTDIR)" :
    if not exist "$(INTDIR)/$(NULL)" mkdir "$(INTDIR)"

F90=df.exe
CPP=cl.exe
CPP_PROJ=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /Fp"$(INTDIR)\gtLib.pch" /Yu"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

.c{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.obj::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.c{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cpp{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

.cxx{$(INTDIR)}.sbr::
   $(CPP) @<<
   $(CPP_PROJ) $< 
<<

MTL=midl.exe
MTL_PROJ=/nologo /D "_DEBUG" /mktyplib203 /win32 
RSC=rc.exe
RSC_PROJ=/l 0x412 /fo"$(INTDIR)\gtLib.res" /d "_DEBUG" 
BSC32=bscmake.exe
BSC32_FLAGS=/nologo /o"$(OUTDIR)\gtLib.bsc" 
BSC32_SBRS= \
	
LINK32=link.exe
LINK32_FLAGS=winmm.lib /nologo /subsystem:windows /dll /incremental:yes /pdb:"$(OUTDIR)\gtLib10d.pdb" /debug /machine:I386 /def:".\gtLibDebug.def" /out:"../Bin/gtLib10d.dll" /implib:"$(OUTDIR)\gtLib10d.lib" /pdbtype:sept 
LINK32_OBJS= \
	"$(INTDIR)\ChartOptionDlg.obj" \
	"$(INTDIR)\GraphOptionDlg.obj" \
	"$(INTDIR)\gtChart.obj" \
	"$(INTDIR)\gtComPort.obj" \
	"$(INTDIR)\gtGraph.obj" \
	"$(INTDIR)\gtLegend.obj" \
	"$(INTDIR)\gtLib.obj" \
	"$(INTDIR)\gtTimer.obj" \
	"$(INTDIR)\gtWaveform.obj" \
	"$(INTDIR)\gtWaveformChart.obj" \
	"$(INTDIR)\gtXYChart.obj" \
	"$(INTDIR)\gtXYGraph.obj" \
	"$(INTDIR)\SelectPoint.obj" \
	"$(INTDIR)\StdAfx.obj" \
	"$(INTDIR)\gtLib.res"

"..\Bin\gtLib10d.dll" : "$(OUTDIR)" $(DEF_FILE) $(LINK32_OBJS)
    $(LINK32) @<<
  $(LINK32_FLAGS) $(LINK32_OBJS)
<<

!ENDIF 


!IF "$(NO_EXTERNAL_DEPS)" != "1"
!IF EXISTS("gtLib.dep")
!INCLUDE "gtLib.dep"
!ELSE 
!MESSAGE Warning: cannot find "gtLib.dep"
!ENDIF 
!ENDIF 


!IF "$(CFG)" == "gtLib - Win32 Release" || "$(CFG)" == "gtLib - Win32 Debug"
SOURCE=.\ChartOptionDlg.cpp

"$(INTDIR)\ChartOptionDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\GraphOptionDlg.cpp

"$(INTDIR)\GraphOptionDlg.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtChart.cpp

"$(INTDIR)\gtChart.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtComPort.cpp

"$(INTDIR)\gtComPort.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtGraph.cpp

"$(INTDIR)\gtGraph.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtLegend.cpp

"$(INTDIR)\gtLegend.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtLib.cpp

"$(INTDIR)\gtLib.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtLib.rc

"$(INTDIR)\gtLib.res" : $(SOURCE) "$(INTDIR)"
	$(RSC) $(RSC_PROJ) $(SOURCE)


SOURCE=.\gtTimer.cpp

"$(INTDIR)\gtTimer.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtWaveform.cpp

"$(INTDIR)\gtWaveform.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtWaveformChart.cpp

"$(INTDIR)\gtWaveformChart.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtXYChart.cpp

"$(INTDIR)\gtXYChart.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\gtXYGraph.cpp

"$(INTDIR)\gtXYGraph.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\SelectPoint.cpp

"$(INTDIR)\SelectPoint.obj" : $(SOURCE) "$(INTDIR)" "$(INTDIR)\gtLib.pch"


SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "gtLib - Win32 Release"

CPP_SWITCHES=/nologo /MD /W3 /GX /O2 /I "../Include" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /Fp"$(INTDIR)\gtLib.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\gtLib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ELSEIF  "$(CFG)" == "gtLib - Win32 Debug"

CPP_SWITCHES=/nologo /MDd /W3 /Gm /GX /ZI /Od /I "../Include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_AFXEXT" /D "_WINDLL" /Fp"$(INTDIR)\gtLib.pch" /Yc"stdafx.h" /Fo"$(INTDIR)\\" /Fd"$(INTDIR)\\" /FD /GZ /c 

"$(INTDIR)\StdAfx.obj"	"$(INTDIR)\gtLib.pch" : $(SOURCE) "$(INTDIR)"
	$(CPP) @<<
  $(CPP_SWITCHES) $(SOURCE)
<<


!ENDIF 


!ENDIF 

