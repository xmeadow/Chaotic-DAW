cmake_minimum_required(VERSION 3.15)
project(Chaotic-DAW CXX C)

# ── Platform detection ─────────────────────────────────────────────────────────
if(WIN32)
    set(USE_WIN32 1)
    set(USE_LINUX 0)
    set(USE_MAC 0)
    message(STATUS "Building for Windows")
elseif(APPLE)
    set(USE_WIN32 0)
    set(USE_LINUX 0)
    set(USE_MAC 1)
    message(STATUS "Building for macOS")
elseif(UNIX)
    set(USE_WIN32 0)
    set(USE_LINUX 1)
    set(USE_MAC 0)
    message(STATUS "Building for Linux")
else()
    message(FATAL_ERROR "Unsupported platform")
endif()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Source root ───────────────────────────────────────────────────────────────
set(ROOT ${CMAKE_CURRENT_SOURCE_DIR})

# ── Collect rosic sources (large DSP library) ─────────────────────────────────
file(GLOB_RECURSE ROSIC_SOURCES
    "${ROOT}/rosic/*.cpp"
)

# ── Data sources (compiled binary assets) ─────────────────────────────────────
set(DATA_SOURCES
    "${ROOT}/Data sources/aripix.cpp"
    "${ROOT}/Data sources/big.cpp"
    "${ROOT}/Data sources/Bold.cpp"
    "${ROOT}/Data sources/dpix.cpp"
    "${ROOT}/Data sources/fixed.cpp"
    "${ROOT}/Data sources/images.cpp"
    "${ROOT}/Data sources/instrum.cpp"
    "${ROOT}/Data sources/metronome_waves.cpp"
    "${ROOT}/Data sources/mfont.cpp"
    "${ROOT}/Data sources/projs.cpp"
    "${ROOT}/Data sources/roxx.cpp"
    "${ROOT}/Data sources/tahoma.cpp"
    "${ROOT}/Data sources/tiny1.cpp"
    "${ROOT}/Data sources/tiny.cpp"
    "${ROOT}/Data sources/visitor.cpp"
)

# ── Main application sources (common to all platforms) ────────────────────────
set(MAIN_SOURCES
    "${ROOT}/Awful.cpp"
    "${ROOT}/Awful_audio.cpp"
    "${ROOT}/awful_audiohost.cpp"
    "${ROOT}/Awful_controls.cpp"
    "${ROOT}/Awful_CursorAndMouse.cpp"
    "${ROOT}/Awful_effects.cpp"
    "${ROOT}/Awful_elements.cpp"
    "${ROOT}/Awful_events_triggers.cpp"
    "${ROOT}/Awful_graphJuce.cpp"
    "${ROOT}/Awful_instruments.cpp"
    "${ROOT}/Awful_JuceComponents.cpp"
    "${ROOT}/Awful_JuceWindows.cpp"
    "${ROOT}/Awful_logger.cpp"
    "${ROOT}/awful_midi.cpp"
    "${ROOT}/Awful_objects.cpp"
    "${ROOT}/Awful_panels.cpp"
    "${ROOT}/Awful_paramedit.cpp"
    "${ROOT}/Awful_parammodule.cpp"
    "${ROOT}/Awful_params.cpp"
    "${ROOT}/Awful_preview.cpp"
    "${ROOT}/awful_renderer.cpp"
    "${ROOT}/Awful_tracks_lanes.cpp"
    "${ROOT}/Awful_UndoRedo.cpp"
    "${ROOT}/Awful_utils_common.cpp"
    "${ROOT}/CVSTHost.cpp"
    "${ROOT}/VSTCollection.cpp"
    "${ROOT}/StdAfx.cpp"
    "${ROOT}/juce/juce_amalgamated.cpp"
)

