LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := curl
LOCAL_SRC_FILES := android/$(TARGET_ARCH_ABI)/libcurl.a
LOCAL_EXPORT_LDLIBS := -lz
include $(PREBUILT_STATIC_LIBRARY)
