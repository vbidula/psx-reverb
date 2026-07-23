cmake_minimum_required(VERSION 3.24)

project(psx_reverb VERSION 0.1.0 LANGUAGES CXX)

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