# ── Platform‑specific sources ─────────────────────────────────────────────────
if(USE_WIN32)
    # Windows‑specific PortAudio core and backends
    set(PORTAUDIO_CORE
        "${ROOT}/External/common/pa_allocation.c"
        "${ROOT}/External/common/pa_converters.c"
        "${ROOT}/External/common/pa_cpuload.c"
        "${ROOT}/External/common/pa_dither.c"
        "${ROOT}/External/common/pa_front.c"
        "${ROOT}/External/common/pa_process.c"
        "${ROOT}/External/common/pa_stream.c"
        "${ROOT}/External/common/pa_trace.c"
        "${ROOT}/External/common/pa_skeleton.c"
        "${ROOT}/External/win/pa_win_hostapis.c"
        "${ROOT}/External/win/pa_win_util.c"
    )

    # ASIO SDK contains 32-bit x86 inline assembly — exclude on 64-bit
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(PORTAUDIO_BACKENDS
            "${ROOT}/External/asio/pa_asio.cpp"
            "${ROOT}/External/asio/iasiothiscallresolver.cpp"
            "${ROOT}/External/wasapi/pa_win_wasapi.cpp"
            "${ROOT}/External/wmme/pa_win_wmme.c"
        )
        set(ASIO_SOURCES
            "${ROOT}/ASIOSDK2/common/asio.cpp"
            "${ROOT}/ASIOSDK2/common/combase.cpp"
            "${ROOT}/ASIOSDK2/host/asiodrivers.cpp"
            "${ROOT}/ASIOSDK2/host/pc/asiolist.cpp"
        )
    else()
        set(PORTAUDIO_BACKENDS
            "${ROOT}/External/wasapi/pa_win_wasapi.cpp"
            "${ROOT}/External/wmme/pa_win_wmme.c"
        )
        set(ASIO_SOURCES "")
    endif()

    set(RESOURCE_FILE "${ROOT}/packaging/windows/Awful.rc")
else()
    # Non‑Windows: no PortAudio/ASIO sources
    set(PORTAUDIO_CORE "")
    set(PORTAUDIO_BACKENDS "")
    set(ASIO_SOURCES "")
    set(RESOURCE_FILE "")
endif()

# ── Executable ───────────────────────────────────────────────────────────────
if(USE_WIN32)
    add_executable(${PROJECT_NAME} WIN32
        ${MAIN_SOURCES}
        ${ROSIC_SOURCES}
        ${DATA_SOURCES}
        ${PORTAUDIO_CORE}
        ${PORTAUDIO_BACKENDS}
        ${ASIO_SOURCES}
        ${RESOURCE_FILE}
    )
else()
    add_executable(${PROJECT_NAME}
        ${MAIN_SOURCES}
        ${ROSIC_SOURCES}
        ${DATA_SOURCES}
    )
endif()

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    ${ROOT}
    ${ROOT}/vst2compat
    ${ROOT}/juce
    ${ROOT}/vstsdk3
    ${ROOT}/vstsdk3/pluginterfaces
)

if(USE_WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${ROOT}/packaging/windows
        ${ROOT}/External/include
        ${ROOT}/External/common
        ${ROOT}/External/win
    )
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        target_include_directories(${PROJECT_NAME} PRIVATE
            ${ROOT}/ASIOSDK2/common
            ${ROOT}/ASIOSDK2/host
            ${ROOT}/ASIOSDK2/host/pc
            ${ROOT}/External/asio
        )
    endif()
endif()

# ── Compile definitions ───────────────────────────────────────────────────────
set(COMMON_DEFINITIONS
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS
    JUCE_STRINGS_ARE_UNICODE=0
    JUCE_WEB_BROWSER=0
    JUCE_OPENGL=0
)

if(USE_WIN32)
    list(APPEND COMMON_DEFINITIONS
        WIN32
        _WINDOWS
        PA_USE_WASAPI=1
        PA_USE_WMME=1
        PA_USE_DS=0
        PA_NO_DS=1
        USE_WIN32=1
    )
    # ASIO only available on 32-bit (SDK uses x86 inline asm)
    if(CMAKE_SIZEOF_VOID_P EQUAL 4)
        list(APPEND COMMON_DEFINITIONS PA_USE_ASIO=1)
    else()
        list(APPEND COMMON_DEFINITIONS PA_USE_ASIO=0 PA_NO_ASIO=1 JUCE_ASIO=0)
    endif()
elseif(USE_LINUX)
    list(APPEND COMMON_DEFINITIONS
        LINUX
        USE_LINUX=1
        USE_OLD_JUCE
        USE_JUCE_AUDIO
        JUCE_ALSA=1
        JUCE_USE_ALSA=1
    )
    if(JACK_FOUND)
        list(APPEND COMMON_DEFINITIONS JUCE_JACK=1 JUCE_USE_JACK=1)
    endif()
elseif(USE_MAC)
    list(APPEND COMMON_DEFINITIONS
        USE_MAC=1
        USE_JUCE_AUDIO
    )
endif()

target_compile_definitions(${PROJECT_NAME} PRIVATE ${COMMON_DEFINITIONS})

