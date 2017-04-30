#include <res.h>

[kidfSimUI]
	FORM=(0 0 160 152) kidcOk
	ALERT=kidcAlert (0 143 75 8)
	LABEL=kidcFps (74 0 12 0) "FPS" kifntShadow right
	LABEL=kidcObjective (0 9 0 0) "" kifntShadow
	LABEL=kidcCountdown (132 9 27 0) "" kifntShadow right
    BUTTON=kidcChat (128 2 40 12) "Chat" kifntShadow

[kidfContinueGame]
	FORM=(0 0 120 60) kidcOk
	LABEL=kidcTitle (0 3 120 8) "BEGIN NEW GAME" kifntTitle center
	LABEL=0 (12 15 100 0) "Would you like to replay the first two missions?" kifntDefault center multiline
	BUTTON=kidcOk (21 38 29 12) "Yes" kifntButton
	BUTTON=kidcCancel (72 38 27 12) "No" kifntButton

[kidfInputUI]
	FORM=(0 0 160 68) kidcOk
	FORMBACKCOLOR=kiclrFormBackground
	BUTTON=kidcMenuButton (0 0 23 7) "" kifntButton nocenter menuup.png menudown.png
	GRAFFITISCROLL=kidcGraffitiScroll (0 9 0 0) 4
	SILKBUTTON=kidcAppsSilkButton (0 0 0 0)
	SILKBUTTON=kidcMenuSilkButton (0 0 0 0)
	SILKBUTTON=kidcCalcSilkButton (0 0 0 0)
	SILKBUTTON=kidcFindSilkButton (0 0 0 0)
	POWER=kidcPower (86 0 40 0)
	CREDITS=kidcCreditsLabel (40 0 40 0)

[kidfMiniMap]
	FORM=(0 0 0 0) kidcOk
	MINIMAP=kidcMiniMap (0 0 0 0)

[kidfStartup]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	BITMAP=0 (28 7 0 0) title.png
	BUTTON=kidcPlay (65 59 110 14) "PLAY" kifntButton
	BUTTON=kidcLeaderboard (65 76 110 14) "LEADERBOARD" kifntButton
	BUTTON=kidcLoadSavedGame (65 93 110 14) "LOAD SAVED GAME" kifntButton
	BUTTON=kidcBuyMe (65 93 110 14) "PURCHASE" kifntButton
	BUTTON=kidcDownloadMissions (65 110 110 14) "ADD-ON MISSION PACKS" kifntButton
	BUTTON=kidcSetupGame (65 127 39 14) "OPTIONS" kifntButton
	BUTTON=kidcHelp (105 127 30 14) "HELP" kifntButton
	BUTTON=kidcForums (136 127 39 14) "FORUMS" kifntButton
	BITMAP=0 (81 150 0 0) copyright.png
	LABEL=kidcVersion (239 148 0 12) "" kifntShadow right

[kidfPlay]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (2 6 236 0) "PLAY GAME" kifntTitle center
	BUTTON=kidcPlaySinglePlayer (65 42 110 14) "SINGLE PLAYER" kifntButton
	BUTTON=kidcPlayMultiPlayer (65 59 110 14) "MULTIPLAYER" kifntButton
	BUTTON=kidcCancel (130 140 40 14) "BACK" kifntButton

[kidfDeleteMissionPack]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (28 6 104 0) "DELETE MISSION PACK" kifntTitle center
	LIST=kidcMissionPackList (3 19 153 115) kifntShadow
	BUTTON=kidcOk (31 140 40 12) "DELETE" kifntButton
	BUTTON=kidcCancel (86 140 40 12) "BACK" kifntButton

[kidfGameOptions]
	FORM=(0 0 160 160) kidcCancel center
	LABEL=0 (40 6 80 8) "OPTIONS" kifntTitle center
	BUTTON=kidcInGameOptions (30 22 100 14) "GAME" kifntButton
	BUTTON=kidcSoundOptions (30 40 100 14) "SOUND" kifntButton
	BUTTON=kidcPerformanceOptions (30 58 100 14) "PERFORMANCE" kifntButton
	BUTTON=kidcColorOptions (30 76 100 14) "COLOR" kifntButton
	BUTTON=kidcDisplayOptions (30 94 100 14) "DISPLAY" kifntButton
	BUTTON=kidcDeleteMissionPack (30 112 100 14) "DELETE MISSION PACK" kifntButton
	BUTTON=kidcCancel (90 140 40 14) "BACK" kifntButton

[kidfInGameOptions]
	FORM=(0 0 160 160) kidcCancel center
	LABEL=0 (38 6 83 0) "GAME OPTIONS" kifntTitle center
	LABEL=0 (11 22 68 0) "Game Speed" kifntShadow
	SLIDER=kidcGameSpeed (11 32 95 8)
	LABEL=kidcGameSpeedLabel (121 32 17 0) "0" kifntShadow right
	LABEL=0 (11 44 68 0) "Scroll Speed" kifntShadow
	SLIDER=kidcScrollSpeed (11 54 95 8)
	LABEL=kidcScrollSpeedLabel (121 54 17 0) "0" kifntShadow right
    LABEL=0 (11 66 68 0) "Max FPS" kifntShadow
	SLIDER=kidcMaxFPS (11 76 95 8)
	LABEL=kidcMaxFPSLabel (121 76 17 0) "0" kifntShadow right
	LABEL=0 (11 89 68 0) "Difficulty" kifntShadow
	CHECKBOX=kidcEasy (11 99 67 0) "Easy" kifntShadow
	CHECKBOX=kidcNormal (11 109 67 0) "Normal" kifntShadow
	CHECKBOX=kidcHard (11 119 67 0) "Hard" kifntShadow
	LABEL=0 (80 89 68 0) "Miscellaneous" kifntShadow
	CHECKBOX=kidcMuteSound (80 99 67 0) "Mute Sound" kifntShadow
	CHECKBOX=kidcLassoSelection (80 109 67 0) "Lasso Selection" kifntShadow
	BUTTON=kidcOk (13 140 40 12) "OK" kifntButton
	BUTTON=kidcCancel (60 140 40 12) "CANCEL" kifntButton
	BUTTON=kidcDefault (107 140 40 12) "DEFAULT" kifntButton

