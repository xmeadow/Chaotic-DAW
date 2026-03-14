cmake_minimum_required(VERSION 3.15)
project(Chaotic-DAW CXX C)

# ── Platform guard ────────────────────────────────────────────────────────────
# This project targets Windows (Win32 API, ASIO, VST2 hosting).
# Build with MinGW-w64 on Windows, or cross-compile from Linux:
#   cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake ..
# ─────────────────────────────────────────────────────────────────────────────

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

# ── PortAudio core ────────────────────────────────────────────────────────────
set(PORTAUDIO_CORE
    "${ROOT}/External/common/pa_allocation.c"
    "${ROOT}/External/common/pa_converters.c"
    "${ROOT}/External/common/pa_cpuload.c"
    # "${ROOT}/External/common/pa_debugprint.c"  # Causes duplicate symbol with pa_front.c
    "${ROOT}/External/common/pa_dither.c"
    "${ROOT}/External/common/pa_front.c"
    "${ROOT}/External/common/pa_process.c"
    "${ROOT}/External/common/pa_stream.c"
    "${ROOT}/External/common/pa_trace.c"
    "${ROOT}/External/common/pa_skeleton.c"
    "${ROOT}/External/win/pa_win_hostapis.c"
    "${ROOT}/External/win/pa_win_util.c"
    # "${ROOT}/External/win/pa_x86_plain_converters.c"  # MSVC assembly, not compatible with MinGW/GCC
)

# ── PortAudio backends ────────────────────────────────────────────────────────
set(PORTAUDIO_BACKENDS
    "${ROOT}/External/asio/pa_asio.cpp"
    "${ROOT}/External/asio/iasiothiscallresolver.cpp"
    "${ROOT}/External/wasapi/pa_win_wasapi.cpp"
    "${ROOT}/External/wmme/pa_win_wmme.c"
    # "${ROOT}/External/dsound/pa_win_ds.c"
    # "${ROOT}/External/dsound/pa_win_ds_dynlink.c"
)

# ── ASIO SDK sources (needed by PortAudio ASIO backend) ───────────────────────
set(ASIO_SOURCES
    "${ROOT}/ASIOSDK2/common/asio.cpp"
    # "${ROOT}/ASIOSDK2/common/asiodrvr.cpp"  # Has #error do not use this for Windows
    "${ROOT}/ASIOSDK2/common/combase.cpp"
    "${ROOT}/ASIOSDK2/host/asiodrivers.cpp"
    "${ROOT}/ASIOSDK2/host/pc/asiolist.cpp"
)

# ── Main application sources ──────────────────────────────────────────────────
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

# ── Windows resource file ─────────────────────────────────────────────────────
if(MINGW OR MSVC)
    set(RESOURCE_FILE "${ROOT}/Awful.rc")
endif()

# ── Executable ───────────────────────────────────────────────────────────────
add_executable(${PROJECT_NAME} WIN32
    ${MAIN_SOURCES}
    ${ROSIC_SOURCES}
    ${DATA_SOURCES}
    ${PORTAUDIO_CORE}
    ${PORTAUDIO_BACKENDS}
    ${ASIO_SOURCES}
    ${RESOURCE_FILE}
)

# ── Include directories ───────────────────────────────────────────────────────
target_include_directories(${PROJECT_NAME} PRIVATE
    # Project root (StdAfx.h, resource.h, all project headers)
    ${ROOT}

    # VST2 compatibility headers (aeffectx.h, audioeffectx.h, AEffEditor.hpp)
    ${ROOT}/vst2compat

    # ASIO SDK headers
    ${ROOT}/ASIOSDK2/common
    ${ROOT}/ASIOSDK2/host
    ${ROOT}/ASIOSDK2/host/pc

    # PortAudio public API and backend headers
    ${ROOT}/External/include
    ${ROOT}/External/asio
    ${ROOT}/External/common
    ${ROOT}/External/win

    # JUCE
    ${ROOT}/juce

    # VST3 SDK base (funknown etc., not needed for VST2 hosting but listed for completeness)
    ${ROOT}/vstsdk3
    ${ROOT}/vstsdk3/pluginterfaces
)

# ── Compile definitions ───────────────────────────────────────────────────────
target_compile_definitions(${PROJECT_NAME} PRIVATE
    WIN32
    _WINDOWS
    # PortAudio backend selection
    PA_USE_ASIO=1
    PA_USE_WASAPI=1
    PA_USE_WMME=1
    PA_USE_DS=0
    PA_NO_DS=1
    # Suppress some common warnings from old code
    _CRT_SECURE_NO_WARNINGS
    _CRT_NONSTDC_NO_WARNINGS
    # Disable JUCE Unicode to avoid T macro conflict with rosic templates
    JUCE_STRINGS_ARE_UNICODE=0
    # Disable JUCE WebBrowser component to avoid missing Exdisp.h header
    JUCE_WEB_BROWSER=0
    # Disable JUCE OpenGL to avoid missing gl/gl.h header
    JUCE_OPENGL=0



)

# ── Compiler flags ────────────────────────────────────────────────────────────
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        /W1
        /MP           # parallel compilation
        /bigobj       # needed for JUCE amalgamated
    )
elseif(MINGW OR CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(${PROJECT_NAME} PRIVATE
        -w                  # suppress warnings (old code has many)
        -Wno-deprecated
        -fpermissive        # old JUCE and project code needs this
        -fno-check-new      # disable checking for out-of-memory on new
        -fno-enforce-eh-specs # don't enforce exception specs
        # JUCE amalgamated is large – allow large objects
        -Wa,-mbig-obj
    )
endif()

# ── Windows system libraries ──────────────────────────────────────────────────
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

# ── Bundled libsndfile ────────────────────────────────────────────────────────
# The repo ships a pre-built libsndfile-1 for Windows.
if(MINGW)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${ROOT}/libsndfile-1.lib"
    )
elseif(MSVC)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "${ROOT}/libsndfile-1.lib"
    )
endif()
