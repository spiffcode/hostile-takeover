@echo off
set TS=%1
if "%TS%" == "" set TS=24
echo Tile size: %TS%
TemplateExtractor -ts %TS% -n grassyNames.txt -tc grassy.tc -art grassyTemplates.png -ter grassyTerrain.png
