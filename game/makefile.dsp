# Microsoft Developer Studio Project File - Name="makefile" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) External Target" 0x0106

CFG=makefile - Win32 Palm_Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "makefile.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "makefile.mak" CFG="makefile - Win32 Palm_Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "makefile - Win32 Palm_Release" (based on "Win32 (x86) External Target")
!MESSAGE "makefile - Win32 Palm_Debug" (based on "Win32 (x86) External Target")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName "makefile"
# PROP Scc_LocalPath "."

!IF  "$(CFG)" == "makefile - Win32 Palm_Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Palm_Release"
# PROP BASE Intermediate_Dir "Palm_Release"
# PROP BASE Cmd_Line "htmake REL=1 -j 2"
# PROP BASE Rebuild_Opt "clean"
# PROP BASE Target_File "ht.prc"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Palm_Release"
# PROP Intermediate_Dir "Palm_Release"
# PROP Cmd_Line "htmake.bat REL"
# PROP Rebuild_Opt "clean"
# PROP Target_File "ht.prc"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ELSEIF  "$(CFG)" == "makefile - Win32 Palm_Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Palm_Debug"
# PROP BASE Intermediate_Dir "Palm_Debug"
# PROP BASE Cmd_Line "htmake.bat -j 2"
# PROP BASE Rebuild_Opt "clean"
# PROP BASE Target_File "ht.prc"
# PROP BASE Bsc_Name ""
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Palm_Debug"
# PROP Intermediate_Dir "Palm_Debug"
# PROP Cmd_Line "htmake.bat"
# PROP Rebuild_Opt "clean"
# PROP Target_File "htd.prc"
# PROP Bsc_Name ""
# PROP Target_Dir ""

!ENDIF 

# Begin Target

# Name "makefile - Win32 Palm_Release"
# Name "makefile - Win32 Palm_Debug"

!IF  "$(CFG)" == "makefile - Win32 Palm_Release"

!ELSEIF  "$(CFG)" == "makefile - Win32 Palm_Debug"

!ENDIF 

# Begin Source File

SOURCE=.\htmake.bat
# End Source File
# Begin Source File

SOURCE=.\makefile
# End Source File
# End Target
# End Project
