# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.
cmake_minimum_required(VERSION 3.4.1)

set(wi_dir ${CMAKE_SOURCE_DIR}/../../../..)
set(game_dir ${wi_dir}/game)
set(base_dir ${wi_dir}/base)
set(yajl_dir ${wi_dir}/yajl)
set(inc_dir ${wi_dir}/inc)
set(mpshared_dir ${wi_dir}/mpshared)

set(game_src
    "${game_dir}/Andy.cpp"
    "${game_dir}/Animation.cpp"
    "${game_dir}/Artillery.cpp"
    "${game_dir}/BuildMgr.cpp"
    "${game_dir}/Builder.cpp"
    "${game_dir}/CutScene.cpp"
    "${game_dir}/Ecom.cpp"
    "${game_dir}/GameObjects.cpp"
    "${game_dir}/GameOptions.cpp"
    "${game_dir}/HRC.cpp"
    "${game_dir}/Headquarters.cpp"
    "${game_dir}/Help.cpp"
    "${game_dir}/InputUI.cpp"
    "${game_dir}/LRInfantry.cpp"
    "${game_dir}/Level.cpp"
    "${game_dir}/Miner.cpp"
    "${game_dir}/MobileBuilder.cpp"
    "${game_dir}/MobileHQ.cpp"
    "${game_dir}/MobileUnit.cpp"
    "${game_dir}/Multiplayer.cpp"
    "${game_dir}/Multiplayer.h"
    "${game_dir}/Overmind.cpp"
    "${game_dir}/Player.cpp"
    "${game_dir}/Processor.cpp"
    "${game_dir}/Radar.cpp"
    "${game_dir}/Reactor.cpp"
    "${game_dir}/Replicator.cpp"
    "${game_dir}/Research.cpp"
    "${game_dir}/SRInfantry.cpp"
    "${game_dir}/Shell.cpp"
    "${game_dir}/SimUI.cpp"
    "${game_dir}/Simulation.cpp"
    "${game_dir}/SpInfantry.cpp"
    "${game_dir}/StateMachine.cpp"
    "${game_dir}/Struct.cpp"
    "${game_dir}/Tank.cpp"
    "${game_dir}/Tower.cpp"
    "${game_dir}/TriggerActions.cpp"
    "${game_dir}/TriggerConditions.cpp"
    "${game_dir}/Unit.cpp"
    "${game_dir}/UnitGroupMgr.cpp"
    "${game_dir}/VTS.cpp"
    "${game_dir}/Warehouse.cpp"
    "${game_dir}/alertcontrol.cpp"
    "${game_dir}/bitmap.cpp"
    "${game_dir}/cachemgr.cpp"
    "${game_dir}/chatcontroller.h"
    "${game_dir}/chatter.cpp"
    "${game_dir}/chatter.h"
    "${game_dir}/chooseserverform.cpp"
    "${game_dir}/chooseserverform.h"
    "${game_dir}/comm.cpp"
    "${game_dir}/completemanager.cpp"
    "${game_dir}/completemanager.h"
    "${game_dir}/compression.cpp"
    "${game_dir}/creategameform.cpp"
    "${game_dir}/creategameform.h"
    "${game_dir}/createroomform.cpp"
    "${game_dir}/createroomform.h"
    "${game_dir}/dlmissionpack.cpp"
    "${game_dir}/downloadbox.cpp"
    "${game_dir}/dragrect.cpp"
    "${game_dir}/dragrect.h"
    "${game_dir}/drawscan.cpp"
    "${game_dir}/drm.cpp"
    "${game_dir}/event.cpp"
    "${game_dir}/fingerhandler.cpp"
    "${game_dir}/flickscroller.cpp"
    "${game_dir}/fogmap.cpp"
    "${game_dir}/font.cpp"
    "${game_dir}/form.cpp"
    "${game_dir}/formmgr.cpp"
    "${game_dir}/game.cpp"
    "${game_dir}/gameform.cpp"
    "${game_dir}/gameform.h"
    "${game_dir}/ht.h"
    "${game_dir}/httpindexloader.cpp"
    "${game_dir}/httpindexloader.h"
    "${game_dir}/httppackinfomanager.cpp"
    "${game_dir}/httppackinfomanager.h"
    "${game_dir}/httppackmanager.cpp"
    "${game_dir}/httppackmanager.h"
    "${game_dir}/httprequest.cpp"
    "${game_dir}/httprequest.h"
    "${game_dir}/httpservice.h"
    "${game_dir}/iphone"
    "${game_dir}/license.h"
    "${game_dir}/loadsave.cpp"
    "${game_dir}/lobby.cpp"
    "${game_dir}/lobby.h"
    "${game_dir}/lobbyform.cpp"
    "${game_dir}/lobbyform.h"
    "${game_dir}/loginform.cpp"
    "${game_dir}/loginform.h"
    "${game_dir}/loginhandler.cpp"
    "${game_dir}/loginhandler.h"
    "${game_dir}/map.cpp"
    "${game_dir}/map.h"
    "${game_dir}/memmgr.cpp"
    "${game_dir}/mempdbreader.cpp"
    "${game_dir}/mempdbreader.h"
    "${game_dir}/misc.cpp"
    "${game_dir}/misccontrols.cpp"
    "${game_dir}/missionlist.cpp"
    "${game_dir}/missionlist.h"
    "${game_dir}/mixer.cpp"
    "${game_dir}/mixer.h"
    "${game_dir}/ops.h"
    "${game_dir}/picktransportform.cpp"
    "${game_dir}/picktransportform.h"
    "${game_dir}/preferences.cpp"
    "${game_dir}/progresscallback.h"
    "${game_dir}/refmap.h"
    "${game_dir}/res.h"
    "${game_dir}/roomform.cpp"
    "${game_dir}/roomform.h"
    "${game_dir}/selectmission.cpp"
    "${game_dir}/serviceurls.cpp"
    "${game_dir}/serviceurls.h"
    "${game_dir}/simplerequest.cpp"
    "${game_dir}/simplerequest.h"
    "${game_dir}/sizeops.h"
    "${game_dir}/sounddevice.h"
    "${game_dir}/soundeffects.h"
    "${game_dir}/soundmgr.cpp"
    "${game_dir}/sprite.h"
    "${game_dir}/stateframe.cpp"
    "${game_dir}/stateframe.h"
    "${game_dir}/statetracker.cpp"
    "${game_dir}/statetracker.h"
    "${game_dir}/stringtable.cpp"
    "${game_dir}/stylushandler.cpp"
    "${game_dir}/tbitmap.cpp"
    "${game_dir}/tbitmapsr.cpp"
    "${game_dir}/terrainmap.cpp"
    "${game_dir}/tests.cpp"
    "${game_dir}/texatlasmgr.cpp"
    "${game_dir}/thunks.cpp"
    "${game_dir}/tilemap.cpp"
    "${game_dir}/timer.cpp"
    "${game_dir}/timerwrap.h"
    "${game_dir}/triggermgr.cpp"
    "${game_dir}/updatemap.cpp"
    "${game_dir}/uploader.cpp"
    "${game_dir}/uploader.h"
    "${game_dir}/vec2d.h"
    "${game_dir}/wistrings.h"
    "${game_dir}/xtransport.cpp"
    "${game_dir}/xtransport.h"

    "${game_dir}/sdl/display.cpp"
    "${game_dir}/sdl/host.cpp"
    "${game_dir}/sdl/hosthelpers.h"
    "${game_dir}/sdl/htplatform.h"
    "${game_dir}/sdl/main.cpp"
    "${game_dir}/sdl/savegame.cpp"
    "${game_dir}/sdl/sdlanimsprite.cpp"
    "${game_dir}/sdl/sdlanimsprite.h"
    "${game_dir}/sdl/sdleventserver.h"
    "${game_dir}/sdl/sdlhttprequest.cpp"
    "${game_dir}/sdl/sdlhttprequest.h"
    "${game_dir}/sdl/sdlhttpservice.cpp"
    "${game_dir}/sdl/sdlhttpservice.h"
    "${game_dir}/sdl/sdlpackfile.cpp"
    "${game_dir}/sdl/sdlpackfile.h"
    "${game_dir}/sdl/sdlselectionsprite.cpp"
    "${game_dir}/sdl/sdlselectionsprite.h"
    "${game_dir}/sdl/sdlsounddev.cpp"
    "${game_dir}/sdl/sdlspritemgr.cpp"
    "${game_dir}/sdl/sdlspritemgr.h"
    "${game_dir}/sdl/sysmessages.h"
    "${game_dir}/sdl/transportmgr.cpp"

    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/SDL_android_main.c"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/SDL_assert_c.h"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/SDL_error_c.h"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/SDL_internal.h"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/dynapi/SDL_dynapi_overrides.h"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/dynapi/SDL_dynapi_procs.h"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/dynapi/SDL_dynapi.c"
    "${game_dir}/sdl/android/app/src/main/cpp/sdl_android/dynapi/SDL_dynapi.h"

    "${game_dir}/sdl/android/app/src/main/cpp/androidchatcontroller.cpp"
    "${game_dir}/sdl/android/app/src/main/cpp/androidchatcontroller.h"
    "${game_dir}/sdl/android/app/src/main/cpp/hosthelpers.cpp"
)

