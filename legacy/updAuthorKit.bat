copy bin\TemplateExtractor.exe authorkit
copy "game\win_debug\Warfare Incorporated.exe" authorkit
copy "game\palm_release\Warfare Incorporated.prc" authorkit
copy game\htdata816.pdb authorkit
copy game\htdata824.pdb authorkit
copy game\htsfx.pdb authorkit
copy bin\m.exe authorkit
copy bin\mcl.exe authorkit
copy bin\m.dll authorkit
copy bin\amx.dll authorkit
copy bin\palbin.exe authorkit
copy bin\shadowmap.exe authorkit
copy bin\packpdb2.exe authorkit
copy data\art824\fixed_8bpp.pal authorkit
copy docs\authorkit\AuthorKit.chm authorkit
copy game\res.h authorkit
copy data\GobTemplates.ini.pp authorkit
copy data\grassy.tc authorkit
copy data\desert.tc authorkit
if not exist authorkit\art824 mkdir authorkit\art824
copy data\art824\*.pal authorkit\art824