[kidfSoundOptions]
	FORM=(0 0 160 160) kidcCancel center
	LABEL=0 (38 6 83 0) "SOUND OPTIONS" kifntTitle center
	LABEL=kidcVolumeString (11 28 68 0) "Volume" kifntShadow
	SLIDER=kidcVol (11 42 95 8)
	LABEL=kidcVolLabel (121 42 17 0) "0" kifntShadow
	CHECKBOX=kidcMute (11 62 67 0) "Mute" kifntShadow
	BUTTON=kidcOk (13 140 40 12) "OK" kifntButton
	BUTTON=kidcCancel (60 140 40 12) "CANCEL" kifntButton

[kidfColorOptions]
	FORM=(0 92 160 68) kidcCancel center
	LABEL=0 (50 6 59 8) "COLOR OPTIONS" kifntTitle center
	LABEL=kidcHueLabelString (8 19 35 8) "Hue" kifntShadow right
	SLIDER=kidcHue (48 19 80 8)
	LABEL=kidcHueLabel (135 19 12 8) "0" kifntShadow
	LABEL=kidcSatLabelString (8 29 35 8) "Saturation" kifntShadow right
	SLIDER=kidcSat (48 29 80 8)
	LABEL=kidcSatLabel (135 29 12 8) "0" kifntShadow
	LABEL=kidcLumLabelString (8 39 35 8) "Brightness" kifntShadow right
	SLIDER=kidcLum (48 39 80 8)
	LABEL=kidcLumLabel (135 39 12 8) "0" kifntShadow
	BUTTON=kidcOk (13 52 40 12) "OK" kifntButton
	BUTTON=kidcCancel (60 52 40 12) "CANCEL" kifntButton
	BUTTON=kidcDefault (107 52 40 12) "DEFAULT" kifntButton

[kidfDisplayOptions]
	FORM=(0 0 160 160) kidcCancel center
	LABEL=0 (38 6 83 0) "DISPLAY OPTIONS" kifntTitle center
	LIST=kidcModesList (7 21 146 60) kifntShadow
	BUTTON=kidcOk (13 140 40 12) "OK" kifntButton
	BUTTON=kidcCancel (60 140 40 12) "CANCEL" kifntButton
	BUTTON=kidcDefault (107 140 40 12) "DEFAULT" kifntButton

[kidfPerformanceOptions]
	FORM=(0 0 160 160) kidcCancel center
	LABEL=0 (38 6 83 0) "PERFORMANCE OPTIONS" kifntTitle center
	CHECKBOX=kidcRocketShots (13 21 134 0) "Rocket Shots" kifntShadow
	CHECKBOX=kidcRocketTrails (13 32 134 0) "Rocket Trails" kifntShadow
	CHECKBOX=kidcRocketImpacts (13 43 134 0) "Rocket Impacts" kifntShadow
	CHECKBOX=kidcShots (13 54 134 0) "Gun & Tank Shots" kifntShadow
	CHECKBOX=kidcShotImpacts (13 65 134 0) "Gun & Tank Shot Impacts" kifntShadow
	CHECKBOX=kidcSelectionBrackets (13 76 134 0) "Selection Brackets" kifntShadow
	CHECKBOX=kidcSmoke (13 87 134 0) "Smoke" kifntShadow
	CHECKBOX=kidcEnemyDamageIndicator (13 98 134 0) "Enemy Damage Indicator" kifntShadow
	CHECKBOX=kidcScorchMarks (13 109 134 0) "Scorch Marks" kifntShadow
	CHECKBOX=kidcSymbolFlashing (13 120 134 0) "Power / Repair / Credits Symbol Flashing" kifntShadow
	BUTTON=kidcOk (13 140 40 12) "OK" kifntButton
	BUTTON=kidcCancel (60 140 40 12) "CANCEL" kifntButton
	BUTTON=kidcDefault (107 140 40 12) "DEFAULT" kifntButton

[kidfLogin]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "MULTIPLAYER LOGIN" kifntTitle center
	LABEL=kidcForums (42 28 52 0) "Enter Player Name and Password:" kifntShadow
	LABEL=kidcPlayerNameLabel (42 56 52 0) "Player Name:" kifntShadow
	EDIT=kidcPlayerName (88 56 90 0) "" kifntShadow
	BUTTON=kidcPlayerNamePanel (180 51 18 12) "..." kifntButton
	LABEL=kidcPasswordLabel (42 84 52 0) "Password:" kifntShadow
	EDIT=kidcPassword (88 84 90 0) "" kifntShadow
	BUTTON=kidcPasswordPanel (180 79 18 12) "..." kifntButton
	CHECKBOX=kidcAnonymous (42 112 57 0) "Login Anonymously" kifntShadow
	BUTTON=kidcLogin (10 140 36 12) "LOGIN" kifntButton
	BUTTON=kidcRegister (51 140 48 12) "REGISTER" kifntButton
	BUTTON=kidcUpdateAccount (104 140 80 12) "UPDATE ACCOUNT" kifntButton
	BUTTON=kidcCancel (189 140 40 12) "CANCEL" kifntButton

[kidfLobby]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "MULTIPLAYER LOBBY" kifntTitle center
	LABEL=kidcPlayerNameLabel (2 18 52 0) "Player name:" kifntShadow
    LABEL=kidcPlayerName (48 18 190 0) "" kifntShadow
    LABEL=kidcLurkerCount (238 18 0 0) "" kifntShadow right
	LIST=kidcRoomList (2 34 236 103) kifntShadow
	BUTTON=kidcJoinRoom (15 140 48 12) "JOIN ROOM" kifntButton
	BUTTON=kidcNewRoom (68 140 59 12) "CREATE ROOM" kifntButton
    BUTTON=kidcSignOut (132 140 48 12) "SIGN OUT" kifntButton
	BUTTON=kidcCancel (185 140 40 12) "BACK" kifntButton

[kidfCreateRoom]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "CREATE ROOM" kifntTitle center
	LABEL=0 (42 28 52 0) "Enter Room Name:" kifntShadow
	LABEL=kidcRoomNameLabel (42 56 52 0) "Room Name:" kifntShadow
	EDIT=kidcRoomName (88 56 90 0) "" kifntShadow
	BUTTON=kidcRoomNamePanel (180 51 18 12) "..." kifntButton
	LABEL=kidcPasswordLabel (42 84 52 0) "Password:" kifntShadow
	EDIT=kidcPassword (88 84 90 0) "" kifntShadow
	BUTTON=kidcPasswordPanel (180 79 18 12) "..." kifntButton
	CHECKBOX=kidcPrivate (42 112 57 0) "Private" kifntShadow
	BUTTON=kidcOk (75 140 40 12) "OK" kifntButton
	BUTTON=kidcCancel (125 140 40 12) "CANCEL" kifntButton

