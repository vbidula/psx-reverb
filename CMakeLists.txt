cmake_minimum_required(VERSION 3.24)

project(psx_reverb VERSION 0.1.0 LANGUAGES CXX)

option(PSX_REVERB_BUILD_VST3 "Build the VST3 plug-in" OFF)

if(APPLE AND PSX_REVERB_BUILD_VST3)
    enable_language(OBJCXX)
endif()

add_library(psx_reverb_dsp STATIC src/dsp/psx_reverb.cpp)
add_library(psx_reverb::dsp ALIAS psx_reverb_dsp)

target_include_directories(psx_reverb_dsp PUBLIC src)
target_compile_features(psx_reverb_dsp PUBLIC cxx_std_17)
set_target_properties(psx_reverb_dsp PROPERTIES POSITION_INDEPENDENT_CODE ON)

if(MSVC)
    target_compile_options(psx_reverb_dsp PRIVATE /W4)
else()
    target_compile_options(psx_reverb_dsp PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(PSX_REVERB_BUILD_VST3)
    include(FetchContent)

    if(APPLE AND XCODE_VERSION AND NOT DEFINED ENV{XCODE_VERSION})
        set(ENV{XCODE_VERSION} "${XCODE_VERSION}")
    endif()

    set(SMTG_CREATE_PLUGIN_LINK OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VST3_PLUGIN_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VST3_HOSTING_EXAMPLES OFF CACHE BOOL "" FORCE)
    set(SMTG_ENABLE_VSTGUI_SUPPORT ON CACHE BOOL "" FORCE)

    FetchContent_Declare(
        vst3sdk
        GIT_REPOSITORY https://github.com/steinbergmedia/vst3sdk.git
        GIT_TAG v3.8.0_build_66
        GIT_SHALLOW TRUE
    )
    FetchContent_MakeAvailable(vst3sdk)

    if(APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        target_compile_options(vstgui PRIVATE
            -Wno-error=deprecated-declarations)
        target_compile_options(vstgui_uidescription PRIVATE
            -Wno-error=deprecated-declarations)
    endif()

    smtg_enable_vst3_sdk()

    add_subdirectory(src/vst3)
endif()
