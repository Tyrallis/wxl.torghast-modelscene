cmake_minimum_required(VERSION 3.21)
project(wxl-torghast-modelscene VERSION 0.2.0 LANGUAGES NONE)

set(WXL_SDK_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../sdk" CACHE PATH "Path to the WarcraftXL SDK")
include("${WXL_SDK_DIR}/cmake/wxl_add_mod.cmake")

wxl_add_mod(torghast-modelscene
    SOURCES src/TorghastModelScene.cpp
    MANIFEST mod.json
    TARGETS win-x64)
