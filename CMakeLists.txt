cmake_minimum_required(VERSION 3.15)

set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type.")
set(BUILD_SHARED_LIBS ON CACHE BOOL "Build a shared library.")
set(BUILD_STATIC_LIBS ON CACHE BOOL "Build a static library.")

if (NOT DEFINED LUA_XZ_BUILD_TESTS)
    set(LUA_XZ_BUILD_TESTS "${BUILD_TESTING}")
endif()

if (LUA_XZ_BUILD_TESTS)
    set(LUA_XZ_BUILD_TESTS ON)

    if (BUILD_STATIC_LIBS AND NOT BUILD_SHARED_LIBS)
        message(WARNING "Tests are NOT built for the static library. Only the shared library gets tested.")
        set(LUA_XZ_BUILD_TESTS OFF)
    endif()
else()
    set(LUA_XZ_BUILD_TESTS OFF)
endif()

set(_IS_LUAJIT OFF)

set(LUA_XZ_DEF "${CMAKE_CURRENT_SOURCE_DIR}/lua-xz.def")
set(LUA_XZ_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(LUA_XZ_SOURCES "${LUA_XZ_SRC_DIR}/lua-xz.c")
set(LUA_XZ_HEADERS "${LUA_XZ_SRC_DIR}/lua-xz.h")
set(LUA_XZ_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.md")
set(LUA_XZ_TEST_DIR "${CMAKE_CURRENT_SOURCE_DIR}/samples")
set(LUA_XZ_TESTS_SOURCES
    "${LUA_XZ_TEST_DIR}/lzma-compress-readme.lua"
    "${LUA_XZ_TEST_DIR}/lzma-decompress-copy-of-readme.lua"
    "${LUA_XZ_TEST_DIR}/lzma-simulate-compression-in-chunks.lua"
    "${LUA_XZ_TEST_DIR}/test-constants.lua"
    "${LUA_XZ_TEST_DIR}/xz-compress-readme.lua"
    "${LUA_XZ_TEST_DIR}/xz-decompress-copy-of-readme.lua"
    "${LUA_XZ_TEST_DIR}/xz-simulate-compression-in-chunks.lua")

file(READ ${LUA_XZ_HEADERS} __LUA_XZ_HEADERS_CONTENT)

# regexing the project version from src/lua-xz.h 
foreach(v "MAJOR" "MINOR" "PATCH")
    string(REGEX MATCH "#define LUA_XZ_BINDING_VERSION_${v} \"([0-9]+)\"" _LUA_XZ_${v}_VERSION "${__LUA_XZ_HEADERS_CONTENT}")

    if ("${_LUA_XZ_${v}_VERSION}" STREQUAL "")
        message(FATAL_ERROR "Failed to find the ${v} version of the binding")
    else()
        set(_LUA_XZ_${v}_VERSION "${CMAKE_MATCH_1}")
    endif()
endforeach()

project(
    lua-xz
    VERSION "${_LUA_XZ_MAJOR_VERSION}.${_LUA_XZ_MINOR_VERSION}.${_LUA_XZ_PATCH_VERSION}"
    LANGUAGES C
)

enable_testing()
include(GNUInstallDirs)

# require at least one build type
if (NOT (BUILD_SHARED_LIBS OR BUILD_STATIC_LIBS))
    message(FATAL_ERROR "You must build at least a shared or static library.")
endif()

# Lua version resolution
if (DEFINED LUA_VERSION)
    # find LuaJIT
    if ("${LUA_VERSION}" MATCHES "^luajit-?(.*)")
        set(LuaJIT_INCLUDE_DIR_SUFFIXES)
        set(LuaJIT_ID_SUFFIX "${CMAKE_MATCH_1}")
        if ("${LuaJIT_ID_SUFFIX}" STREQUAL "" OR "${LuaJIT_ID_SUFFIX}" STREQUAL "openresty")
            list(APPEND LuaJIT_INCLUDE_DIR_SUFFIXES "luajit-2.0" "luajit-2.1")
        elseif ("${LuaJIT_ID_SUFFIX}" MATCHES "^[0-9]\\.[0-9]$")
            list(APPEND LuaJIT_INCLUDE_DIR_SUFFIXES "luajit-${LuaJIT_ID_SUFFIX}")
        else()
            message(FATAL_ERROR "Unknown LuaJIT version. Valid arguments are 'luajit', 'luajit-2.0', 'luajit-2.1' or 'luajit-openresty'")
        endif()

        find_path(LuaJIT_INCLUDE_DIR NAME lua.h PATH_SUFFIXES ${LuaJIT_INCLUDE_DIR_SUFFIXES} REQUIRED)
        get_filename_component(LuaJIT_DIR "${LuaJIT_INCLUDE_DIR}" PATH)
        get_filename_component(LuaJIT_DIR "${LuaJIT_DIR}" PATH)

        foreach(h "lua.h" "luaconf.h" "lualib.h" "lauxlib.h" "lua.hpp")
            if (NOT EXISTS "${LuaJIT_INCLUDE_DIR}/${h}")
                message(STATUS "${LuaJIT_INCLUDE_DIR}/${h} not found")
                message(FATAL_ERROR "Unable to use LuaJIT (${LUA_VERSION}), because a broken installation was found at ${LuaJIT_DIR}.")
            endif()
        endforeach()

        find_library(LuaJIT_LIBRARY NAMES luajit luajit-5.1 PATHS "${LuaJIT_DIR}" REQUIRED)

        # LuaJIT_LIBRARY must share the same LuaJIT_DIR base directory
        get_filename_component(__LuaJIT_PARENT_LIBRARY_DIRS "${LuaJIT_LIBRARY}" PATH)
        set(__LuaJIT_PARENT_COUNTER 0)
        while (${__LuaJIT_PARENT_COUNTER} LESS 3 AND NOT ("${__LuaJIT_PARENT_LIBRARY_DIRS}" STREQUAL "${LuaJIT_DIR}"))
            get_filename_component(__LuaJIT_PARENT_LIBRARY_DIRS "${__LuaJIT_PARENT_LIBRARY_DIRS}" PATH)
            math(EXPR __LuaJIT_PARENT_COUNTER "${__LuaJIT_PARENT_COUNTER} + 1")
        endwhile()

        if (NOT "${__LuaJIT_PARENT_LIBRARY_DIRS}" STREQUAL "${LuaJIT_DIR}")
            message(FATAL_ERROR "Unable to use LuaJIT (${LUA_VERSION}), because a library was not found.")
        endif()

        # Getting the interpreter for tests
        if (LUA_XZ_BUILD_TESTS)
            find_program(LuaJIT_INTERPRETER NAMES luajit PATHS "${LuaJIT_DIR}" REQUIRED)

            # LuaJIT_INTERPRETER must share the same LuaJIT_DIR base directory
            get_filename_component(__LuaJIT_PARENT_INTERPRETER_DIRS "${LuaJIT_INTERPRETER}" PATH)
            set(__LuaJIT_PARENT_COUNTER 0)
            while (${__LuaJIT_PARENT_COUNTER} LESS 2 AND NOT ("${__LuaJIT_PARENT_INTERPRETER_DIRS}" STREQUAL "${LuaJIT_DIR}"))
                get_filename_component(__LuaJIT_PARENT_INTERPRETER_DIRS "${__LuaJIT_PARENT_INTERPRETER_DIRS}" PATH)
                math(EXPR __LuaJIT_PARENT_COUNTER "${__LuaJIT_PARENT_COUNTER} + 1")
            endwhile()

            if (NOT "${__LuaJIT_PARENT_INTERPRETER_DIRS}" STREQUAL "${LuaJIT_DIR}")
                message(FATAL_ERROR "Unable to use LuaJIT (${LUA_VERSION}) to run tests, because interpreter was not found.")
            endif()

            set(LUA_INTERPRETER "${LuaJIT_INTERPRETER}")
        endif()

        set(LUA_INCLUDE_DIR "${LuaJIT_INCLUDE_DIR}")
        set(LUA_LIBRARIES "${LuaJIT_LIBRARY}")

        set(_IS_LUAJIT ON)

        message(STATUS "Found LuaJIT: ${LUA_LIBRARIES} (found suitable version \"${LUA_VERSION}\")")
        message(STATUS "LuaJIT include directory: ${LUA_INCLUDE_DIR}")
        message(STATUS "LuaJIT library: ${LUA_LIBRARIES}")

    elseif("${LUA_VERSION}" MATCHES "^([0-9]+)\\.([0-9]+)$")
        if ("${LUA_VERSION}" VERSION_LESS "5.1")
            message(FATAL_ERROR "This version of Lua is not supported.")
        endif()

        find_package(Lua "${LUA_VERSION}" EXACT REQUIRED)
        message(STATUS "Lua include directory: ${LUA_INCLUDE_DIR}")
        message(STATUS "Lua libraries: ${LUA_LIBRARIES}")        
    else()
        message(FATAL_ERROR "Unknown LUA_VERSION format. Valid arguments are X.Y (e.g.: 5.1, 5.4), 'luajit', 'luajit-2.0', 'luajit-2.1' or 'luajit-openresty'")
    endif()

# Lua version not provided.
# Assume that the latest Lua version
# provided by the system is the one
# wanted by the user.
else()
    find_package(Lua 5.1 REQUIRED)
    message(STATUS "Lua include directory: ${LUA_INCLUDE_DIR}")
    message(STATUS "Lua libraries: ${LUA_LIBRARIES}")
    set(LUA_VERSION "${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}")
endif()

# Get Lua interpreter for tests,
# but exclude LuaJIT, because
# LuaJIT interpreter was already found earlier.
if (LUA_XZ_BUILD_TESTS AND NOT _IS_LUAJIT)
    # Getting the interpreter for tests
    if (LUA_XZ_BUILD_TESTS)
        find_program(LUA_INTERPRETER
            NAMES
                "lua${LUA_VERSION_MAJOR}${LUA_VERSION_MINOR}"
                "lua${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
                "lua-${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
                "lua.${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
                "lua"
            REQUIRED)

        get_filename_component(Lua_DIR "${LUA_INTERPRETER}" PATH)
        get_filename_component(Lua_DIR "${Lua_DIR}" PATH)

        set(_guessed_lua_incdirs
            "${Lua_DIR}/include/lua${LUA_VERSION_MAJOR}${LUA_VERSION_MINOR}"
            "${Lua_DIR}/include/lua${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
            "${Lua_DIR}/include/lua-${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
            "${Lua_DIR}/include/lua.${LUA_VERSION_MAJOR}.${LUA_VERSION_MINOR}"
            "${Lua_DIR}/include"
        )
        set(_found_incdir OFF)

        foreach(_lua_incdir ${_guessed_lua_incdirs})
            if ("${_lua_incdir}" STREQUAL "${LUA_INCLUDE_DIR}")
                set(_found_incdir ON)
                break()
            endif()
        endforeach()
        
        if (NOT _found_incdir)
            message(FATAL_ERROR "Unable to use Lua (${LUA_VERSION}) to run tests, because interpreter was not found.")
        endif()
    endif()
endif()

find_package(LibLZMA 5.0.0 REQUIRED)

# On Windows, when going for tests,
# add the binary directory of
# liblzma to the system PATH
# environment variable
if (WIN32 AND LUA_XZ_BUILD_TESTS)
    
    get_filename_component(_LIBLZMA_DIR "${LIBLZMA_INCLUDE_DIR}" PATH)
    get_filename_component(_LIBLZMA_DIR "${_LIBLZMA_DIR}" PATH)

    set(LIBLZMA_BINDIR "${_LIBLZMA_DIR}/bin")
    if (NOT IS_DIRECTORY "${LIBLZMA_BINDIR}")
        message(FATAL_ERROR "Binary directory for liblzma was not found.")
    endif()
endif()

set(LUA_XZ_SHARED "lua-xz")
set(LUA_XZ_STATIC "lua-xz-static")
set(_targets_to_install)

# Enable @rpath in the install name
set(CMAKE_MACOSX_RPATH ON)

STRING(TOUPPER "${CMAKE_BUILD_TYPE}" _BUILD_TYPE_UPPER)

foreach(kind "SHARED" "STATIC")
    if (BUILD_${kind}_LIBS)
        set(_target_kind ${LUA_XZ_${kind}})
        add_library(${_target_kind} ${kind} ${LUA_XZ_SOURCES})
        target_include_directories(${_target_kind} PRIVATE ${LUA_INCLUDE_DIR} ${LIBLZMA_INCLUDE_DIRS})
        if ("${_BUILD_TYPE_UPPER}" STREQUAL "RELEASE")
            target_compile_definitions(${_target_kind} PRIVATE "NDEBUG" "_NDEBUG")
        endif()
        target_compile_definitions(${_target_kind} PRIVATE "LUA_XZ_BUILD_${kind}")
        if (${kind} STREQUAL "SHARED")
            if (WIN32)
                target_sources(${_target_kind} PRIVATE "${LUA_XZ_DEF}")
            endif()
            target_link_libraries(${_target_kind} PRIVATE ${LUA_LIBRARIES} ${LIBLZMA_LIBRARIES})
            if (APPLE)
                target_link_options(${_target_kind} PRIVATE "LINKER:-undefined,dynamic_lookup")
                set_target_properties(${_target_kind} PROPERTIES SUFFIX ".so")
            endif()
            set_target_properties(${_target_kind}
                PROPERTIES
                OUTPUT_NAME ${_target_kind}
                PUBLIC_HEADER ${LUA_XZ_HEADERS}
                PREFIX "")

            if (LUA_XZ_BUILD_TESTS)
                foreach(t ${LUA_XZ_TESTS_SOURCES})
                    get_filename_component(tn "${t}" NAME_WE)
                    add_test(NAME "${tn}"
                        COMMAND "${LUA_INTERPRETER}" "${t}"
                        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
                    if (WIN32)
                        set_tests_properties("${tn}" PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_CURRENT_BINARY_DIR}/?.dll" "PATH=${LIBLZMA_BINDIR}\;$ENV{PATH}")
                    else()
                        set_tests_properties("${tn}" PROPERTIES ENVIRONMENT "LUA_CPATH=${CMAKE_CURRENT_BINARY_DIR}/?.so")
                    endif()
                endforeach()
            endif()
        else()
            set_target_properties(${_target_kind}
                PROPERTIES
                OUTPUT_NAME ${_target_kind}
                PUBLIC_HEADER ${LUA_XZ_HEADERS})
        endif()

        list(APPEND _targets_to_install ${_target_kind})
    endif()
endforeach()

install(FILES "${LUA_XZ_LICENSE}" DESTINATION "${CMAKE_INSTALL_DOCDIR}")

install(TARGETS ${_targets_to_install}
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

message(STATUS "")
message(STATUS "")
message(STATUS " ${PROJECT_NAME} ${PROJECT_VERSION} configuration summary:")
message(STATUS "")
message(STATUS "   Dependencies:")
message(STATUS "")
if (_IS_LUAJIT)
message(STATUS "     LuaJIT (${LUA_VERSION}):")
else()
message(STATUS "     Lua (${LUA_VERSION}):")
endif()
message(STATUS "")
if (LUA_XZ_BUILD_TESTS)
message(STATUS "       Interpreter ................... : ${LUA_INTERPRETER}")
endif()
message(STATUS "       Libraries ..................... : ${LUA_LIBRARIES}")
message(STATUS "       Include dirs .................. : ${LUA_INCLUDE_DIR}")
message(STATUS "")
message(STATUS "     liblzma:")
message(STATUS "")
message(STATUS "       Libraries ..................... : ${LIBLZMA_LIBRARIES}")
message(STATUS "       Include dirs .................. : ${LIBLZMA_INCLUDE_DIRS}")
message(STATUS "")
message(STATUS "   Build options:")
message(STATUS "")
message(STATUS "     Build type ...................... : ${CMAKE_BUILD_TYPE}")
message(STATUS "     Build shared libs ............... : ${BUILD_SHARED_LIBS}")
message(STATUS "     Build static libs ............... : ${BUILD_STATIC_LIBS}")
if (LUA_XZ_BUILD_TESTS)
message(STATUS "     Build tests ..................... : ON")
else()
message(STATUS "     Build tests ..................... : OFF")
endif()
message(STATUS " ")
message(STATUS "   Install directories:")
message(STATUS " ")
message(STATUS "     Install prefix .................. : ${CMAKE_INSTALL_PREFIX}")
message(STATUS "     Binary dir ...................... : ${CMAKE_INSTALL_BINDIR}")
message(STATUS "     Lib dir ......................... : ${CMAKE_INSTALL_LIBDIR}")
message(STATUS "     Include dir ..................... : ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "     Doc dir ......................... : ${CMAKE_INSTALL_DOCDIR}")
message(STATUS "")
message(STATUS "")