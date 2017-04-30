LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := SDL2_image
LOCAL_SRC_FILES := ../../../libs/SDL2_image/android/$(TARGET_ARCH_ABI)/libSDL2_image.so
LOCAL_EXPORT_C_INCLUDES := ../../../libs/SDL2_image/include
include $(PREBUILT_SHARED_LIBRARY)
