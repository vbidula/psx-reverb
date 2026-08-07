smtg_add_vst3plugin(PSXReverb
    Info.plist.in
    controller.cpp
    controller.hpp
    factory.cpp
    plugin_ids.hpp
    processor.cpp
    processor.hpp
    resources/editor.uidesc
    version.hpp
)

target_link_libraries(PSXReverb
    PRIVATE
        psx_reverb::dsp
        sdk
        vstgui_support
)

target_include_directories(PSXReverb PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")
target_compile_features(PSXReverb PRIVATE cxx_std_17)
smtg_target_configure_version_file(PSXReverb)

smtg_target_add_plugin_resources(PSXReverb
    RESOURCES
        resources/editor.uidesc
)

if(SMTG_MAC)
    set_target_properties(PSXReverb PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist.in"
    )
    set_property(TARGET PSXReverb APPEND PROPERTY LINK_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/resources/editor.uidesc"
    )

    smtg_target_set_bundle(PSXReverb
        BUNDLE_IDENTIFIER com.vbidula.psx-reverb
        COMPANY_NAME vbidula
    )

    add_custom_command(TARGET PSXReverb POST_BUILD
        COMMAND /usr/bin/codesign --force --sign - "$<TARGET_BUNDLE_DIR:PSXReverb>"
        COMMENT "Signing the completed VST3 bundle"
    )
endif()