[kidfRoom]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "GAME ROOM" kifntTitle center
    LABEL=kidcRoomName (2 18 236 0) "" kifntShadow
    LABEL=kidcNoGames (2 76 236 0) "No games in this room. To create one, press Create Game." kifntShadow center
	LIST=kidcGameList (2 34 236 103) kifntShadow
	BUTTON=kidcJoinGame (11 140 48 12) "JOIN GAME" kifntButton
	BUTTON=kidcNewGame (69 140 59 12) "CREATE GAME" kifntButton
    BUTTON=kidcChat (138 140 40 12) "CHAT" kifntButton
	BUTTON=kidcCancel (188 140 40 12) "BACK" kifntButton

[kidfCreateGame]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (31 6 99 0) "HOST MULTIPLAYER GAME" kifntTitle center
	LABEL=kidcGameNameLabel (2 18 46 0) "Game Name:" kifntShadow
	EDIT=kidcGameName (50 18 88 0) "<unimplemented>" kifntShadow
	BUTTON=kidcGameNamePanel (140 13 18 12) "..." kifntButton
	LABEL=0 (2 30 50 0) "Game Speed:" kifntShadow
	SLIDER=kidcGameSpeed (50 30 88 8)
	LABEL=kidcGameSpeedLabel (140 30 17 0) "0" kifntShadow right
	LABEL=0 (2 42 18 0) "Choose a map to play on:" kifntShadow
	LIST=kidcMapList (2 52 156 76) kifntShadow
	BUTTON=kidcOk (17 140 60 12) "CREATE GAME" kifntButton
	BUTTON=kidcCancel (93 140 50 12) "CANCEL" kifntButton

[kidfCreateGameWide]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "CREATE MULTIPLAYER GAME" kifntTitle center
	LABEL=0 (42 18 50 0) "Game Speed:" kifntShadow
	SLIDER=kidcGameSpeed (90 18 88 8)
	LABEL=kidcGameSpeedLabel (180 18 17 0) "0" kifntShadow right
	RADIOBUTTONBAR=kidcCategories (0 32 0 0) "Challenge|Add-On" kifntShadow
	LIST=kidcChallengeList (2 47 236 79) kifntShadow
	LIST=kidcAddOnList (2 47 236 79) kifntShadow
	LABEL=kidcMissionPackInfo (2 127 236 12) "" kifntShadow
	LABEL=kidcAddOnMessage (2 114 236 0) "Install Add-On Mission Packs from the Main Menu" kifntShadow center
	BUTTON=kidcOk (35 141 60 12) "CREATE GAME" kifntButton
    BUTTON=kidcChat(105 141 40 12) "CHAT" kifntButton
	BUTTON=kidcCancel (155 141 50 12) "CANCEL" kifntButton

[kidfGameStart]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (74 6 91 0) "WAIT FOR JOINING PLAYERS" kifntTitle center
	LABEL=0 (42 18 23 0) "Game:" kifntShadow
	LABEL=kidcGameName (84 18 10 0) "Acquiring game details..." kifntShadow
	LABEL=0 (42 29 18 0) "Map:" kifntShadow
	LABEL=kidcMapName (84 29 10 0) "" kifntShadow
	LABEL=0 (42 40 37 0) "# players:" kifntShadow
	LABEL=kidcNumPlayers (84 40 10 0) "" kifntShadow
	LABEL=0 (122 40 37 0) "Game Speed:" kifntShadow
	LABEL=kidcGameSpeedLabel (172 40 37 0) "" kifntShadow
	LABEL=0 (104 66 32 0) "PLAYERS" kifntTitle center
	LIST=kidcPlayerList (52 76 136 57) kifntShadow
	BUTTON=kidcOk (40 140 50 12) "READY" kifntButton
    BUTTON=kidcChat (100 140 40 12) "CHAT" kifntButton
	BUTTON=kidcCancel (150 140 50 12) "CANCEL" kifntButton

[kidfPickLevel]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (28 6 104 0) "SELECT MISSION" kifntTitle center
	LIST=kidcLevelList (3 19 153 97) kifntShadow
	LABEL=0 (80 118 0 0) "For more missions go to:" kifntShadow center
	LABEL=0 (80 128 0 0) "http://www.WarfareIncorporated.com" kifntShadow center
	BUTTON=kidcOk (31 140 40 12) "PLAY" kifntButton
	BUTTON=kidcCancel (86 140 40 12) "BACK" kifntButton

[kidfSelectMissionWide]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "PLAY SINGLE PLAYER MISSION" kifntTitle center
	RADIOBUTTONBAR=kidcCategories (0 19 0 0) "Story|Challenge|Add-On" kifntShadow
	LIST=kidcStoryList (2 33 236 93) kifntShadow
	LIST=kidcChallengeList (2 33 236 93) kifntShadow
	LIST=kidcAddOnList (2 33 236 93) kifntShadow
	LABEL=kidcMissionPackInfo (2 127 236 12) "" kifntShadow
	LABEL=kidcAddOnMessage (2 114 236 0) "Install Add-On Mission Packs from the Main Menu" kifntShadow center
	BUTTON=kidcOk (71 141 40 12) "PLAY" kifntButton
	BUTTON=kidcCancel (126 141 40 12) "BACK" kifntButton

[kidfAddOnSingleMulti]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (2 6 236 0) "ADD-ON MISSION PACKS" kifntTitle center
	BUTTON=kidcPlaySinglePlayer (55 42 120 14) "SINGLE PLAYER MISSION PACKS" kifntButton
	BUTTON=kidcPlayMultiPlayer (55 59 120 14) "MULTIPLAYER MISSION PACKS" kifntButton
	BUTTON=kidcCancel (130 140 40 14) "BACK" kifntButton

[kidfDownloadMissionPackWide]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "DOWNLOAD ADD-ON MISSION PACKS" kifntTitle center
	LIST=kidcMissionPackList (2 24 236 86) kifntShadow
	LABEL=kidcStatus (0 16 0 0) "+" kifntShadow
	LABEL=kidcTitle (0 16 0 0) "Title" kifntShadow
	LABEL=kidcNumPlayers (0 16 0 0) "# Players" kifntShadow
	LABEL=kidcNumMissions (0 16 0 0) "# Missions" kifntShadow
	LABEL=kidcMissionPackInfo (2 112 236 25) "" kifntShadow multiline clipvert
	BUTTON=kidcOk (40 141 55 12) "DOWNLOAD" kifntButton
	BUTTON=kidcDiscuss (103 141 48 12) "DISCUSS" kifntButton
	BUTTON=kidcCancel (159 141 40 12) "DONE" kifntButton

