# Compatibility
cmake_minimum_required(VERSION 2.6)

if(WIN32)
    # Windows XP compatibility. Must be set before everything else.
    set(CMAKE_GENERATOR_TOOLSET "v120_xp")
endif()

# Project name
project(sp)

# Project version
set(${PROJECT_NAME}_MAJOR_VERSION 0)
set(${PROJECT_NAME}_MINOR_VERSION 1)
set(${PROJECT_NAME}_PATCH_LEVEL 0)

set(SPAWNER_PATH "${CMAKE_BINARY_DIR}/sp")
set(BUILD_DIR "${CMAKE_BINARY_DIR}")
set(ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(PROTOTYPE_TESTER "${ROOT_DIR}/bin/sp.exe")

add_subdirectory("${PROJECT_SOURCE_DIR}/libspawner" libspawner)

include_directories("${PROJECT_SOURCE_DIR}/inc")
include_directories("${PROJECT_SOURCE_DIR}/libspawner")
include_directories("${PROJECT_SOURCE_DIR}/lib/rapidjson/include")

if(UNIX)
    include_directories("${PROJECT_SOURCE_DIR}/libspawner/inc/posix")
elseif(WIN32)
    include_directories("${PROJECT_SOURCE_DIR}/libspawner/inc/win32")
endif()

# Always include symbols
if(UNIX)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

if(CMAKE_BUILD_TYPE)
else()
    set(CMAKE_BUILD_TYPE Release)
endif()

set(SP_BUILD_TYPE ${CMAKE_BUILD_TYPE})

set(SOURCES
    src/arguments.cpp
    src/main.cpp
    src/spawner_base.cpp
    src/spawner_new.cpp
    src/spawner_old.cpp
    src/spawner_pcms2.cpp
)

set(HEADERS
    inc/arguments.h
    inc/spawner_base.h
    inc/spawner_new.h
    inc/spawner_old.h
    inc/spawner_pcms2.h
)

set(LIBRARIES libspawner)

if(WIN32)
    find_library(USERENV userenv)
    if(USERENV)
        set(LIBRARIES ${LIBRARIES} ${USERENV})
    endif()
endif()

if(UNIX)
    # Linux requires -lrt for shm_{open,...}
    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        set(LIBRARIES ${LIBRARIES} "rt")
    endif()
    set(LIBRARIES ${LIBRARIES} "pthread")
endif()

set(PROJECT_EXECUTABLE ${PROJECT_NAME})
set(BIN_PATH "../bin/new")
set(BIN_SPECIFIC_PATH "../bin/new/${CMAKE_GENERATOR}")

add_executable(${PROJECT_EXECUTABLE} ${SOURCES} ${HEADERS})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${BIN_PATH}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${BIN_SPECIFIC_PATH}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:${PROJECT_NAME}> ${BIN_SPECIFIC_PATH}
)

target_link_libraries(${PROJECT_EXECUTABLE} ${LIBRARIES})

set_property(TARGET sp PROPERTY CXX_STANDARD 11)
set_property(TARGET sp PROPERTY CXX_STANDARD_REQUIRED ON)
set_property(TARGET libspawner PROPERTY CXX_STANDARD 11)
set_property(TARGET libspawner PROPERTY CXX_STANDARD_REQUIRED ON)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(CMAKE_CXX_FLAGS "--stdlib=libc++ ${CMAKE_CXX_FLAGS}")
endif()

if(MSVC)
    # MultiThreaded static.
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
endif()
