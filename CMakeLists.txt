####################################################################################################################
#                                                                                                                  #
# The MIT License (MIT)                                                                                            #
#                                                                                                                  #
# Copyright (c) 2025 luau-project [https://github.com/luau-project/lua-xz](https://github.com/luau-project/lua-xz) #
#                                                                                                                  #
# Permission is hereby granted, free of charge, to any person obtaining a copy                                     #
# of this software and associated documentation files (the "Software"), to deal                                    #
# in the Software without restriction, including without limitation the rights                                     #
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell                                        #
# copies of the Software, and to permit persons to whom the Software is                                            #
# furnished to do so, subject to the following conditions:                                                         #
#                                                                                                                  #
# The above copyright notice and this permission notice shall be included in all                                   #
# copies or substantial portions of the Software.                                                                  #
#                                                                                                                  #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR                                       #
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,                                         #
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE                                      #
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER                                           #
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,                                    #
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE                                    #
# SOFTWARE.                                                                                                        #
#                                                                                                                  #
####################################################################################################################

cmake_minimum_required(VERSION 3.10)

##########################################################
# Project definitions                                    #
# -------------------------------------------------------#

set(_LUA_XZ_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(_LUA_XZ_TESTS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/samples")
set(_LUA_XZ_INCLUDE_DIR "${_LUA_XZ_SRC_DIR}")
set(_LUA_XZ_PUBLIC_HEADER "${_LUA_XZ_SRC_DIR}/lua-xz.h")
set(_LUA_XZ_SOURCE_C_FILES "${_LUA_XZ_SRC_DIR}/lua-xz.c")
set(_LUA_XZ_DEF_FILE "${CMAKE_CURRENT_SOURCE_DIR}/lua-xz.def")
set(_LUA_XZ_SHARED_LIB_FILES "${_LUA_XZ_SOURCE_C_FILES}")
set(_LUA_XZ_TESTS_FILES
    "${_LUA_XZ_TESTS_DIR}/lzma-compress-readme.lua"
    "${_LUA_XZ_TESTS_DIR}/xz-compress-readme.lua"
    "${_LUA_XZ_TESTS_DIR}/lzma-simulate-compression-in-chunks.lua"
    "${_LUA_XZ_TESTS_DIR}/xz-decompress-copy-of-readme.lua"
    "${_LUA_XZ_TESTS_DIR}/test-constants.lua"
    "${_LUA_XZ_TESTS_DIR}/lzma-decompress-copy-of-readme.lua"
    "${_LUA_XZ_TESTS_DIR}/xz-simulate-compression-in-chunks.lua"
)
set(_LUA_XZ_CLEAN_TESTS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/lua-xz.lzma"
    "${CMAKE_CURRENT_SOURCE_DIR}/lua-xz.xz"
    "${CMAKE_CURRENT_SOURCE_DIR}/lzma-copy-of-README.md"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md.lzma"
    "${CMAKE_CURRENT_SOURCE_DIR}/README.md.xz"
    "${CMAKE_CURRENT_SOURCE_DIR}/xz-copy-of-README.md"
)

if (WIN32)
    list(APPEND _LUA_XZ_SHARED_LIB_FILES "${_LUA_XZ_DEF_FILE}")
endif()

# extract the library version from the public header
file(STRINGS "${_LUA_XZ_PUBLIC_HEADER}" _LUA_XZ_PUBLIC_HEADER_CONTENT)

foreach(KIND "MAJOR" "MINOR" "PATCH")
    string(
        REGEX
        REPLACE ".*;#define[ \t]+LUA_XZ_BINDING_VERSION_${KIND}[ \t]+\"([0-9])\"[ \t]*;.*"
        "\\1"
        LUA_XZ_VERSION_${KIND} ";${_LUA_XZ_PUBLIC_HEADER_CONTENT};"
    )
    if (NOT LUA_XZ_VERSION_${KIND} MATCHES "^[0-9]+$")
        message(FATAL_ERROR "Unable to find lua-xz ${KIND} version from the public header `${_LUA_XZ_PUBLIC_HEADER}'")
    endif()
endforeach()

project(
    lua-xz
    VERSION "${LUA_XZ_VERSION_MAJOR}.${LUA_XZ_VERSION_MINOR}.${LUA_XZ_VERSION_PATCH}"
    LANGUAGES C
)

include(GNUInstallDirs)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "The build type")
option(LUA_XZ_TESTS "Run tests." ON)

##########################################################
# Detect the version of Lua in the LUA_VERSION parameter #
# -------------------------------------------------------#
# Rules:                                                 #
#   * PUC-Lua: X.Y.Z or X.Y format                       #
#   * LuaJIT: luajit                                     #
#   * OpenResty: openresty                               #
##########################################################

if (DEFINED LUA_VERSION)
    if ("${LUA_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)(\\.[0-9]+)?$")
        find_package(Lua "${LUA_VERSION}" EXACT REQUIRED)
        set(_LUA_MAJOR_VERSION "${CMAKE_MATCH_1}")
        set(_LUA_MINOR_VERSION "${CMAKE_MATCH_2}")

        if ("${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}" VERSION_LESS "5.1")
            message(FATAL_ERROR "Lua version must be 5.1 or newer.")
        endif()
    elseif("${LUA_VERSION}" MATCHES "^(luajit|openresty)$")
        set(_LUA_MAJOR_VERSION "5")
        set(_LUA_MINOR_VERSION "1")

        set(_LUAJIT_INCLUDE_DIR_FOUND FALSE)
        foreach(_LUAJIT_VERSION "2.0" "2.1")
            find_path(
                _LUAJIT_INCLUDE_DIR
                NAMES "luajit.h"
                PATH_SUFFIXES "include/luajit-${_LUAJIT_VERSION}"
            )
            if (_LUAJIT_INCLUDE_DIR)
                set(_LUAJIT_INCLUDE_DIR_FOUND TRUE)
            endif()
            if (_LUAJIT_INCLUDE_DIR_FOUND)
                foreach(_LUAJIT_HEADER "lua.h" "lauxlib.h" "lualib.h" "luaconf.h" "lua.hpp")
                    if (NOT EXISTS "${_LUAJIT_INCLUDE_DIR}/${_LUAJIT_HEADER}")
                        set(_LUAJIT_INCLUDE_DIR_FOUND FALSE)
                        break()
                    endif()
                endforeach()

                if (_LUAJIT_INCLUDE_DIR_FOUND)
                    get_filename_component(_LUAJIT_DIR "${_LUAJIT_INCLUDE_DIR}" PATH)
                    get_filename_component(_LUAJIT_DIR "${_LUAJIT_DIR}" PATH)
                endif()

                if (_LUAJIT_INCLUDE_DIR_FOUND)
                    set(_LUAJIT_BINDIR "${_LUAJIT_DIR}/bin")
                    if (NOT EXISTS "${_LUAJIT_BINDIR}")
                        set(_LUAJIT_INCLUDE_DIR_FOUND FALSE)
                    endif()
                endif()

                if (_LUAJIT_INCLUDE_DIR_FOUND)
                    if (WIN32)
                        if (MSVC OR "${CMAKE_C_SIMULATE_ID}" MATCHES "MSVC")
                            set(_LUAJIT_IMPORT_LIBRARY "${_LUAJIT_DIR}/lib/lua51.lib")
                        elseif (EXISTS "${_LUAJIT_DIR}/lib/libluajit-5.1.dll.a")
                            set(_LUAJIT_IMPORT_LIBRARY "${_LUAJIT_DIR}/lib/libluajit-5.1.dll.a")
                        else()
                            set(_LUAJIT_IMPORT_LIBRARY "${_LUAJIT_BINDIR}/lua51.dll")
                        endif()
                        if (NOT EXISTS "${_LUAJIT_IMPORT_LIBRARY}")
                            set(_LUAJIT_INCLUDE_DIR_FOUND FALSE)
                        endif()
                    endif()
                endif()

                if (_LUAJIT_INCLUDE_DIR_FOUND)
                    break()
                endif()
            endif()
        endforeach()

        if (NOT _LUAJIT_INCLUDE_DIR_FOUND)
            message(FATAL_ERROR "Unable to find ${LUA_VERSION}")
        endif()
    else()
        message(FATAL_ERROR "Unknown `LUA_VERSION' format.")
    endif()
else()
    find_package(Lua "5.1" REQUIRED)
    set(_LUA_MAJOR_VERSION "${CMAKE_MATCH_1}")
    set(_LUA_MINOR_VERSION "${CMAKE_MATCH_2}")
endif()

function(GET_LUA_CMOD LUA_MAJOR_VERSION LUA_MINOR_VERSION)
    if (MINGW AND MSYS AND "$ENV{SHELL}" MATCHES "bash")
        # assume it is running on MSYS2 through bash
        set(LUA_CMOD "lib/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
    elseif (WIN32)
        if ("${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" VERSION_LESS "5.3")
            set(LUA_CMOD "bin" PARENT_SCOPE)
        else()
            set(LUA_CMOD "lib/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
        endif()
    elseif (EXISTS "/etc/debian_version")
        if (CMAKE_LIBRARY_ARCHITECTURE AND "${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
            set(LUA_CMOD "lib/${CMAKE_LIBRARY_ARCHITECTURE}/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
        else()
            set(LUA_CMOD "lib/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
        endif()
    elseif ("${CMAKE_INSTALL_PREFIX}" MATCHES "^/usr/?$")
        if ("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(LUA_CMOD "lib64/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
        else()
            set(LUA_CMOD "lib/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
        endif()
    else()
        set(LUA_CMOD "lib/lua/${LUA_MAJOR_VERSION}.${LUA_MINOR_VERSION}" PARENT_SCOPE)
    endif()
endfunction()

GET_LUA_CMOD("${_LUA_MAJOR_VERSION}" "${_LUA_MINOR_VERSION}")

# require liblzma
find_package(LibLZMA 5.0.0 REQUIRED)

##########################################################
# Main body for the project                              #
# -------------------------------------------------------#

# configuration
add_library(lua-xz-shared SHARED "${_LUA_XZ_SHARED_LIB_FILES}")
target_compile_definitions(lua-xz-shared PRIVATE "LUA_XZ_BUILD_SHARED")
target_include_directories(lua-xz-shared PRIVATE "${_LUA_XZ_INCLUDE_DIR}")
target_include_directories(lua-xz-shared PRIVATE "${LIBLZMA_INCLUDE_DIRS}")
if (_LUAJIT_INCLUDE_DIR_FOUND)
    target_include_directories(lua-xz-shared PRIVATE "${_LUAJIT_INCLUDE_DIR}")
else()
    target_include_directories(lua-xz-shared PRIVATE "${LUA_INCLUDE_DIR}")
endif()
target_link_libraries(lua-xz-shared PRIVATE "${LIBLZMA_LIBRARIES}")
set_target_properties(lua-xz-shared
    PROPERTIES
    OUTPUT_NAME "lua-xz"
    PUBLIC_HEADER "${_LUA_XZ_PUBLIC_HEADER}"
    PREFIX ""
)
if (WIN32)
    if (_LUAJIT_INCLUDE_DIR_FOUND)
        target_link_libraries(lua-xz-shared PRIVATE "${_LUAJIT_IMPORT_LIBRARY}")
    else()
        target_link_libraries(lua-xz-shared PRIVATE "${LUA_LIBRARIES}")
    endif()
elseif (APPLE)
    set_target_properties(lua-xz-shared
        PROPERTIES
        SUFFIX ".so"
        LINK_FLAGS " -undefined dynamic_lookup -all_load "
    )
endif()

# tests
if (LUA_XZ_TESTS)
    enable_testing()
    foreach(_LUA_XZ_TEST_FILE_TO_CLEAN ${_LUA_XZ_CLEAN_TESTS_FILES})
        if ("${CMAKE_VERSION}" VERSION_LESS "3.17")
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E remove -f "${_LUA_XZ_TEST_FILE_TO_CLEAN}"
            )
        else()
            execute_process(
                COMMAND ${CMAKE_COMMAND} -E rm -f "${_LUA_XZ_TEST_FILE_TO_CLEAN}"
            )
        endif()
    endforeach()

    if (_LUAJIT_INCLUDE_DIR_FOUND)
        find_program(_LUA_INTERPRETER
            NAMES
                "luajit"
                "lua${_LUA_MAJOR_VERSION}${_LUA_MINOR_VERSION}"
                "lua${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua-${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua.${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua"
            REQUIRED
        )
    else()
        find_program(_LUA_INTERPRETER
            NAMES
                "lua${_LUA_MAJOR_VERSION}${_LUA_MINOR_VERSION}"
                "lua${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua-${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua.${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}"
                "lua"
            REQUIRED
        )
    endif()

    execute_process(
        COMMAND "${_LUA_INTERPRETER}" "-e" "print(_VERSION:sub(5))"
        OUTPUT_VARIABLE _LUA_VERSION_OUTPUT
    )

    if (NOT "${_LUA_VERSION_OUTPUT}" MATCHES "^${_LUA_MAJOR_VERSION}\\.${_LUA_MINOR_VERSION}")
        message(FATAL_ERROR "The program `${_LUA_INTERPRETER}' is not compatible to Lua ${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}.")
    endif()

    foreach(_LUA_XZ_TEST_FILE ${_LUA_XZ_TESTS_FILES})
        get_filename_component(_LUA_XZ_TEST_NAME "${_LUA_XZ_TEST_FILE}" NAME_WE)
        add_test(
            NAME "${_LUA_XZ_TEST_NAME}"
            COMMAND "${_LUA_INTERPRETER}" "${_LUA_XZ_TEST_FILE}"
            WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
        )
        if (WIN32)
            get_filename_component(_LIBLZMA_DIR "${LIBLZMA_INCLUDE_DIR}" PATH)
            set(_LIBLZMA_BINDIR "${_LIBLZMA_DIR}/bin")
            set_tests_properties(
                "${_LUA_XZ_TEST_NAME}"
                PROPERTIES
                ENVIRONMENT "LUA_CPATH=${CMAKE_CURRENT_BINARY_DIR}/?.dll\;$ENV{LUA_CPATH};PATH=${_LIBLZMA_BINDIR}\;$ENV{PATH}"
            )
        else()
            set_tests_properties(
                "${_LUA_XZ_TEST_NAME}"
                PROPERTIES
                ENVIRONMENT "LUA_CPATH=${CMAKE_CURRENT_BINARY_DIR}/?.so\;$ENV{LUA_CPATH}"
            )
        endif()
    endforeach()
endif()

# install
if (WIN32)
    install(
        TARGETS lua-xz-shared
        RUNTIME DESTINATION "${LUA_CMOD}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
else()
    install(
        TARGETS lua-xz-shared
        LIBRARY DESTINATION "${LUA_CMOD}"
        PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
    )
endif()

message(STATUS "")
message(STATUS "")
message(STATUS "  ${PROJECT_NAME} ${PROJECT_VERSION} configuration summary:")
message(STATUS "")
message(STATUS "    Dependencies:")
message(STATUS "")
if (_LUAJIT_INCLUDE_DIR_FOUND)
if ("${LUA_VERSION}" MATCHES "^openresty$")
message(STATUS "      OpenResty")
else()
message(STATUS "      LuaJIT")
endif()
else()
message(STATUS "      Lua ${_LUA_MAJOR_VERSION}.${_LUA_MINOR_VERSION}")
endif()
message(STATUS "")
if (LUA_XZ_TESTS)
message(STATUS "        Interpreter ................... : ${_LUA_INTERPRETER}")
endif()
if (_LUAJIT_INCLUDE_DIR_FOUND)
message(STATUS "        Include dir ................... : ${_LUAJIT_INCLUDE_DIR}")
else()
if (WIN32)
foreach(lua_lib ${LUA_LIBRARIES})
message(STATUS "        Library ....................... : ${lua_lib}")
endforeach()
endif()
foreach(lua_incdir ${LUA_INCLUDE_DIRS})
message(STATUS "        Include dir ................... : ${lua_incdir}")
endforeach()
endif()
message(STATUS "")
message(STATUS "      liblzma:")
message(STATUS "")
foreach(libzma_lib ${LIBLZMA_LIBRARIES})
message(STATUS "        Library ....................... : ${libzma_lib}")
endforeach()
foreach(libzma_incdir ${LIBLZMA_INCLUDE_DIRS})
message(STATUS "        Include dir ................... : ${libzma_incdir}")
endforeach()
message(STATUS "")
message(STATUS "    Build options:")
message(STATUS "")
message(STATUS "      Build type ...................... : ${CMAKE_BUILD_TYPE}")
message(STATUS "")
if (LUA_XZ_TESTS)
message(STATUS "    Run tests ......................... : ON")
else()
message(STATUS "    Run tests ......................... : OFF")
endif()
message(STATUS "")
message(STATUS "    Install directories:")
message(STATUS "")
message(STATUS "      Install prefix .................. : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "      Include dir ..................... : ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "      Lua C modules ................... : ${LUA_CMOD}")
message(STATUS "")
message(STATUS "")