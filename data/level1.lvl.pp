#include <res.h>

[General]
Title=Our Very First Test Level!
TileMap=test.tmap
TerrainMap=test.trmap
Palette=palette1.palbin
InitialView=0,0
PlayerCredits=5000
WinCondition=5
LoseCondition=10

[kidfDefault]
FORM=(0 0 160 160) kidcOk LevelIntroBkgnd.tbm
LABEL=0 (4 4 0 0) "This test level is our simplest repre-" knfntTest
LABEL=0 (4 15 0 0) "sentation of a playable RTS game." knfntTest
LABEL=0 (4 29 0 0) "You are given 5,000 credits and a" knfntTest
LABEL=0 (4 40 0 0) "Human Resource Center capable of" knfntTest
LABEL=0 (4 51 0 0) "recruiting Short Range Infantry." knfntTest
LABEL=0 (4 66 0 0) "The enemy has the same capability." knfntTest
LABEL=0 (4 89 0 0) "Objective:" knfntTest
LABEL=0 (4 100 0 0) "Destroy the enemy base before they" knfntTest
LABEL=0 (4 111 0 0) "destroy yours!" knfntTest
BUTTON=kidcOk (52 140 0 0) "OK!" knfntTest

; GameObject prototypes:
; name=kgtSurfaceDecal,bitmap name,x,y[,flags]
; name=kgtScenery,bitmap name,x,y[,flags]	// UNDONE: animation instead of bitmap
; name=kgtShortRangeInfantry,side,x,y[,flags]
; name=kgtHumanResourceCenter,side,x,y[,flags]

[GameObjects]
nil=kgtHumanResourceCenter,ksideA,38,100
nil=kgtProcessor,ksideA,22,68
nil=kgtShortRangeInfantry,ksideA,30,56
nil=kgtShortRangeInfantry,ksideA,62,138
nil=kgtGalaxMiner,ksideA,46,154
nil=kgtGalaxMiner,ksideA,62,154

nil=kgtHumanResourceCenter,ksideB,170,158
nil=kgtReactor,ksideB,138,56
nil=kgtShortRangeInfantry,ksideB,142,40
nil=kgtShortRangeInfantry,ksideB,158,120
nil=kgtProcessor,ksideB,168,110
nil=kgtGalaxMiner,ksideB,158,154

nil=kgtScenery,tree2.tbm,80,100
nil=kgtScenery,tree2.tbm,197,67
nil=kgtScenery,tree2.tbm,242,289
nil=kgtScenery,tree2.tbm,194,407
nil=kgtScenery,tree2.tbm,208,219
nil=kgtScenery,tree2.tbm, 94,112
nil=kgtSurfaceDecal,tree2.tbm,395,306
nil=kgtScenery,tree2.tbm,266,290