# ── Compiler flags ────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W1
        /MP
        /bigobj
    )
elseif(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -w
        -Wno-deprecated
        -fpermissive
        -fno-check-new
    )
    if(MINGW)
        target_compile_options(${PROJECT_NAME} PRIVATE -Wa,-mbig-obj)
    endif()
endif()

# ── Platform‑specific libraries ───────────────────────────────────────────────
if(USE_WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm
        ole32
        oleaut32
        uuid
        comctl32
        comdlg32
        user32
        gdi32
        kernel32
        advapi32
        shlwapi
        ws2_32
        dsound
        dxguid
        version
        wininet
        vfw32
    )

    # Bundled libsndfile for Windows
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        target_link_libraries(${PROJECT_NAME} PRIVATE "${ROOT}/packaging/windows/libsndfile-1-x64.lib")
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE "${ROOT}/packaging/windows/libsndfile-1.lib")
    endif()
endif()

if(USE_LINUX)
    # Find Linux dependencies with pkg‑config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(ALSA REQUIRED alsa)
    pkg_check_modules(JACK jack)
    pkg_check_modules(X11 REQUIRED x11 xext xrandr xinerama xcursor xi)
    pkg_check_modules(SNDFILE REQUIRED sndfile)
    pkg_check_modules(FREETYPE REQUIRED freetype2)
    pkg_check_modules(CURL REQUIRED libcurl)

    set(LINUX_INCLUDES
        ${ALSA_INCLUDE_DIRS}
        ${X11_INCLUDE_DIRS}
        ${SNDFILE_INCLUDE_DIRS}
        ${FREETYPE_INCLUDE_DIRS}
        ${CURL_INCLUDE_DIRS}
    )
    if(JACK_FOUND)
        list(APPEND LINUX_INCLUDES ${JACK_INCLUDE_DIRS})
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE
        ${LINUX_INCLUDES}
    )

    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${ALSA_LIBRARIES}
        ${X11_LIBRARIES}
        ${SNDFILE_LIBRARIES}
        ${FREETYPE_LIBRARIES}
        ${CURL_LIBRARIES}
        pthread
        dl
        rt
    )
    if(JACK_FOUND)
        target_link_libraries(${PROJECT_NAME} PRIVATE ${JACK_LIBRARIES})
    endif()

    # Optional: link system libsndfile if bundled version not desired
    # target_link_libraries(${PROJECT_NAME} PRIVATE sndfile)
endif()

if(USE_MAC)
    # macOS frameworks (to be filled in when macOS port is undertaken)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework Cocoa"
        "-framework CoreAudio"
        "-framework AudioUnit"
        "-framework CoreFoundation"
        "-framework CoreServices"
        "-framework IOKit"
        "-framework Carbon"
        "-framework Accelerate"
    )
endif()

# ── Copy runtime assets next to binary ────────────────────────────────────────
# Samples, Presets, and Projects must be alongside the executable
set(ASSET_DIRS Samples Presets Projects Plugins)
foreach(ASSET_DIR ${ASSET_DIRS})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ROOT}/${ASSET_DIR}"
            "$<TARGET_FILE_DIR:${PROJECT_NAME}>/${ASSET_DIR}"
        COMMENT "Copying ${ASSET_DIR}/ to build output"
    )
endforeach()

# ── Installation (Linux‑specific) ─────────────────────────────────────────────
if(USE_LINUX)
    install(TARGETS ${PROJECT_NAME} DESTINATION bin)

    # Install asset directories
    foreach(ASSET_DIR ${ASSET_DIRS})
        install(DIRECTORY "${ROOT}/${ASSET_DIR}/" DESTINATION "share/chaotic-daw/${ASSET_DIR}")
    endforeach()

    # install(FILES ${ROOT}/chaotic-daw.desktop DESTINATION share/applications)
    # install(FILES ${ROOT}/icons/chaotic-daw.png DESTINATION share/icons/hicolor/256x256/apps)
endif()

# ── Summary ───────────────────────────────────────────────────────────────────
message(STATUS "Platform: WIN32=${USE_WIN32}, LINUX=${USE_LINUX}, MAC=${USE_MAC}")
message(STATUS "Sources: ${MAIN_SOURCES}")
if(USE_WIN32)
    message(STATUS "Using Windows‑specific PortAudio and ASIO backends")
elseif(USE_LINUX)
    message(STATUS "Using JUCE audio with ALSA/JACK backends")
endif()
