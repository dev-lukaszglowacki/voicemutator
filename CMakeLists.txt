cmake_minimum_required(VERSION 3.15)
project(VoiceMutator VERSION 1.0.0)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(JUCE)

juce_add_gui_app(VoiceMutator)

juce_generate_juce_header(VoiceMutator)

target_sources(VoiceMutator PRIVATE
    "Main.cpp"
    "MainComponent.cpp"
    "AudioRecorder.cpp"
    "RecordingThumbnail.cpp"
)

# Find dependencies using pkg-config
find_package(PkgConfig REQUIRED)
pkg_check_modules(DEPS REQUIRED gtk+-3.0 webkit2gtk-4.1 libcurl)

# Add include directories to the target
target_include_directories(VoiceMutator PRIVATE ${DEPS_INCLUDE_DIRS})
# Add other compiler flags
target_compile_options(VoiceMutator PRIVATE ${DEPS_CFLAGS_OTHER})

target_link_libraries(VoiceMutator PRIVATE
    ${DEPS_LIBRARIES}
    juce::juce_core
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_data_structures
)