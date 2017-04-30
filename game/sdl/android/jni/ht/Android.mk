LOCAL_PATH := $(call my-dir)
ABSOLUTE_PATH := $(shell pwd)/$(LOCAL_PATH)

include $(CLEAR_VARS)

LOCAL_MODULE := main

# hostile-takeover
HT_PATH := $(ABSOLUTE_PATH)/../../../../..
GAME_PATH := $(HT_PATH)/game
MPS_PATH := $(HT_PATH)/mpshared
BASE_PATH := $(HT_PATH)/base
YAJL_PATH := $(HT_PATH)/yajl
HOST_PATH := $(GAME_PATH)/sdl
INC_PATH := $(HT_PATH)/inc
HTA_PATH := $(HOST_PATH)/android/jni/ht/
SDL2_PATH := $(HOST_PATH)/libs/SDL2
CURL_PATH := $(HOST_PATH)/libs/curl
SDL2IMG_PATH = $(HOST_PATH)/libs/SDL2_image

LOCAL_C_INCLUDES := $(SDL2_PATH)/include $(HT_PATH) $(INC_PATH) $(GAME_PATH) \
$(HOST_PATH) $(CURL_PATH)/include $(SDL2IMG_PATH)/include

SRC_GAME := $(wildcard $(GAME_PATH)/*.cpp) \
            $(wildcard $(MPS_PATH)/*.cpp) \
            $(wildcard $(BASE_PATH)/*.cpp) \
            $(wildcard $(YAJL_PATH)/src/*.c) \
            $(wildcard $(YAJL_PATH)/wrapper/*.cpp) \
            $(wildcard $(INC_PATH)/*.cpp) \
            $(wildcard $(HOST_PATH)/*.cpp) \
            $(wildcard $(HTA_PATH)/*.cpp) \
            $(wildcard $(HTA_PATH)/sdl_android/*.c) \
            $(wildcard $(HTA_PATH)/sdl_android/dynapi/*.c)

# game excludes
SRC_GAME := $(filter-out %SocTransport.cpp %miscgraphics.cpp %hashtablecode.cpp %filepdbreader.cpp %DebugHelpers.cpp, $(SRC_GAME))

# base excludes
SRC_GAME := $(filter-out %epollserver.cpp %stdindispatcher.cpp, $(SRC_GAME))

# yajl excludes
SRC_GAME := $(filter-out %jsonbuilder_test.cpp, $(SRC_GAME))

LOCAL_SRC_FILES := $(SRC_GAME)

LOCAL_SHARED_LIBRARIES := SDL2 SDL2_image
LOCAL_STATIC_LIBRARIES := curl

LOCAL_CFLAGS := -Wno-write-strings -fsigned-char -w
LOCAL_CFLAGS += -DSDL -DMULTIPLAYER -DTRACKSTATE
LOCAL_CPPFLAGS += -std=c++0x

ifndef REL
LOCAL_CFLAGS += -DDEV_BUILD -DDEBUG_LOGGING -DDEBUG -DLOGGING
endif

LOCAL_LDLIBS := -llog -landroid

include $(BUILD_SHARED_LIBRARY)