[kidfChooseServer]
	FORM=(0 0 240 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (0 6 240 0) "CHOOSE SERVER" kifntTitle center
	LABEL=kidcServerName (0 20 0 0) "Name" kifntShadow
	LABEL=kidcServerLocation (0 20 0 0) "Location" kifntShadow
	LABEL=kidcServerStatus (0 20  0 0) "Status" kifntShadow
	LABEL=kidcNumPlayers (0 20 0 0) "# Players" kifntShadow
	LIST=kidcServerList (2 32 236 111) kifntShadow
	BUTTON=kidcOk (40 141 50 12) "CONNECT" kifntButton
	BUTTON=kidcRefresh (100 141 50 12) "REFRESH" kifntButton
	BUTTON=kidcCancel (160 141 40 12) "BACK" kifntButton

[kidfPickTransport]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (28 6 104 0) "CHOOSE A TRANSPORT" kifntTitle center
	BUTTON=kidcTransport1 (30 34 100 14) "TRANSPORT 1" kifntButton
	BUTTON=kidcTransport2 (30 51 100 14) "TRANSPORT 2" kifntButton
	BUTTON=kidcTransport3 (30 68 100 14) "TRANSPORT 3" kifntButton
	BUTTON=kidcTransport4 (30 85 100 14) "TRANSPORT 4" kifntButton
	BUTTON=kidcTransport5 (30 102 100 14) "TRANSPORT 5" kifntButton
	BUTTON=kidcTransport6 (30 119 100 14) "TRANSPORT 6" kifntButton
	LABEL=kidcNoTransportsAvailable (10 73 140 0) "No supported communication methods are available." kifntShadow center multiline
	BUTTON=kidcCancel (90 140 40 12) "BACK" kifntButton

[kidfInGameMenu]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (62 6 35 0) "MENU" kifntTitle center
	BUTTON=kidcObjectives (30 19 100 14) "STATE OBJECTIVES" kifntButton
	BUTTON=kidcOptions (30 36 100 14) "OPTIONS" kifntButton
	BUTTON=kidcSaveGame (30 53 100 14) "SAVE GAME" kifntButton
	BUTTON=kidcLoadGame (30 70 100 14) "LOAD GAME" kifntButton
	BUTTON=kidcRestartMission (30 87 100 14) "RESTART MISSION" kifntButton
	BUTTON=kidcAbortMission (30 104 100 14) "ABORT MISSION" kifntButton
	BUTTON=kidcChat (30 121 100 14) "CHAT" kifntButton
	BUTTON=kidcExitGame (30 121 100 14) "EXIT WARFARE" kifntButton
	BUTTON=kidcHelp (30 138 48 14) "HELP" kifntButton
	BUTTON=kidcCancel (82 138 48 14) "BACK" kifntButton

[kidfTestOptions]
	FORM=(0 0 160 151) kidcOk center
	LABEL=0 (53 2 54 0) "TEST OPTIONS" kifntTitle center
	CHECKBOX=kidcDrawPaths (2 15 57 0) "Draw paths" kifntShadow
	CHECKBOX=kidcDrawLines (2 24 82 0) "Draw target lines" kifntShadow
	CHECKBOX=kidcOvermind (2 33 77 0) "Enable Overmind" kifntShadow
	CHECKBOX=kidcMaxRepaint (2 42 59 0) "Max repaint" kifntShadow
	CHECKBOX=kidcShowFPS (2 51 48 0) "Show FPS" kifntShadow
	CHECKBOX=kidcGodMode (2 60 50 0) "God mode" kifntShadow
	CHECKBOX=kidcSuspendUpdates (2 69 81 0) "Suspend Updates" kifntShadow
	CHECKBOX=kidcAutosave (2 78 81 0) "Autosave" kifntShadow
	CHECKBOX=kidcDrawUpdateRects (2 87 70 0) "Draw Update Rects" kifntShadow
	CHECKBOX=kidcStylusUI (2 96 56 0) "Stylus UI" kifntShadow
	CHECKBOX=kidcShowStats (2 96 56 0) "Show stats" kifntShadow
	CHECKBOX=kidcLockStep (2 105 82 0) "Lock Step Simulations" kifntShadow
	BUTTON=kidcClearFog (104 14 54 12) "Clear Fog" kifntButton
	BUTTON=kidcMemoryUse (104 30 54 12) "Memory..." kifntButton
	BUTTON=kidcHelp (104 46 54 12) "Help" kifntButton
	BUTTON=kidcGobCount (104 62 54 12) "Counts..." kifntButton
	BUTTON=kidcBreak (104 78 54 12) "Break" kifntButton
	BUTTON=kidcOk (51 135 54 12) "OK" kifntButton

[kidfMemoryUse]
	FORM=(0 0 160 151) kidcOk center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (55 2 50 0) "MEMORY USE" kifntTitle center
	LABEL=kidcDynDbInitial (2 15 0 0) "" kifntShadow
	LABEL=kidcDynUse (2 25 0 0) "" kifntShadow
	LABEL=kidcMmgrDynDbReserve (2 35 0 0) "" kifntShadow
	LABEL=kidcMmgrUse (2 45 0 0) "" kifntShadow
	LABEL=kidcCacheUse (2 65 0 0) "" kifntShadow
	BUTTON=kidcClearCache (2 79 70 12) "Clear Cache" kifntButton
	CHECKBOX=kidcLimitCache (2 96 56 0) "Limit Cache" kifntShadow
	BUTTON=kidcAdd10KCache (2 110 25 12) "+10k" kifntButton
	BUTTON=kidcSub10KCache (31 110 25 12) "-10k" kifntButton
	LABEL=kidcCacheLimit (2 127 50 0) "Cache Limit: " kifntShadow
	BUTTON=kidcOk (51 135 54 12) "OK" kifntButton

[kidfObjectives]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "OBJECTIVES" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=kidcPage1 (80 63 0 0) "BRIEFING" kifntTitle center
	LABEL=kidcPage2 (80 63 0 0) "STATISTICS" kifntTitle center
	LABEL=kidcObjectiveInfo (4 74 152 69) "" kifntShadow left multiline
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 128 0 0) "Title" kifntShadow
	BUTTON=kidcStatistics (50 140 60 14) "STATISTICS" kifntButton center
	LABEL=kidcRankTitle (155 128 0 0) "{$ranktitle}" kifntShadow right
	BUTTON=kidcInfo (50 140 60 14) "BRIEFING" kifntButton center
	BUTTON=kidcCancel (120 140 40 14) "OK" kifntButton center

