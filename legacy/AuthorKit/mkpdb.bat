@echo off

REM Usage:
REM mkpdb <-pdb database> [-tc template collection] [-ld level] [-size size] [-nocleanup]
REM -pdb database: e.g., htdata816.pdb
REM -tc template collection: e.g., grassy.tc
REM -ld level: e.g., grassy_test.ld
REM -size size: tile size (16, 24, or 32)
REM -gt
REM -lvl level: e.g., grassy_test.lvl

REM Example:
REM mkpdb -pdb htdata816.pdb -tc enva.tc -ld enva.ld -size 16
REM or
REM mkpdb -pdb htdata824.pdb -gt

SETLOCAL

REM Miscellaneous constants

set DEPTH=8
set FIXED_PAL=fixed_%DEPTH%bpp.pal
set TMPDIR=mkdb_temp
set TOOLDIR=.
set CREATORID=WARI

REM Parse command-line arguments

:NextArg
if "%1"=="" (
	goto :ProcessArgs
) else if "%1"=="-tc" (
	set TC=%2
	shift
) else if "%1"=="-pdb" (
	set PDB=%2
	shift
) else if "%1"=="-ld" (
	set LD=%2
	shift
) else if "%1"=="-size" (
	set TILESIZE=%2
	shift
) else if "%1"=="-nocleanup" (
	set NOCLEANUP=1
) else if "%1"=="-gt" (
	set GOBTEMPLATES=1
) else if "%1"=="-lvl" (
	set LVL=%TMPDIR%\%2
	shift
) else (
	echo Error: unknown argument "%1"
	goto :EOF
)

shift
goto :NextArg

:ProcessArgs

REM Prompt user for any missing required arguments and fill in
REM defaults for missing optional ones.

if "%PDB%"=="" (
	set /P PDB="Enter database (.pdb) file name: "
)
if "%PDB%"=="" (
	echo Error: database file name required
	goto :EOF
)

if "%TILESIZE%"=="" (
	set TILESIZE=24
)
echo Tile size: %TILESIZE%

REM

set ARTDIR=art%DEPTH%%TILESIZE%
set ENV_PAL=%ARTDIR%\%TC:.tc=%_%DEPTH%bpp.pal

REM

md %TMPDIR%
cd %TMPDIR%
echo Unpacking database...
..\%TOOLDIR%\packpdb2 -u ..\%PDB% >stdout.txt
if %ERRORLEVEL% NEQ 0 (
	cd ..
	echo Error unpacking database
	goto :Error
)
del stdout.txt
cd ..

if NOT "%TC%"=="" (
	echo Calculating palette...
	%TOOLDIR%\mcl -makepal %TILESIZE% %TC% 256 128 32 %FIXED_PAL% %ENV_PAL%
	if %ERRORLEVEL% NEQ 0 (
		echo Error creating palette
		goto :Error
	)

	echo Crunching palette...
	%TOOLDIR%\palbin %ENV_PAL% %TMPDIR%\%TC:.tc=%.palbin
	if %ERRORLEVEL% NEQ 0 (
		echo Error crunching palette
		goto :Error
	)

	echo Creating shadow map...
	%TOOLDIR%\shadowmap %ENV_PAL% %TMPDIR%\%TC:.tc=%.palbin.shadowmap 0.6
	if %ERRORLEVEL% NEQ 0 (
		echo Error creating shadow map
		goto :Error
	)
)

if "%LVL%"=="" (
	set LVL=%TMPDIR%\%LD:.ld=%.lvl
)

if NOT "%LD%"=="" (
	echo Crunching level...
	%TOOLDIR%\mcl -levels %TILESIZE% %DEPTH% 0.8 0.9 0.9 1.1 1.3 %ARTDIR% %TMPDIR% %LD%
	if %ERRORLEVEL% NEQ 0 (
		echo Error compiling level
		goto :Error
	)
	%TOOLDIR%\cl /nologo /EP /FI ..\res.h %LVL% > ini.tmp
	if %ERRORLEVEL% NEQ 0 (
		echo Error preprocessing level
		goto :Error
	)
	%TOOLDIR%\inicrunch ini.tmp %LVL%
	if %ERRORLEVEL% NEQ 0 (
		del ini.tmp
		echo Error inicrunching level
		goto :Error
	)
	del ini.tmp
)

if NOT "%GOBTEMPLATES%"=="" (
	echo Crunching GobTemplates...
	%TOOLDIR%\cl /nologo /EP /I. GobTemplates.ini.pp > ini.tmp
	if %ERRORLEVEL% NEQ 0 (
		echo Error preprocessing GobTemplates
		goto :Error
	)
	%TOOLDIR%\inicrunch ini.tmp %TMPDIR%\GobTemplates.ini
	if %ERRORLEVEL% NEQ 0 (
		del ini.tmp
		echo Error inicrunching GobTemplates
		goto :Error
	)
	del ini.tmp
)

%TOOLDIR%\packpdb2 -p %CREATORID% %PDB% %TMPDIR%\*.* -nocompress *.*
if %ERRORLEVEL% NEQ 0 (
	echo Error packing database
	goto :Error
)
endlocal

echo :NoError
if "%NOCLEANUP%"=="" (
	@echo cleaning up...
	if exist %TMPDIR% rd /q /s %TMPDIR%
)
endlocal
set MKPDB_RET=0
goto :EOF

:Error
echo :Error
if "%NOCLEANUP%"=="" (
	@echo cleaning up...
	if exist %TMPDIR% rd /q /s %TMPDIR%
)
endlocal
set MKPDB_RET=1
