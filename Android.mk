LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := GunExecution
LOCAL_SRC_FILES := main.cpp
LOCAL_CFLAGS    := -O2 -mfloat-abi=softfp -DNDEBUG -std=c++17
LOCAL_C_INCLUDES := $(LOCAL_PATH)   # dito ilagay ang mod/ folder galing sa AML SDK
LOCAL_LDLIBS    := -llog
include $(BUILD_SHARED_LIBRARY)

# Application.mk:
#   APP_ABI := armeabi-v7a
#   APP_PLATFORM := android-21
#   APP_STL := c++_static
