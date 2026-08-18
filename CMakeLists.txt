cmake_minimum_required(VERSION 3.20)

project(bad-apple C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)


# ============================================================
# SOURCE
# ============================================================

add_executable(bad-apple
    src/main.c
)


# ============================================================
# LINUX
# ============================================================

if(NOT EMSCRIPTEN)

    message(STATUS "Building Linux version")

    target_compile_options(bad-apple PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

endif()


# ============================================================
# WEBASSEMBLY
# ============================================================

if(EMSCRIPTEN)

    message(STATUS "Building WebAssembly version")

    set_target_properties(bad-apple PROPERTIES
        OUTPUT_NAME "main"
        RUNTIME_OUTPUT_DIRECTORY
            "${CMAKE_SOURCE_DIR}/web"
    )


    # --------------------------------------------------------
    # Emscripten linker
    # --------------------------------------------------------

    target_link_options(bad-apple PRIVATE

        "-O2"

        "--preload-file"
        "${CMAKE_SOURCE_DIR}/frames/frames.bin@/frames/frames.bin"

        "-sEXPORTED_FUNCTIONS=['_get_frame','_get_frame_count','_get_fps']"

        "-sEXPORTED_RUNTIME_METHODS=['UTF8ToString']"

        "-sALLOW_MEMORY_GROWTH=1"

    )


    # --------------------------------------------------------
    # Copy audio to web directory
    # --------------------------------------------------------

    add_custom_command(
        TARGET bad-apple
        POST_BUILD

        COMMAND ${CMAKE_COMMAND} -E copy
            "${CMAKE_SOURCE_DIR}/frames/BA.wav"
            "${CMAKE_SOURCE_DIR}/web/BA.wav"
    )

endif()