set(base_src
    "${base_dir}/base64.cpp"
    "${base_dir}/base64.h"
    "${base_dir}/bytebuffer.cpp"
    "${base_dir}/bytebuffer.h"
    "${base_dir}/criticalsection.h"
    "${base_dir}/deletetracker.cpp"
    "${base_dir}/deletetracker.h"
    "${base_dir}/dispatcher.h"
    "${base_dir}/eventer.cpp"
    "${base_dir}/eventer.h"
    "${base_dir}/format.cpp"
    "${base_dir}/format.h"
    "${base_dir}/log.h"
    "${base_dir}/md5.h"
    "${base_dir}/md5c.cpp"
    "${base_dir}/messagehandler.cpp"
    "${base_dir}/messagehandler.h"
    "${base_dir}/messagequeue.cpp"
    "${base_dir}/messagequeue.h"
    "${base_dir}/misc.cpp"
    "${base_dir}/misc.h"
    "${base_dir}/selectserver.cpp"
    "${base_dir}/selectserver.h"
    "${base_dir}/sigslot.h"
    "${base_dir}/socket.cpp"
    "${base_dir}/socket.h"
    "${base_dir}/socketaddress.cpp"
    "${base_dir}/socketaddress.h"
    "${base_dir}/socketserver.cpp"
    "${base_dir}/socketserver.h"
    "${base_dir}/thread.cpp"
    "${base_dir}/thread.h"
    "${base_dir}/tick.cpp"
    "${base_dir}/tick.h"
)