[kidfWinSummary]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "PERFORMANCE REVIEW" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=kidcMissionResult (80 63 0 0) "OBJECTIVES COMPLETED!" kifntTitle center
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 128 0 0) "Title" kifntShadow
	LABEL=kidcRankTitle (155 128 0 0) "{$ranktitle}" kifntShadow right
	BUTTON=kidcCancel (80 140 40 14) "OK" kifntButton center

[kidfLoseSummary]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "PERFORMANCE REVIEW" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=0 (80 63 0 0) "MISSION FAILED" kifntTitle center
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 128 0 0) "Title" kifntShadow
	LABEL=kidcRankTitle (155 128 0 0) "{$ranktitle}" kifntShadow right
	BUTTON=kidcRestartMission (3 140 40 14) "RETRY" kifntButton
	BUTTON=kidcAbortMission (50 140 40 14) "ABORT" kifntButton
	BUTTON=kidcLoadGame (97 140 60 14) "LOAD SAVED" kifntButton

[kidfBuildInfantry]
	FORM=(0 0 160 143) kidcCancel
	LABEL=0 (2 5 82 0) "RECRUIT PERSONNEL" kifntTitle
	BUTTON=kidcHelp (135 3 10 12) "" kifntButton nocenter helpup.png helpdown.png
	BUTTON=kidcCancel (147 3 10 12) "" kifntButton nocenter cancelup.png canceldown.png
	ANIMLIST=kidcList (-1 17 28 121) kifntShadow 17
	LABEL=kidcDescription (30 79 127 8) "" kifntShadow left multiline
	LABEL=kidcName (30 19 75 0) "" kifntShadow
	LABEL=0 (30 31 22 0) "COST" kifntShadow
	LABEL=0 (30 40 22 0) "ARMOR" kifntShadow
	LABEL=0 (30 49 22 0) "DAMAGE" kifntShadow
	LABEL=0 (30 58 22 0) "SPEED" kifntShadow
	LABEL=0 (30 67 22 0) "RANGE" kifntShadow
	LABEL=kidcCost (136 31 22 0) "" kifntShadow right
	LABEL=kidcArmorStrength (136 40 22 0) "" kifntShadow right
	LABEL=kidcMoveRate (136 58 22 0) "" kifntShadow right
	LABEL=kidcWeaponRange (136 67 22 0) "" kifntShadow right
	LABEL=kidcLimitReached (33 123 32 12) "Limit Reached" kifntShadow multiline center
	BUTTON=kidcOrder (30 124 32 12) "Order" kifntButton
	BUTTON=kidcCancelOrder (65 124 59 12) "Cancel Order" kifntButton
	BUTTON=kidcOk (127 124 31 12) "Close" kifntButton
	PIPMETER=kidcCostMeter (64 31 71 9)
	PIPMETER=kidcArmorStrengthMeter (64 40 71 9)
	DAMAGEMETER=kidcWeaponStrengthMeter (64 49 94 9)
	PIPMETER=kidcMoveRateMeter (64 58 71 9)
	PIPMETER=kidcWeaponRangeMeter (64 67 71 9)

[kidfBuildVehicle]
	FORM=(0 0 160 143) kidcCancel
	LABEL=0 (2 5 82 0) "ORDER VEHICLE" kifntTitle
	BUTTON=kidcHelp (135 3 10 12) "" kifntButton nocenter helpup.png helpdown.png
	BUTTON=kidcCancel (147 3 10 12) "" kifntButton nocenter cancelup.png canceldown.png
	ANIMLIST=kidcList (-1 17 28 121) kifntShadow 17
	LABEL=kidcDescription (30 79 127 8) "" kifntShadow left multiline
	LABEL=kidcName (30 19 75 0) "" kifntShadow
	LABEL=0 (30 31 22 0) "COST" kifntShadow
	LABEL=0 (30 40 22 0) "ARMOR" kifntShadow
	LABEL=0 (30 49 22 0) "DAMAGE" kifntShadow
	LABEL=0 (30 58 22 0) "SPEED" kifntShadow
	LABEL=0 (30 67 22 0) "RANGE" kifntShadow
	LABEL=kidcCost (136 31 22 0) "" kifntShadow right
	LABEL=kidcArmorStrength (136 40 22 0) "" kifntShadow right
	LABEL=kidcMoveRate (136 58 22 0) "" kifntShadow right
	LABEL=kidcWeaponRange (136 67 22 0) "" kifntShadow right
	LABEL=kidcLimitReached (33 123 32 12) "Limit Reached" kifntShadow multiline center
	BUTTON=kidcOrder (30 124 32 12) "Order" kifntButton
	BUTTON=kidcCancelOrder (65 124 59 12) "Cancel Order" kifntButton
	BUTTON=kidcOk (127 124 31 12) "Close" kifntButton
	PIPMETER=kidcCostMeter (64 31 71 9)
	PIPMETER=kidcArmorStrengthMeter (64 40 71 9)
	DAMAGEMETER=kidcWeaponStrengthMeter (64 49 94 9)
	PIPMETER=kidcMoveRateMeter (64 58 71 9)
	PIPMETER=kidcWeaponRangeMeter (64 67 71 9)

[kidfBuildStructure]
	FORM=(0 0 160 143) kidcCancel
	LABEL=0 (2 5 82 0) "ORDER BUILDING" kifntTitle
	BUTTON=kidcHelp (135 3 10 12) "" kifntButton nocenter helpup.png helpdown.png
	BUTTON=kidcCancel (147 3 10 12) "" kifntButton nocenter cancelup.png canceldown.png
	ANIMLIST=kidcList (-1 17 28 121) kifntShadow 18
	LABEL=kidcDescription (30 79 127 8) "" kifntShadow left multiline
	LABEL=kidcName (30 19 75 0) "UNIT NAME" kifntShadow
	LABEL=0 (30 31 22 0) "COST" kifntShadow
	LABEL=0 (30 40 22 0) "ARMOR" kifntShadow
	LABEL=0 (30 49 22 0) "DAMAGE" kifntShadow
	LABEL=0 (30 58 22 0) "P DEMAND" kifntShadow
	LABEL=0 (30 67 22 0) "P SUPPLY" kifntShadow
	LABEL=kidcCost (136 31 22 0) "2000" kifntShadow right
	LABEL=kidcArmorStrength (136 40 22 0) "HEAVY" kifntShadow right
	LABEL=kidcPowerDemand (136 58 22 0) "500" kifntShadow right
	LABEL=kidcPowerSupply (136 67 22 0) "0" kifntShadow right
	LABEL=kidcLimitReached (30 127 93 12) "Building Limit Reached" kifntShadow center
	BUTTON=kidcOk (86 124 34 12) "Build" kifntButton
	BUTTON=kidcCancel (124 124 34 12) "Close" kifntButton
	PIPMETER=kidcCostMeter (64 31 71 9)
	PIPMETER=kidcArmorStrengthMeter (64 40 71 9)
	DAMAGEMETER=kidcWeaponStrengthMeter (64 49 94 9)
	PIPMETER=kidcPowerDemandMeter (64 58 71 9)
	PIPMETER=kidcPowerSupplyMeter (64 67 71 9)

