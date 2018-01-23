cmake_minimum_required(VERSION 3.5)
project(zadanie3pw C)

set(CMAKE_C_STANDARD 11)

macro (add_executable _name)
    # invoke built-in add_executable
    _add_executable(${ARGV})
    if (TARGET ${_name})
        target_link_libraries(${_name} rt m)
    endif()
endmacro()

set(CMAKE_C_FLAGS "-O3 -Wno-unused-result")

set(SOURCE_FILES1 tester.c head.c)
add_executable(tester ${SOURCE_FILES1})
set(SOURCE_FILES2 validator.c head.c)
add_executable(validator ${SOURCE_FILES2})
set(SOURCE_FILES3 run.c head.c)
add_executable(run ${SOURCE_FILES3})