set(yajl_src
    "${yajl_dir}/api/yajl_common.h"
    "${yajl_dir}/api/yajl_gen.h"
    "${yajl_dir}/api/yajl_parse.h"

    "${yajl_dir}/src/yajl.c"
    "${yajl_dir}/src/yajl_buf.c"
    "${yajl_dir}/src/yajl_buf.h"
    "${yajl_dir}/src/yajl_encode.c"
    "${yajl_dir}/src/yajl_encode.h"
    "${yajl_dir}/src/yajl_gen.c"
    "${yajl_dir}/src/yajl_lex.c"
    "${yajl_dir}/src/yajl_lex.h"
    "${yajl_dir}/src/yajl_parser.c"
    "${yajl_dir}/src/yajl_parser.h"

    "${yajl_dir}/wrapper/jsonbuilder.cpp"
    "${yajl_dir}/wrapper/jsonbuilder.h"
    "${yajl_dir}/wrapper/jsontypes.cpp"
    "${yajl_dir}/wrapper/jsontypes.h"
)

set(mpshared_src
    "${mpshared_dir}/constants.h"
    "${mpshared_dir}/decompress.cpp"
    "${mpshared_dir}/decompress.h"
    "${mpshared_dir}/enum.h"
    "${mpshared_dir}/indexloader.cpp"
    "${mpshared_dir}/indexloader.h"
    "${mpshared_dir}/ini.cpp"
    "${mpshared_dir}/ini.h"
    "${mpshared_dir}/messages.cpp"
    "${mpshared_dir}/messages.h"
    "${mpshared_dir}/misc.cpp"
    "${mpshared_dir}/misc.h"
    "${mpshared_dir}/mpht.h"
    "${mpshared_dir}/netmessage.cpp"
    "${mpshared_dir}/netmessage.h"
    "${mpshared_dir}/packfile.cpp"
    "${mpshared_dir}/packfile.h"
    "${mpshared_dir}/packinfomanager.cpp"
    "${mpshared_dir}/packinfomanager.h"
    "${mpshared_dir}/packmanager.cpp"
    "${mpshared_dir}/packmanager.h"
    "${mpshared_dir}/pdbreader.h"
    "${mpshared_dir}/side.h"
    "${mpshared_dir}/xmsg.h"
    "${mpshared_dir}/xmsglog.cpp"
    "${mpshared_dir}/xmsglog.h"
    "${mpshared_dir}/xpump.cpp"
    "${mpshared_dir}/xpump.h"
)

set(inc_src
    "${inc_dir}/rip.cpp"
    "${inc_dir}/rip.h"
)

set(sources
    ${game_src}
    ${base_src}
    ${yajl_src}
    ${inc_src}
    ${mpshared_src}
)

set(includes
    ${wi_dir}
    ${game_dir}
    ${game_dir}/sdl
    ${inc_dir}
    ${base_dir}
    ${yajl_dir}
    ${mpshared_dir}

    ${game_dir}/sdl/libs/SDL2/include
    ${game_dir}/sdl/libs/SDL2_image/include
    ${game_dir}/sdl/libs/curl/include
)

include_directories(${includes})

add_library(main SHARED ${sources})

add_library(libSDL2 SHARED IMPORTED)
set_target_properties(libSDL2 PROPERTIES IMPORTED_LOCATION ${game_dir}/sdl/libs/SDL2/android/${ANDROID_ABI}/libSDL2.so)

add_library(libSDL2_image SHARED IMPORTED)
set_target_properties(libSDL2_image PROPERTIES IMPORTED_LOCATION ${game_dir}/sdl/libs/SDL2_image/android/${ANDROID_ABI}/libSDL2_image.so)

add_library(libCurl STATIC IMPORTED)
set_target_properties(libCurl PROPERTIES IMPORTED_LOCATION
    ${game_dir}/sdl/libs/curl/android/${ANDROID_ABI}/libcurl.a)

find_library(libLog log)
find_library(libAndroid android)
find_library(libz z)

target_link_libraries(main
    libSDL2
    libSDL2_image
    libCurl
    ${libLog}
    ${libAndroid}
    ${libz}
)