[kidfUpgrade]
	FORM=(0 0 160 143) kidcCancel
	LABEL=0 (2 5 82 0) "R&D TECHNOLOGIES" kifntTitle
	BUTTON=kidcHelp (135 3 10 12) "" kifntButton nocenter helpup.png helpdown.png
	BUTTON=kidcCancel (147 3 10 12) "" kifntButton nocenter cancelup.png canceldown.png
	ANIMLIST=kidcList (-1 17 28 121) kifntShadow 18
	LABEL=kidcName (30 19 75 0) "UPGRADE NAME" kifntShadow
	LABEL=kidcCostLabel (30 31 22 0) "COST" kifntShadow
	LABEL=kidcCost (136 31 22 0) "2000" kifntShadow right
	LABEL=kidcDescription (30 49 127 8) "" kifntShadow left multiline
	LABEL=kidcPrerequisitesLabel (30 97 22 0) "PREREQUISITES" kifntShadow
	LABEL=kidcPrerequisites (30 106 127 8) "" kifntShadow left multiline
	BUTTON=kidcOk (78 124 42 12) "Research" kifntButton
	BUTTON=kidcCancel (124 124 34 12) "Close" kifntButton
	PIPMETER=kidcCostMeter (64 31 71 9)

[kidfPlaceStructure]
	FORM=(0 0 320 320) kidcCancel
	BUTTON=kidcOk (120 135 10 12) "" kifntButton nocenter okup.png okdown.png
	BUTTON=kidcCancel (0 135 10 12) "" kifntButton nocenter cancelup.png canceldown.png

[kidfHrcMenu]
	FORM=(0 0 66 68) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcBuild (3 14 60 12) "" kifntButton nocenter build_btn_up.png build_btn_down.png build_btn_disabled.png
	BUTTON=kidcRepair (3 31 60 12) "" kifntButton nocenter repair_btn_up.png repair_btn_down.png repair_btn_disabled.png
	BUTTON=kidcAbortRepair (3 31 60 12) "" kifntButton nocenter abort_repair_btn_up.png abort_repair_btn_down.png
	BUTTON=kidcSelfDestruct (3 48 60 12) "" kifntButton nocenter sell_btn_up.png sell_btn_down.png sell_btn_disabled.png

[kidfVtsMenu]
	FORM=(0 0 66 68) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcBuild (3 14 60 12) "" kifntButton nocenter build_btn_up.png build_btn_down.png build_btn_disabled.png
	BUTTON=kidcRepair (3 31 60 12) "" kifntButton nocenter repair_btn_up.png repair_btn_down.png repair_btn_disabled.png
	BUTTON=kidcAbortRepair (3 31 60 12) "" kifntButton nocenter abort_repair_btn_up.png abort_repair_btn_down.png
	BUTTON=kidcSelfDestruct (3 48 60 12) "" kifntButton nocenter sell_btn_up.png sell_btn_down.png sell_btn_disabled.png

[kidfHqMenu]
	FORM=(0 0 66 68) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcBuild (0 14 0 0) "" kifntButton nocenter build_btn_up.png build_btn_down.png build_btn_disabled.png
	BUTTON=kidcAbortBuild (0 14 0 0) "" kifntButton nocenter abort_build_btn_up.png abort_build_btn_down.png
	BUTTON=kidcRepair (14 14 0 0) "" kifntButton nocenter repair_btn_up.png repair_btn_down.png repair_btn_disabled.png
	BUTTON=kidcAbortRepair (14 14 0 0) "" kifntButton nocenter abort_repair_btn_up.png abort_repair_btn_down.png
	BUTTON=kidcSelfDestruct (28 14 0 0) "" kifntButton nocenter sell_btn_up.png sell_btn_down.png sell_btn_disabled.png

[kidfResearchMenu]
	FORM=(0 0 76 68) kidcCancel
	LABEL=kidcTitle (0 2 76 0) "" kifntTitle center
	BUTTON=kidcResearch (3 14 60 12) "" kifntButton nocenter build_btn_up.png build_btn_down.png build_btn_disabled.png
	BUTTON=kidcAbortUpgrade (3 14 60 12) "" kifntButton nocenter abort_build_btn_up.png abort_build_btn_down.png
	BUTTON=kidcRepair (3 31 60 12) "" kifntButton nocenter repair_btn_up.png repair_btn_down.png repair_btn_disabled.png
	BUTTON=kidcAbortRepair (3 31 60 12) "" kifntButton nocenter abort_repair_btn_up.png abort_repair_btn_down.png
	BUTTON=kidcSelfDestruct (3 48 60 12) "" kifntButton nocenter sell_btn_up.png sell_btn_down.png sell_btn_disabled.png

[kidfStructMenu]
	FORM=(0 0 66 51) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcRepair (3 31 60 12) "" kifntButton nocenter repair_btn_up.png repair_btn_down.png repair_btn_disabled.png
	BUTTON=kidcAbortRepair (3 31 60 12) "" kifntButton nocenter abort_repair_btn_up.png abort_repair_btn_down.png
	BUTTON=kidcSelfDestruct (3 48 60 12) "" kifntButton nocenter sell_btn_up.png sell_btn_down.png sell_btn_disabled.png

[kidfUnitMenu]
	FORM=(0 0 66 51) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center

[kidfMobileHqMenu]
	FORM=(0 0 66 51) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcTransform (3 14 60 12) "Transform" kifntButton
	LABEL=kidcCantTransform (2 14 62 24) "Can't Transform. Move to clear area." kifntShadow left multiline

[kidfMinerMenu]
	FORM=(0 0 66 51) kidcCancel
	LABEL=kidcTitle (0 2 66 0) "" kifntTitle center
	BUTTON=kidcDeliver (3 14 60 12) "Deliver" kifntButton

