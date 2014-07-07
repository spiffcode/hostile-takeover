@echo off
call mkpdb -pdb htdata824.pdb -ld %1 -lvl %2
if "%MKPDB_RET%"=="0" (
	echo Launching Hostile Takeover...
	"Warfare Incorporated" -l %2
)
