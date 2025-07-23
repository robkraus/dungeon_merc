cmake_minimum_required(VERSION 3.16)
project(DungeonMerc VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O2")

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Find OpenSSL
find_package(OpenSSL REQUIRED)

# Find libssh2 (try pkg-config first, then manual paths)
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(SSH2 QUIET libssh2)
endif()

if(NOT SSH2_FOUND)
    # Manual libssh2 detection for Homebrew
    find_path(SSH2_INCLUDE_DIRS
        NAMES libssh2.h
        PATHS /opt/homebrew/include /usr/local/include /usr/include
    )
    
    find_library(SSH2_LIBRARIES
        NAMES ssh2 libssh2
        PATHS /opt/homebrew/lib /usr/local/lib /usr/lib
    )
    
    if(SSH2_INCLUDE_DIRS AND SSH2_LIBRARIES)
        set(SSH2_FOUND TRUE)
    endif()
endif()

if(NOT SSH2_FOUND)
    message(FATAL_ERROR "libssh2 not found. Please install it via: brew install libssh2")
endif()

# Include directories
include_directories(include)
if(SSH2_INCLUDE_DIRS)
    include_directories(${SSH2_INCLUDE_DIRS})
endif()

# Add source files
file(GLOB_RECURSE SOURCES "src/*.cpp")
file(GLOB_RECURSE HEADERS "include/*.hpp")

# Create executable
add_executable(dungeon_merc ${SOURCES} ${HEADERS})

# Link libraries
target_link_libraries(dungeon_merc Threads::Threads ${SSH2_LIBRARIES} OpenSSL::SSL OpenSSL::Crypto)
if(SSH2_CFLAGS_OTHER)
    target_compile_options(dungeon_merc PRIVATE ${SSH2_CFLAGS_OTHER})
endif()

# Set output directory
set_target_properties(dungeon_merc PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add tests if enabled
option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif()