[kidfMessageBox]
	FORM=(0 0 140 50) kidcOk center topmost
	LABEL=kidcTitle (61 2 18 0) "Title" kifntTitle center
	LABEL=kidcMessage (6 15 128 8) "Message" kifntShadow center multiline
	BUTTON=kidcOk (44 32 54 12) "OK" kifntButton

[kidfMessageBoxQuery]
	FORM=(0 0 140 50) kidcOk center
	LABEL=kidcTitle (61 2 18 0) "Title" kifntTitle center
	LABEL=kidcMessage (6 15 128 8) "Message" kifntShadow center multiline
	BUTTON=kidcOk (10 32 54 12) "Yes" kifntButton
	BUTTON=kidcCancel (74 32 54 12) "No" kifntButton

[kidfDownloadBox]
	FORM=(0 0 190 70) kidcOk center topmost
	LABEL=kidcTitle (61 2 18 0) "Download Mission Pack" kifntTitle center
	LABEL=kidcMessage (6 15 178 8) "Message" kifntShadow center multiline
	LABEL=kidcStatus (6 33 178 8) "Status" kifntShadow center multiline
	BUTTON=kidcOk (36 52 54 12) "OK" kifntButton
	BUTTON=kidcPlay (100 52 54 12) "PLAY" kifntButton

[kidfBluetoothPatchQuery]
	FORM=(0 0 140 50) kidcOk center
	LABEL=kidcTitle (61 2 18 0) "Title" kifntTitle center
	LABEL=kidcMessage (6 15 128 8) "Message" kifntShadow center multiline
	BUTTON=kidcOk (6 32 69 12) "Already installed" kifntButton
	BUTTON=kidcCancel (80 32 54 12) "Cancel" kifntButton

[kidfLoadGame]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LIST=kidcGameList (3 19 153 115) kifntShadow
	LABEL=0 (38 6 84 0) "LOAD SAVED GAME" kifntTitle center
	BUTTON=kidcOk (31 140 40 12) "LOAD" kifntButton
	BUTTON=kidcCancel (86 140 40 12) "BACK" kifntButton

[kidfSaveGame]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LIST=kidcGameList (3 19 153 115) kifntShadow
	LABEL=0 (38 6 84 0) "SAVE GAME" kifntTitle center
	BUTTON=kidcOk (31 140 40 12) "Save" kifntButton
	BUTTON=kidcCancel (86 140 40 12) "Cancel" kifntButton

[kidfAreYouSure]
	FORM=(0 0 73 45) kidcCancel
	LABEL=kidcTitle (0 2 73 0) "SELL BUILDING" kifntTitle center
	LABEL=0 (12 15 55 0) "Are you sure?" kifntShadow center
	BUTTON=kidcOk (4 29 29 12) "Yes" kifntButton
	BUTTON=kidcCancel (42 29 27 12) "No" kifntButton

[kidfEcomLarge]
	FORM=(0 0 160 150) kidcOk
	LABEL=kidcFrom (3 19 56 0) "Olstrom@@acme" kifntShadow
	LABEL=kidcTo (3 34 54 0) "Olstrom@@acme" kifntShadow
	BITMAP=kidcFromBitmap (80 4 0 0) olstrom.png
	BITMAP=kidcToBitmap (121 4 0 0) jana.png
	ECOMTEXT=kidcMessage (4 50 152 81) "EcomText" kifntEcom left
	BUTTON=kidcOk (108 135 50 12) "OK" kifntButton

[kidfEcomSmall]
	FORM=(0 0 160 75) kidcOk
	LABEL=kidcFrom (30 6 49 0) "olstrom@@acme" kifntShadow
	LABEL=kidcTo (96 6 53 0) "olstrom@@acme" kifntShadow
	ECOMTEXT=kidcMessage (4 16 152 40) "EcomText" kifntEcom left
	BUTTON=kidcOk (108 60 50 12) "OK" kifntButton

[kidfHelp]
	FORM=(0 0 160 160) kidcOk center
	HELP=kidcHelp (5 6 150 130)
	BUTTON=kidcOk (5 140 45 12) "OK" kifntButton
	BUTTON=kidcIndex (55 140 45 12) "Index" kifntButton
	BUTTON=kidcBack (119 142 6 6) "" kifntButton nocenter backup.png backdown.png
	BUTTON=kidcNextPage (130 144 4 4) "" kifntButton nocenter scrolldownup.png scrolldowndown.png
	BUTTON=kidcPrevPage (140 144 4 4) "" kifntButton nocenter scrollupup.png scrollupdown.png

[kidfHelpWide]
	FORM=(0 0 240 160) kidcOk center
	HELP=kidcHelp (5 6 228 130)
	BUTTON=kidcOk (70 140 45 12) "OK" kifntButton
	BUTTON=kidcIndex (125 140 45 12) "Index" kifntButton
	BUTTON=kidcBack (189 142 6 6) "" kifntButton nocenter backup.png backdown.png
	BUTTON=kidcNextPage (200 144 4 4) "" kifntButton nocenter scrolldownup.png scrolldowndown.png
	BUTTON=kidcPrevPage (210 144 4 4) "" kifntButton nocenter scrollupup.png scrollupdown.png

[kidfCutScene]
	FORM=(0 0 160 160) kidcOk center
	BITMAP=kidcBitmap (5 5 150 0)
	ECOMTEXT=kidcMessage (5 51 150 70) "EcomText" kifntShadow left
	BUTTON=kidcOk (105 141 50 12) "More..." kifntButton

[kidfGobCount]
	FORM=(0 0 160 160) kidcOk center
	FORMBACKCOLOR=kiclrFormBackground

[kidfDrmCode]
	FORM=(0 0 160 160) kidcExitGame center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (80 9 0 0) "HANDMARK PRESENTS" kifntTitle center
	BITMAP=0 (0 17 0 0) title.png
	LABEL=0 (30 60 100 8) "Copyright 2003, 2004 Spiffcode, Inc." kifntDefault center
	LABEL=0 (30 70 100 8) "All Rights Reserved" kifntDefault center
	LABEL=0 (8 91 143 8) "Purchase at www.WarfareIncorporated.com" kifntDefault center
	LABEL=kidcCode (30 104 100 8) "C-AAAA-BBBB-CCCC-DDDD" kifntShadow center
	BUTTON=kidcEnterKey (42 119 74 12) "Enter License Key" kifntDefault
	BUTTON=kidcPlayDemo (42 137 74 12) "Play Demo" kifntDefault

