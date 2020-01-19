cmake_minimum_required(VERSION 3.13)
project(Sudoku)

set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} \
        -std=gnu++2a \
        -pedantic \
        -Wall \
        -Wextra\
         \
        -Wconversion \
        -Wenum-compare \
        -Wfloat-equal \
        -Woverloaded-virtual \
    "
    )

set(STATIC_BUILD TRUE)
if (CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3")
    set(STATIC_BUILD TRUE)
endif ()

if (STATIC_BUILD)
    set(CMAKE_EXE_LINKER_FLAGS "-static -static-libgcc")
endif ()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Executable/bin)
set(ENV_ROOT "D:/Programms/ProgrammingEnvironment")


# Magic for add ico to exe
if (MINGW)
    set(CMAKE_RC_COMPILER_INIT windres)
    ENABLE_LANGUAGE(RC)
    SET(CMAKE_RC_COMPILE_OBJECT "<CMAKE_RC_COMPILER> <FLAGS> <DEFINES> -o <OBJECT> <SOURCE>")
endif (MINGW)

add_executable(${PROJECT_NAME}
    config.rc # Add ico to exe
    main.cpp

    )











