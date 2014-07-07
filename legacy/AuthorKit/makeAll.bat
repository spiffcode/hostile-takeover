@echo off

set TS=%1
if "%TS%" == "" set TS=24
echo Input Tile size: %TS%
echo --- Extracting templates ---
TemplateExtractor -ts %TS% -n grassyNames.txt -tc grassy.tc -art grassyTemplates.png
echo.
echo --- Building 16-pixel tile .pdb ---
call updGrassy16
echo.
echo --- Building 24-pixel tile .pdb ---
call updGrassy24
echo.
echo --- Building 32-pixel tile .pdb ---
call updGrassy32
