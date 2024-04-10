# **********************************************************************
#
#  App (test)
#
# **********************************************************************

cmake_minimum_required(VERSION 3.24)

cmake_policy(SET CMP0091 OLD) # not set MSVC default args

project(app LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(USE_LOCAL_MUSE_FRAMEWORK ON)

if (NOT USE_LOCAL_MUSE_FRAMEWORK)
    set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/_deps)
    include(FetchContent)
    FetchContent_Declare(
      muse_framework
      GIT_REPOSITORY git@github.com:musescore/framework_tmp.git
      GIT_TAG        HEAD
    )

    FetchContent_GetProperties(muse_framework)
    if(NOT muse_framework_POPULATED)
      FetchContent_Populate(muse_framework)
    endif()
else()

    set(muse_framework_SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/..)
    set(muse_framework_BINARY_DIR ${PROJECT_BINARY_DIR}/muse_framework)

endif()

message("muse_framework_SOURCE_DIR: ${muse_framework_SOURCE_DIR}")

set(MUSE_FRAMEWORK_PATH ${muse_framework_SOURCE_DIR})
set(MUSE_FRAMEWORK_SRC_PATH ${muse_framework_SOURCE_DIR}/framework)

set(CMAKE_MODULE_PATH
    ${CMAKE_CURRENT_LIST_DIR}
    ${MUSE_FRAMEWORK_PATH}/build/cmake
    ${CMAKE_MODULE_PATH}
    )

###########################################
# Setup option and build settings
###########################################
include(GetPaths)

set(APP_BUILD_CONFIGURATION "app" CACHE STRING "Build configuration")
# Possible values:
# - app             - for desktop app
# - app-portable    - for desktop portable app (Windows build for PortableApps.com)
# - utest           - for unit tests (for CI)

set(APP_BUILD_MODE "dev" CACHE STRING "Build mode")
# Possible values:
# - dev     - for development/nightly builds
# - testing - for testing versions (alpha, beta, RC)
# - release - for stable release builds

set(APP_REVISION "" CACHE STRING "Build revision")

include(muse_framework/DeclareOptions)

set(MUSE_MODULE_AUTOBOT OFF)
set(MUSE_MODULE_DRAW OFF)

# Modules (alphabetical order please)
option(MU_BUILD_APPSHELL_MODULE "Build appshell module" OFF)

# === Setup ===

# === Pack ===
option(MU_RUN_LRELEASE "Generate .qm files" ON)

# === Tests ===
option(MU_BUILD_UNIT_TESTS "Build unit tests" OFF)
option(MU_BUILD_ASAN "Enable Address Sanitizer" OFF)
set(MU_CRASH_REPORT_URL "" CACHE STRING "URL where to send crash reports")
option(MU_CRASHPAD_HANDLER_PATH "Path to custom crashpad_handler executable (optional)" "")

# === Tools ===

# === Compile ===
option(MU_COMPILE_BUILD_MACOS_APPLE_SILICON "Build for Apple Silicon architecture. Only applicable on Macs with Apple Silicon, and requires suitable Qt version." OFF)
option(MU_COMPILE_INSTALL_QTQML_FILES "Whether to bundle qml files along with the installation (relevant on MacOS only)" ON)
option(MU_COMPILE_USE_PCH "Use precompiled headers." ON)
option(MU_COMPILE_USE_UNITY "Use unity build." ON)
option(MU_COMPILE_USE_CCACHE "Try use ccache" ON)

# === Debug ===
option(MU_ENABLE_STRING_DEBUG_HACK "Enable string debug hack (only clang)" ON)


###########################################
# Setup Configure
###########################################
add_definitions(-DQT_SUPPORT)
add_definitions(-DKORS_LOGGER_QT_SUPPORT)
include(muse_framework/SetupConfigure)

###########################################
# Setup compiler and build environment
###########################################

include(SetupBuildEnvironment)
include(GetPlatformInfo)
if (MUE_COMPILE_USE_CCACHE)
    include(TryUseCcache)
endif(MUE_COMPILE_USE_CCACHE)


###########################################
# Setup external dependencies
###########################################
set(QT_MIN_VERSION "6.2.4")
include(FindQt6)

if (OS_IS_WIN)
    include(FetchContent)
    FetchContent_Declare(
      muse_prebuild_win_deps
      GIT_REPOSITORY https://github.com/musescore/musescore_prebuild_win_deps.git
      GIT_TAG        HEAD
    )
    FetchContent_MakeAvailable(muse_prebuild_win_deps)
    set(DEPENDENCIES_DIR ${muse_prebuild_win_deps_SOURCE_DIR})
    set(DEPENDENCIES_LIB_DIR ${DEPENDENCIES_DIR}/libx64)
    set(DEPENDENCIES_INC ${DEPENDENCIES_DIR}/include)
endif(OS_IS_WIN)

include(FindSndFile)

###########################################
# Add source tree
###########################################

add_subdirectory(${muse_framework_SOURCE_DIR}/framework ${muse_framework_BINARY_DIR})
add_subdirectory(src)

###########################################
# Setup Packaging
###########################################