[kidfDrmKey]
	FORM=(0 0 160 160) kidcOk center
	LABEL=0 (42 6 75 8) "HOSTILE TAKEOVER" kifntTitle center
	LABEL=0 (28 20 104 8) "Please enter your License Key:" kifntDefault center
	LABEL=kidcKey (35 35 87 8) "K-??????-??????-??????" kifntShadow center
	BUTTON=kidc0 (6 50 34 14) "0" kifntTitle
	BUTTON=kidc1 (44 50 34 14) "1" kifntTitle
	BUTTON=kidc2 (83 50 34 14) "2" kifntTitle
	BUTTON=kidc3 (121 50 34 14) "3" kifntTitle
	BUTTON=kidc4 (6 67 34 14) "4" kifntTitle
	BUTTON=kidc5 (44 67 34 14) "5" kifntTitle
	BUTTON=kidc6 (83 67 34 14) "6" kifntTitle
	BUTTON=kidc7 (121 67 34 14) "7" kifntTitle
	BUTTON=kidc8 (6 84 34 14) "8" kifntTitle
	BUTTON=kidc9 (44 84 34 14) "9" kifntTitle
	BUTTON=kidcA (83 84 34 14) "A" kifntTitle
	BUTTON=kidcB (121 84 34 14) "B" kifntTitle
	BUTTON=kidcC (6 101 34 14) "C" kifntTitle
	BUTTON=kidcD (44 101 34 14) "D" kifntTitle
	BUTTON=kidcE (83 101 34 14) "E" kifntTitle
	BUTTON=kidcF (121 101 34 14) "F" kifntTitle
	BUTTON=kidcBackspace (121 118 34 14) "<--" kifntTitle
	BUTTON=kidcOk (32 139 40 12) "OK" kifntButton
	BUTTON=kidcCancel (88 139 40 12) "CANCEL" kifntButton

[kidfRegisterNow]
	FORM=(0 0 160 160) kidcOk center
	LABEL=0 (56 6 48 8) "Purchase Now!" kifntTitle center
	LABEL=0 (12 22 136 8) "Find out what happens to Andy, Jana and" kifntShadow
	LABEL=0 (12 32 121 8) "crew! Purchase now and get:" kifntShadow
	LABEL=0 (31 50 97 8) "Rich Story Driven Experience" kifntShadow center
	LABEL=0 (44 60 71 8) "20 Single Player Missions" kifntShadow center
	LABEL=0 (45 70 69 8) "21 Multi Player Missions" kifntShadow center
	LABEL=0 (46 80 67 8) "2 Play Environments" kifntShadow center
	LABEL=0 (53 90 54 8) "11 Building Types" kifntShadow center
	LABEL=0 (60 100 40 8) "11 Unit Types" kifntShadow center
	LABEL=0 (42 110 76 8) "Digitized Sound Effects" kifntShadow center
	LABEL=0 (50 120 60 8) "Mission Statistics" kifntShadow center
	LABEL=0 (63 130 33 8) "Mission Editor, and more!" kifntShadow center
	LABEL=0 (8 147 143 8) "Purchase at www.WarfareIncorporated.com" kifntShadow center

[kidfInputPanel]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=0 (42 6 77 0) "INPUT PANEL" kifntTitle center
	LABEL=kidcInputLabel (2 18 52 0) "???" kifntShadow
	EDIT=kidcInputEdit (48 18 110 0) "???" kifntShadow
	BUTTON=kidcBackspace (121 118 34 14) "<--" kifntTitle
	BUTTON=kidcOk (32 139 40 12) "OK" kifntButton
	BUTTON=kidcCancel (88 139 40 12) "CANCEL" kifntButton

[kidfLoading]
	FORM=(0 0 80 25) kidcCancel center
	LABEL=kidcTitle (0 9 80 0) "LOADING..." kifntTitle center

[kidfWaiting]
	FORM=(0 0 150 25) kidcCancel center topmost
	FORMBACKCOLOR=kiclrBlack
	LABEL=kidcTitle (0 9 150 0) "" kifntTitle center

[kidfMultiplayerWinSummary]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "PERFORMANCE REVIEW" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=kidcMissionResult (80 63 0 0) "YOU WIN!" kifntTitle center
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	BUTTON=kidcCancel (80 140 40 14) "OK" kifntButton center

[kidfMultiplayerLoseSummary]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "PERFORMANCE REVIEW" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=0 (80 63 0 0) "YOU LOSE!" kifntTitle center
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	BUTTON=kidcCancel (80 140 40 14) "OK" kifntButton center

[kidfMultiplayerObjectives]
	FORM=(0 0 160 160) kidcCancel center
	FORMBACKCOLOR=kiclrFormBackground
	LABEL=kidcMissionTitle (62 6 35 0) "" kifntTitle center
	LABEL=0 (62 14 35 0) "OBJECTIVES" kifntTitle center
	LABEL=kidcObjectiveText1 (4 25 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus1 (155 25 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText2 (4 34 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus2 (155 34 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText3 (4 43 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus3 (155 43 0 0) "" kifntShadow right
	LABEL=kidcObjectiveText4 (4 52 0 0) "" kifntShadow
	LABEL=kidcObjectiveStatus4 (155 52 0 0) "" kifntShadow right
	LABEL=kidcPage1 (80 63 0 0) "BRIEFING" kifntTitle center
	LABEL=kidcPage2 (80 63 0 0) "STATISTICS" kifntTitle center
	LABEL=kidcObjectiveInfo (4 74 152 69) "" kifntShadow left multiline
	LABEL=kidcPage2 (4 74 0 0) "Enemy units destroyed" kifntShadow
	LABEL=kidcMobileUnitsKilled (155 74 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 83 0 0) "Enemy buildings destroyed" kifntShadow
	LABEL=kidcStructuresKilled (155 83 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 92 0 0) "Friendly units lost" kifntShadow
	LABEL=kidcMobileUnitsLost (155 92 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 101 0 0) "Friendly buildings lost" kifntShadow
	LABEL=kidcStructuresLost (155 101 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 110 0 0) "Credits acquired / spent" kifntShadow
	LABEL=kidcCreditsAction (155 110 0 0) "" kifntShadow right
	LABEL=kidcPage2 (4 119 0 0) "Elapsed time" kifntShadow
	LABEL=kidcGameTime (155 119 0 0) "" kifntShadow right
	BUTTON=kidcStatistics (50 140 60 14) "STATISTICS" kifntButton center
	BUTTON=kidcInfo (50 140 60 14) "BRIEFING" kifntButton center
	BUTTON=kidcCancel (120 140 40 14) "OK" kifntButton center

