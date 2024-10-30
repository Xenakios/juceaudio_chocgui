cmake_minimum_required(VERSION 3.22)
project(CONSOLE_APP_EXAMPLE VERSION 0.0.1)
add_subdirectory(../JUCE ignore EXCLUDE_FROM_ALL)                    # If you've put JUCE in a subdirectory called JUCE
juce_add_console_app(ConsoleAppExample
    PRODUCT_NAME "Console App Example")     # The name of the final executable, which can differ from the target name

include_directories(choc/gui)

target_sources(ConsoleAppExample
    PRIVATE
        Main.cpp)

target_compile_definitions(ConsoleAppExample
    PRIVATE
        # JUCE_WEB_BROWSER and JUCE_USE_CURL would be on by default, but you might not need them.
        JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_console_app` call
        JUCE_USE_CURL=0)    # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_console_app` call

target_link_libraries(ConsoleAppExample
    PRIVATE
        # ConsoleAppData            # If you'd created a binary data target, you'd link to it here
        juce::juce_core
        juce::juce_audio_utils
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
