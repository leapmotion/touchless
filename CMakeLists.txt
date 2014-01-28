cmake_minimum_required(VERSION 2.8)
if(COMMAND CMAKE_POLICY)
  cmake_policy(SET CMP0003 NEW)
endif(COMMAND CMAKE_POLICY)

# This version number must conform to semver.org (2.0.0-rc.1) standard for any release starting with 1.0.0
set(TOUCHLESS_VERSION_MAJOR 1)
set(TOUCHLESS_VERSION_MINOR 0)
set(TOUCHLESS_VERSION_PATCH 9)
# set(BUILD_NUMBER "-1" CACHE STRING "Build number")
#
# set(FILES_TO_TRANSLATE)

# Adds a precompiled header
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  if(MSVC)
    set_source_files_properties(${PrecompiledSource}
        PROPERTIES
        COMPILE_FLAGS "/Yc${PrecompiledHeader}"
        )
    foreach( src_file ${${SourcesVar}} )
        set_source_files_properties(
            ${src_file}
            PROPERTIES
            COMPILE_FLAGS "/Yu${PrecompiledHeader}"
            )
    endforeach( src_file ${${SourcesVar}} )
    list(APPEND ${SourcesVar} ${PrecompiledHeader} ${PrecompiledSource})
  endif(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)

# WARNING!!! Only change the minimum compatibility SDK version if there are
# breaking changes to the underlying Protocol Buffer structures or a structure
# is no longer transmitted to clients. A breaking change is when a type or
# index of an existing Protocol Buffer structures is modified (e.g., changing a
# type from double to float).
#
# set(LEAP_COMPATIBILITY_VERSION_MAJOR 0)
# set(LEAP_COMPATIBILITY_VERSION_MINOR 7)
# set(LEAP_COMPATIBILITY_VERSION_PATCH 0)
# # Minimum SDK compatibility build number
# set(LEAP_COMPATIBILITY_VERSION_BUILD_NUMBER 701)

project(Touchless)

list( APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}" "${CMAKE_SOURCE_DIR}/..")
include(LeapConfig)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib CACHE INTERNAL "Single output directory for building all static libraries.")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all dynamic libraries.")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin CACHE INTERNAL "Single output directory for building all executables.")
set(PROJECT_LIBRARY_PATH ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(PROJECT_EXECUTABLE_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})
set(PROJECT_TEST_PATH ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${CMAKE_CFG_INTDIR})

if(BUILD_LINUX)
  set(CMAKE_SKIP_BUILD_RPATH FALSE)
  set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
  set(CMAKE_INSTALL_RPATH "\${ORIGIN}:\${ORIGIN}/../lib/Leap")
  set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
endif()

#Setup Compiler Flags
# OPTION(USE_PRODUCTION_FLAGS "Enable compiler optimizations that increase build time" OFF)
# OPTION(USE_EXPERIMENTAL_MODELS "Enable kinematic model and various templates that double compile time on Windows" OFF)
# option(USE_CRASH_REPORTING "Build crash reporting support (Mac-only for now)" ON)
# if(BUILD_WINDOWS)
#   # Fix for Zm flag error in Visual Studio
#   option(USE_ZMPATCH "Fixes memory errors in windows builds using visual studio. Flagged by zm." ON)
# endif()
# set(USE_EXPERIMENTAL_MODELS_FLAG 0)
# if(USE_EXPERIMENTAL_MODELS)
#   set(USE_EXPERIMENTAL_MODELS_FLAG 1)
# endif()
# set(USE_CRASH_REPORTING_FLAG 0)
# if(USE_CRASH_REPORTING)
#   set(USE_CRASH_REPORTING_FLAG 1)
# endif()

set(ALTERNATE_LIBRARY "")
if(BUILD_MAC)
  option(USE_LIBCXX "Build with libc++ instead of libstdc++" OFF)
  if(USE_LIBCXX)
    set(ALTERNATE_LIBRARY "-libc++")
  endif()
endif()
if(MSVC90)
  set(ALTERNATE_LIBRARY "-vs2008")
endif()

set(BASE_MSVC_FLAGS /arch:SSE2)
# misnomer: The above MSVC flags are not used in Debug mode
set(PRODUCTION_MSVC_FLAGS /GL /Ox /Oi /Ot /Ob2 ) # TODO: remove whole-program optimization
set(BASE_GCC_FLAGS -std=c++0x -fpermissive -msse2 -Wall -Wno-reorder -Wno-strict-aliasing -Wno-missing-braces -Wno-uninitialized -Wno-sequence-point )
if(BUILD_64_BIT)
  set(BASE_GCC_FLAGS ${BASE_GCC_FLAGS} -fPIC )
else()
  set(BASE_GCC_FLAGS ${BASE_GCC_FLAGS} -m32 )
endif()
set(BASE_CLANG_FLAGS -std=c++11 -Wall -Wno-deprecated-declarations -Wno-address-of-temporary -Wno-dynamic-class-memaccess -Wno-overloaded-virtual -Wno-missing-braces -Wno-unneeded-internal-declaration -Wno-reorder )
set(PRODUCTION_GCC_FLAGS "")
if(BUILD_WINDOWS)
  if (USE_ZMPATCH)
    set(COMPILER_DEBUG_FLAGS /Zi /D_DEBUG /MDd /Od /fp:except /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS=1 /RTC1 /MP /Zm200)
    set(COMPILER_RELEASE_FLAGS /Zi /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /MD /O2 /fp:fast /WX /MP /Zm200)
    MESSAGE("ZM Patch Activated")
  else()
    set(COMPILER_DEBUG_FLAGS /Zi /D_DEBUG /MDd /Od /fp:except /D_CRT_SECURE_NO_WARNINGS /D_SCL_SECURE_NO_WARNINGS=1 /RTC1 /MP)
    set(COMPILER_RELEASE_FLAGS /Zi /D NDEBUG /D _CRT_SECURE_NO_WARNINGS /MD /O2 /fp:fast /WX /MP)
  endif()
  # USE_PRODUCTION_FLAGS will replace /Od or /O2 with /Ox /Ob2 /Oi /Ot, adds ~20% compile time
  set(COMPILER_NOPRODUCTION_FLAGS /Zi )
  set(BASE_LINKER_FLAGS /NODEFAULTLIB:LIBCMT /ignore:4099 /DEBUG )
  set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} /OPT:REF,ICF")
  set(NOPRODUCTION_LINKER_FLAGS "")
  set(PRODUCTION_LINKER_FLAGS /LTCG )
  add_definitions(-DNOMINMAX)
else() # clang or gcc
  set(COMPILER_DEBUG_FLAGS -g )
  set(COMPILER_RELEASE_FLAGS -O3 -g -DNDEBUG )
  set(COMPILER_NOPRODUCTION_FLAGS "")
  set(BASE_LINKER_FLAGS "")
  set(NOPRODUCTION_LINKER_FLAGS "")
  set(PRODUCTION_LINKER_FLAGS "")
  if(BUILD_MAC)
    if(USE_LIBCXX)
      set(COMPILER_DEBUG_FLAGS ${COMPILER_DEBUG_FLAGS} -mmacosx-version-min=10.7 -fvisibility=hidden -stdlib=libc++)
      set(COMPILER_RELEASE_FLAGS ${COMPILER_RELEASE_FLAGS} -mmacosx-version-min=10.7 -fvisibility=hidden -stdlib=libc++)
    else()
      set(COMPILER_DEBUG_FLAGS ${COMPILER_DEBUG_FLAGS} -mmacosx-version-min=10.4 -fvisibility=hidden)
      set(COMPILER_RELEASE_FLAGS ${COMPILER_RELEASE_FLAGS} -mmacosx-version-min=10.4 -fvisibility=hidden)
    endif()
    set(STRIPPED_EXE_LINKER_FLAGS "-Xlinker -unexported_symbol -Xlinker \"*\" -Xlinker -dead_strip -Xlinker -dead_strip_dylibs")
  else()
    if(USE_PRODUCTION_FLAGS)
      set(STRIPPED_EXE_LINKER_FLAGS "-Xlinker --no-export-dynamic")
      # gcc: Defer stripping (-s) until after saving breakpad symbols
    else()
      set(STRIPPED_EXE_LINKER_FLAGS " ")
    endif()
    set(COMPILER_DEBUG_FLAGS ${COMPILER_DEBUG_FLAGS} -fvisibility=hidden)
    set(COMPILER_RELEASE_FLAGS ${COMPILER_RELEASE_FLAGS} -fvisibility=hidden)
  endif()
endif()

if(BUILD_WINDOWS)
  set(COMPILER_FLAGS ${COMPILER_FLAGS} ${BASE_MSVC_FLAGS})
elseif(BUILD_MAC)
  set(COMPILER_FLAGS ${COMPILER_FLAGS} ${BASE_CLANG_FLAGS})
else() # Linux
  set(COMPILER_FLAGS ${COMPILER_FLAGS} ${BASE_GCC_FLAGS})
endif()

set(LEAP_OS_WINDOWS 0)
set(LEAP_OS_WINDOWS32 0)
set(LEAP_OS_WINDOWS64 0)
set(LEAP_OS_LINUX 0)
set(LEAP_OS_LINUX32 0)
set(LEAP_OS_LINUX64 0)
set(LEAP_OS_MAC 0)

if(BUILD_WINDOWS)
  set(LEAP_OS_WINDOWS 1)
  if(BUILD_64_BIT)
    set(LEAP_OS_WINDOWS64 1)
  else()
    set(LEAP_OS_WINDOWS32 1)
  endif()
elseif(BUILD_MAC)
    set(LEAP_OS_MAC 1)
else() # Linux
  set(LEAP_OS_LINUX 1)
  if(BUILD_64_BIT)
    set(LEAP_OS_LINUX64 1)
  else()
    set(LEAP_OS_LINUX32 1)
  endif()
endif()

if(USE_PRODUCTION_FLAGS)
  if(BUILD_WINDOWS)
    set(COMPILER_FLAGS ${COMPILER_FLAGS} ${PRODUCTION_MSVC_FLAGS})
  elseif(BUILD_MAC)
    set(COMPILER_FLAGS ${COMPILER_FLAGS} ${PRODUCTION_CLANG_FLAGS})
  else() # Linux
    set(COMPILER_FLAGS ${COMPILER_FLAGS} ${PRODUCTION_GCC_FLAGS})
  endif()
else()
  set(COMPILER_FLAGS ${COMPILER_FLAGS} ${COMPILER_NOPRODUCTION_FLAGS})
endif()

set(COMPILER_RELEASE_FLAGS ${COMPILER_RELEASE_FLAGS} ${COMPILER_FLAGS})
if(NOT BUILD_WINDOWS)
  # Windows-only: BASE_MSVC_FLAGS is a misnomer - don't include COMPILER_FLAGS for debug
  set(COMPILER_DEBUG_FLAGS ${COMPILER_DEBUG_FLAGS} ${COMPILER_FLAGS})
endif()

set(LINKER_FLAGS ${LINKER_FLAGS} ${BASE_LINKER_FLAGS})

if(USE_PRODUCTION_FLAGS)
  if(BUILD_WINDOWS)
    # Visual Studio: /LTCG clashes with /INCREMENTAL, so add it to Release only
    set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} ${PRODUCTION_LINKER_FLAGS}")
  else()
    set(LINKER_FLAGS ${LINKER_FLAGS} ${PRODUCTION_LINKER_FLAGS})
  endif()
else()
  set(LINKER_FLAGS ${LINKER_FLAGS} ${NOPRODUCTION_LINKER_FLAGS})
endif()

# TEMP_* stuff: We need to string-ify each list to avoid semicolons
foreach(arg ${COMPILER_RELEASE_FLAGS})
  set(TEMP_COMPILER_RELEASE_FLAG_LIST "${TEMP_COMPILER_RELEASE_FLAG_LIST} ${arg}")
endforeach(arg ${COMPILER_RELEASE_FLAGS})

foreach(arg ${COMPILER_DEBUG_FLAGS})
  set(TEMP_COMPILER_DEBUG_FLAG_LIST "${TEMP_COMPILER_DEBUG_FLAG_LIST} ${arg}")
endforeach(arg ${COMPILER_DEBUG_FLAGS})

if(USE_PRODUCTION_FLAGS)
  string(REGEX REPLACE "/O2" "" TEMP_COMPILER_RELEASE_FLAG_LIST ${TEMP_COMPILER_RELEASE_FLAG_LIST})

  # For production Leapd.dll (_HAS_ITERATOR_DEBUGGING), keep codegen settings close to Release
  string(REGEX REPLACE "/DEBUG" "" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  string(REGEX REPLACE "/D_DEBUG" "/D _HAS_ITERATOR_DEBUGGING=1" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  # Stick with /MDd to avoid confusing users, and /MD does not improve Leapd.dll's final size
  # string(REGEX REPLACE "/MDd" "/MD" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  # WG: optimization must be disabled & Zi set for breakpoints & code stepping to work in VS
  # string(REGEX REPLACE "/Od" "/Ox /Ob2 /Oi /Ot" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  # string(REGEX REPLACE "/Zi" "" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})

  string(REGEX REPLACE "/GS-" "/GS" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  string(REGEX REPLACE "/RTC1" "" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
  # Floating point exceptions probably make it easier for us to diagnose user issues, and tend to make things harder to reverse-engineer
  # string(REGEX REPLACE "/fp:except" "/fp:fast" TEMP_COMPILER_DEBUG_FLAG_LIST ${TEMP_COMPILER_DEBUG_FLAG_LIST})
endif()

foreach(arg ${LINKER_FLAGS})
  set(TEMP_LINKER_FLAG_LIST "${TEMP_LINKER_FLAG_LIST} ${arg}")
endforeach(arg ${LINKER_FLAGS})

set(CMAKE_CXX_FLAGS_DEBUG "${TEMP_COMPILER_DEBUG_FLAG_LIST}" CACHE STRING "compiler flags" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${TEMP_COMPILER_RELEASE_FLAG_LIST}" CACHE STRING "compiler flags" FORCE)
set(CMAKE_EXE_LINKER_FLAGS "${TEMP_LINKER_FLAG_LIST}" CACHE STRING "linker flags" FORCE)

set(STATIC_LIB_FLAGS "${TEMP_LINKER_FLAG_LIST}")
set(SHARED_LIB_FLAGS "${TEMP_LINKER_FLAG_LIST}")
if(BUILD_WINDOWS)
  string(REGEX REPLACE "/DEBUG" "" STATIC_LIB_FLAGS ${STATIC_LIB_FLAGS})
  if(USE_PRODUCTION_FLAGS)
    set(STATIC_LIB_FLAGS "${STATIC_LIB_FLAGS} ${PRODUCTION_LINKER_FLAGS}")
    # 1. SHARED_LIB_FLAGS: To avoid /LCTG + /INCREMENTAL incompatibility, distinguish
    #    LINK_FLAGS vs LINK_FLAGS_RELEASE in source/API/CMakeLists.txt
    # 2. For production Leapd.dll, remove dead code despite /DEBUG
    set(SHARED_LIB_FLAGS "${SHARED_LIB_FLAGS} /INCREMENTAL:NO /OPT:REF,ICF")
  endif()
endif()
include(CTest)
enable_testing()

#include(CPack)

#Setup Libraries

#Look for one of our external libraries in the PATH or EXTERNAL_LIBRARY_PATHS
find_path(EXTERNAL_LIBRARY_DIR "boost_1_51_0" PATHS $ENV{PATH} $ENV{EXTERNAL_LIBRARY_DIR} NO_DEFAULT_PATH)
if( EXTERNAL_LIBRARY_DIR STREQUAL "EXTERNAL_LIBRARY_DIR-NOTFOUND" )
  message(STATUS "External Library Directory not found, please specify a folder to look for external libraries")
  return()
endif()

set(Boost_DIR ${EXTERNAL_LIBRARY_DIR}/boost_1_51_0${ALTERNATE_LIBRARY} CACHE PATH "boost root path" FORCE)
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_INCLUDE_DIR ${Boost_DIR}/include CACHE PATH "boost include path" FORCE)
set(Boost_LIBRARY_DIRS ${Boost_DIR}/lib CACHE PATH "boost lib path" FORCE)
find_package(Boost REQUIRED)
link_directories(${Boost_LIBRARY_DIRS})
mark_as_advanced(Boost_DIR)

# if(BUILD_WINDOWS)
# add_library(pipemanlib STATIC IMPORTED)
# set_target_properties(pipemanlib PROPERTIES IMPORTED_LOCATION_DEBUG ${EXTERNAL_LIBRARY_DIR}/pipeman/lib/Debug/PipeManLib.lib IMPORTED_LOCATION_RELEASE ${EXTERNAL_LIBRARY_DIR}/pipeman/lib/Release/PipeManLib.lib)
# include_directories(${EXTERNAL_LIBRARY_DIR}/pipeman/include/)
# endif(BUILD_WINDOWS)

set(LUAJIT_DIR ${EXTERNAL_LIBRARY_DIR}/luajit)
set(LUAJIT_INCLUDE_DIR ${LUAJIT_DIR}/include)

if(BUILD_WINDOWS)
  if(BUILD_64_BIT)
    set(USE_OPENSUBDIV 0)
    set(LUAJIT_BIN_DIR ${LUAJIT_DIR}/bin/x64)
    set(LUAJIT_LIB_DIR ${LUAJIT_DIR}/lib/x64)
  else()
    set(USE_OPENSUBDIV 1)
    set(LUAJIT_BIN_DIR ${LUAJIT_DIR}/bin/x86)
    set(LUAJIT_LIB_DIR ${LUAJIT_DIR}/lib/x86)
  endif()
else()
  set(USE_OPENSUBDIV 1)
  set(LUAJIT_BIN_DIR ${LUAJIT_DIR}/bin/x64)

  if(BUILD_MAC)
    set(LUAJIT_LIB_DIR ${LUAJIT_DIR}/lib/uni)
  else() #linux
    set(LUAJIT_LIB_DIR ${LUAJIT_DIR}/lib/x64)
  endif()
endif()

set(LUAJIT_EXE ${LUAJIT_BIN_DIR}/luajit${EXE_SUFFIX})
set(LUAJIT_LIB ${LUAJIT_LIB_DIR}/${LIBRARY_PREFIX}luajit${STATIC_LIBRARY_SUFFIX})

# option(BUILD_DEMO_APPS "Build demo applications in the main solution" OFF)
# option(BUILD_UTIL_APPS "Build utility applications in the main solution" OFF)
# option(BUILD_FIRMWARE_UPDATER "Build the firmware update tool (LeapFwPatch)" OFF)

# if(BUILD_UTIL_APPS)
# if(BUILD_WINDOWS)
#   if(BUILD_64_BIT)
#     find_package(OpenCV PATHS ${EXTERNAL_LIBRARY_DIR}/OpenCV/Build64 REQUIRED)
#   else()
#     find_package(OpenCV PATHS ${EXTERNAL_LIBRARY_DIR}/OpenCV/Build32 REQUIRED)
#   endif()
#   set(OpenCV_BIN_DIR ${OpenCV_DIR}/bin)
#   set(OpenCV_SRC_DIR ${OpenCV_DIR}/..)
# else()
#   find_package(OpenCV PATHS ${EXTERNAL_LIBRARY_DIR}/OpenCV/share/OpenCV REQUIRED)
# endif()
# On Mac, OpenCV_LIB_DIR is two folders including lib and lib/..
#
# if(BUILD_WINDOWS)
#   # we rely on a special in-source build of OpenCV where DLLs are in the bin dir
#   install(DIRECTORY ${OpenCV_BIN_DIR}/ DESTINATION ${PROJECT_BINARY_DIR}/bin FILES_MATCHING PATTERN "*${SHARED_LIBRARY_SUFFIX}")
# else()
#   # but on Linux the .so or .dylib files are in the lib dir...
#   install(DIRECTORY ${OpenCV_DIR}/lib/ DESTINATION ${PROJECT_BINARY_DIR}/bin FILES_MATCHING PATTERN "*${SHARED_LIBRARY_SUFFIX}")
# endif()
# endif(BUILD_UTIL_APPS)

macro(target_strip Target)
  # primary function of this macro: strip symbols at link time on Mac and Linux
  # secondary function: avoid MSVC warnings in Debug by specifying /nodefaultlib
  # tertiary function: avoid MSVC warnings about combining /incremental with /ltcg
  if(BUILD_WINDOWS)
    if(USE_PRODUCTION_FLAGS)
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/INCREMENTAL:NO /NODEFAULTLIB:MSVCRT")
    else()
      set_target_properties(${Target} PROPERTIES LINK_FLAGS_DEBUG "/NODEFAULTLIB:MSVCRT")
    endif()
  else()
    set_target_properties(${Target} PROPERTIES LINK_FLAGS_RELEASE ${STRIPPED_EXE_LINKER_FLAGS})
  endif()
endmacro()

macro(add_target_file SourceDir FileName)
  # For simple destination files like app0.dat
  if(BUILD_WINDOWS)
    configure_file(${SourceDir}/${FileName}
                   ${PROJECT_BINARY_DIR}/bin/Release/${FileName}
                   COPYONLY)
    configure_file(${SourceDir}/${FileName}
                   ${PROJECT_BINARY_DIR}/bin/Debug/${FileName}
                   COPYONLY)
  else()
    # Mac or Linux
    configure_file(${SourceDir}/${FileName}
                   ${PROJECT_BINARY_DIR}/bin/${FileName}
                   COPYONLY)
  endif()
endmacro()

macro(add_shared_library SourceDir LibName LinuxVersionSuffix)
  # For the case of a single Windows/Mac/Linux shared lib which doesn't distinguish Debug vs Release
  if(BUILD_WINDOWS)
    configure_file(${SourceDir}/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}
                   ${PROJECT_BINARY_DIR}/bin/Release/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}
                   COPYONLY)
    configure_file(${SourceDir}/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}
                   ${PROJECT_BINARY_DIR}/bin/Debug/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}
                   COPYONLY)
  else()
    # Mac or Linux
    if(BUILD_LINUX)
      set(_LIBRARY_VERSION_SUFFIX ${LinuxVersionSuffix})
    endif()
    configure_file(${SourceDir}/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}
                   ${PROJECT_BINARY_DIR}/bin/${LIBRARY_PREFIX}${LibName}${SHARED_LIBRARY_SUFFIX}${_LIBRARY_VERSION_SUFFIX}
                   COPYONLY)
  endif()
endmacro()

option(USE_QT "USE QT" ON )
# option(USE_CG "USE CG" OFF)

set(USE_QT_FLAG 0)
if(USE_QT)
set(USE_QT_FLAG 1)
if(BUILD_WINDOWS)
  set(QT_RELEASE_SUFFIX "4")
  set(QT_DEBUG_SUFFIX "d4")
  set(QT_LINK_LIBRARY_SUFFIX ${STATIC_LIBRARY_SUFFIX})
else()
  set(QT_RELEASE_SUFFIX "")
  set(QT_DEBUG_SUFFIX "")
  set(QT_LINK_LIBRARY_SUFFIX ${SHARED_LIBRARY_SUFFIX})
endif()

if(NOT BUILD_MAC)
  set(QT_VERSION_SUFFIX "")
  if(BUILD_WINDOWS)
    set(QT_LIBRARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin/Release)
  else()
    # Linux
    set(QT_LIBRARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin)
    set(QT_VERSION_SUFFIX ".4")
    # configure_file follows hyperlinks before the copy and sets chmod +x
    # so no need to copy "4.8.4" suffixed .so files
  endif()
endif()

set(QT_QMAKE_EXECUTABLE ${EXTERNAL_LIBRARY_DIR}/qt-4.8.4/bin/qmake${EXE_SUFFIX} CACHE FILEPATH "qmake.exe path")
if(DEFINED QT_QMAKE_EXECUTABLE)
  string(REGEX REPLACE "[\\\\/]bin[\\\\/]qmake${EXE_SUFFIX}" "" QT_DIR "${QT_QMAKE_EXECUTABLE}")
  if("${QT_DIR}" STREQUAL "${QT_QMAKE_EXECUTABLE}")
    message("Unable to parse Qt path from ${QT_QMAKE_EXECUTABLE}")
  endif()

  set(QT_HEADERS_DIR ${QT_DIR}/include)
  set(QT_BINARY_DIR ${QT_DIR}/bin CACHE FILEPATH "" FORCE)
  set(QT_MOC_EXECUTABLE ${QT_DIR}/bin/moc${EXE_SUFFIX} CACHE FILEPATH "" FORCE)
  set(QT_RCC_EXECUTABLE ${QT_DIR}/bin/rcc${EXE_SUFFIX} CACHE FILEPATH "" FORCE)
  set(QT_UIC_EXECUTABLE ${QT_DIR}/bin/uic${EXE_SUFFIX} CACHE FILEPATH "" FORCE)
endif()

macro(add_qt_library LibName UppercaseLibName)
  if(DEFINED QT_QMAKE_EXECUTABLE)
    set(QT_QT${UppercaseLibName}_INCLUDE_DIR ${QT_DIR}/include/Qt${LibName} CACHE PATH "" FORCE)
    set(QT_QT${UppercaseLibName}_LIBRARY optimized;${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_RELEASE_SUFFIX}${QT_LINK_LIBRARY_SUFFIX};debug;${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_DEBUG_SUFFIX}${QT_LINK_LIBRARY_SUFFIX} CACHE string "" FORCE)
    set(QT_QT${UppercaseLibName}_LIBRARY_DEBUG ${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_DEBUG_SUFFIX}${QT_LINK_LIBRARY_SUFFIX} CACHE FILEPATH "" FORCE)
    set(QT_QT${UppercaseLibName}_LIBRARY_RELEASE ${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_RELEASE_SUFFIX}${QT_LINK_LIBRARY_SUFFIX} CACHE FILEPATH "" FORCE)
  endif()

  if(NOT BUILD_MAC)
    # Copy to bin/Release (on Windows) or wherever ${QT_LIBRARY_OUTPUT_DIR} dictates (other OSes)
    configure_file(${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_RELEASE_SUFFIX}${SHARED_LIBRARY_SUFFIX}
                   ${QT_LIBRARY_OUTPUT_DIR}/${LIBRARY_PREFIX}Qt${LibName}${QT_RELEASE_SUFFIX}${SHARED_LIBRARY_SUFFIX}${QT_VERSION_SUFFIX}
                   COPYONLY)
    if(BUILD_WINDOWS)
      # Copy to bin/Debug
      configure_file(${QT_DIR}/lib/${LIBRARY_PREFIX}Qt${LibName}${QT_DEBUG_SUFFIX}${SHARED_LIBRARY_SUFFIX}
                     ${PROJECT_BINARY_DIR}/bin/Debug/${LIBRARY_PREFIX}Qt${LibName}${QT_DEBUG_SUFFIX}${SHARED_LIBRARY_SUFFIX}${QT_VERSION_SUFFIX}
                     COPYONLY)
    endif()
  endif()
endmacro()

add_qt_library(Core CORE)
add_qt_library(Gui GUI)
add_qt_library(Script SCRIPT)
add_qt_library(Network NETWORK)
add_qt_library(WebKit WEBKIT)
add_qt_library(XmlPatterns XMLPATTERNS)
add_qt_library(OpenGL OPENGL)
if(BUILD_LINUX)
  add_qt_library(DBus DBUS)
endif()

if(DEFINED QT_QMAKE_EXECUTABLE)
  find_package(Qt4 REQUIRED)
  set(Leap_DIR ${EXTERNAL_LIBRARY_DIR}/LeapSDK/ CACHE PATH "LeapSDK root path" FORCE)
  find_package(Leap REQUIRED)
endif()

endif(USE_QT)

# option(BUILD_PROCESS_PIPELINE "Build the command-line tool (ProcessPipeline) that predates the LeapControlPanel GUI" ON)

# set(CROSSROADS_DIR ${EXTERNAL_LIBRARY_DIR}/libxs-1.2.0${ALTERNATE_LIBRARY})
set(EIGEN_ROOT_DIR ${EXTERNAL_LIBRARY_DIR}/eigen-3.1.2)
# set(FREEGLUT_DIR ${EXTERNAL_LIBRARY_DIR}/freeglut-2.8.0)
# set(ZLIB_DIR ${EXTERNAL_LIBRARY_DIR}/zlib-1.2.8)
# set(BREAKPAD_DIR ${EXTERNAL_LIBRARY_DIR}/breakpad-0.1)

if(BUILD_MAC)
  find_library(OPENGL_FRAMEWORK OpenGL)
  find_library(GLUT_FRAMEWORK GLUT)
  find_library(PYTHON_FRAMEWORK python)
  find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
  find_library(QUARTZ_CORE_FRAMEWORK QuartzCore)
  find_library(CORE_SERVICES_FRAMEWORK CoreServices)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  find_library(APPKIT_FRAMEWORK AppKit)
  find_library(FOUNDATION_FRAMEWORK Foundation)
  find_library(PREFERENCE_PANES_FRAMEWORK PreferencePanes)
  find_library(COCOA_FRAMEWORK Cocoa)
  find_library(CARBON_FRAMEWORK Carbon)
  find_library(IOKIT_FRAMEWORK IOKit)
  mark_as_advanced(OPENGL_FRAMEWORK GLUT_FRAMEWORK PYTHON_FRAMEWORK CORE_FOUNDATION_FRAMEWORK QUARTZ_CORE_FRAMEWORK CORE_SERVICES_FRAMEWORK APPKIT_FRAMEWORK FOUNDATION_FRAMEWORK PREFERENCE_PANES_FRAMEWORK COCOA_FRAMEWORK CARBON_FRAMEWORK IOKIT_FRAMEWORK)
  # set(TBB_DIR ${EXTERNAL_LIBRARY_DIR}/tbb/4.0u4/bin)
else() # Windows or Linux
  # set(TBB_DIR ${EXTERNAL_LIBRARY_DIR}/tbb40_20111003oss/bin)
endif()
# if(BUILD_WINDOWS)
#   set(CYAPI_DIR ${EXTERNAL_LIBRARY_DIR}/cyapi-1.2)
# else() # Mac or Linux
#   set(LIBUSB_DIR ${EXTERNAL_LIBRARY_DIR}/libusb)
#   set(LIBUVC_DIR ${EXTERNAL_LIBRARY_DIR}/libuvc)
# endif()
set(DRV_INT_DIR ${EXTERNAL_LIBRARY_DIR}/ocu-drv-int-1)
# set(CURL_DIR ${EXTERNAL_LIBRARY_DIR}/curl-7.32.0)
# if(NOT USE_PUBLIC_SETTINGS)
#   set(ANTTWEAKBAR_DIR ${EXTERNAL_LIBRARY_DIR}/AntTweakBar)
# endif()
# set(PROTOBUF_DIR ${EXTERNAL_LIBRARY_DIR}/protobuf-2.5.0${ALTERNATE_LIBRARY})
# set(OPENSSL_DIR ${EXTERNAL_LIBRARY_DIR}/openssl)
set(WEBSOCKETPP_DIR ${EXTERNAL_LIBRARY_DIR}/websocketpp)
# if(BUILD_MAC)
#   set(PYTHON_DIR /Library/Frameworks/Python.framework/Versions/2.7)
# else()
#   set(PYTHON_DIR ${EXTERNAL_LIBRARY_DIR}/python2.7)
# endif()
# set(JDK_DIR ${EXTERNAL_LIBRARY_DIR}/jdk)
# if(USE_CG)
# set(GLEW_DIR ${EXTERNAL_LIBRARY_DIR}/glew-1.9.0)
# set(CG_DIR ${EXTERNAL_LIBRARY_DIR}/cg-3.1)
# if(USE_OPENSUBDIV)
# set(OPENSUBDIV_DIR ${EXTERNAL_LIBRARY_DIR}/opensubdiv-0.1)
# endif(USE_OPENSUBDIV)
# endif(USE_CG)

# if(BUILD_MAC)
#   add_shared_library(${LIBUSB_DIR}/lib usb-1.0.0 "")
# elseif(BUILD_LINUX)
#   # special renaming
#   configure_file(${LIBUSB_DIR}/lib/libusb-1.0.0${SHARED_LIBRARY_SUFFIX}
#                  ${PROJECT_BINARY_DIR}/bin/libusb-1.0${SHARED_LIBRARY_SUFFIX}.0
#                  COPYONLY)
# endif()

# if(NOT USE_PUBLIC_SETTINGS)
#   if(BUILD_WINDOWS AND BUILD_64_BIT)
#     add_shared_library(${ANTTWEAKBAR_DIR}/lib AntTweakBar64 .1)
#   else()
#     add_shared_library(${ANTTWEAKBAR_DIR}/lib AntTweakBar .1)
#   endif()
# endif()

# if(USE_CRASH_REPORTING AND BUILD_MAC)
#   file(REMOVE_RECURSE ${PROJECT_BINARY_DIR}/Frameworks/Breakpad.framework)
#   file(COPY ${BREAKPAD_DIR}/lib/Breakpad.framework DESTINATION ${PROJECT_BINARY_DIR}/Frameworks)
# endif()

include_directories(
  ${PROJECT_BINARY_DIR}
  ${PROJECT_SOURCE_DIR}
  ${PROJECT_SOURCE_DIR}/source
  ${PROJECT_LIBRARY_PATH}
  ${PROJECT_EXECUTABLE_PATH}
  ${PROJECT_TEST_PATH}
  # ${CROSSROADS_DIR}/include
  ${Boost_INCLUDE_DIR}
  ${DRV_INT_DIR}/include
  # ${CURL_DIR}/include
  # ${PROTOBUF_DIR}/include
  # ${OPENSSL_DIR}/include
  ${WEBSOCKETPP_DIR}/include
  # ${JDK_DIR}/include
  ${EIGEN_ROOT_DIR}
  # ${FREEGLUT_DIR}/include/
  )

# if(NOT USE_PUBLIC_SETTINGS)
#   include_directories( ${ANTTWEAKBAR_DIR}/include )
# endif()

# if(BUILD_WINDOWS)
#   # legacy: on Windows, should change zlib into an "install"
#   include_directories(${ZLIB_DIR})
# else()
#   include_directories(${ZLIB_DIR}/include)
# endif()

# Python and Java unfortunately have very OS-specific directory structures
# if(BUILD_WINDOWS)
#   # Windows sample directory structure: C:\Python27\include, C:\Python27\libs
#   include_directories(${PYTHON_DIR}/include)
# else()
#   # Mac or Linux
#   include_directories(${PYTHON_DIR}/include/python2.7)
# endif()

# if(BUILD_WINDOWS)
#   include_directories(${JDK_DIR}/include/win32)
# elseif(NOT BUILD_MAC)
#   include_directories(${JDK_DIR}/include/linux)
# endif()

# if(BUILD_WINDOWS)
#   include_directories(${CYAPI_DIR}/inc
#                       )
# else()
#   include_directories(${LIBUSB_DIR}/include
#                       ${LIBUVC_DIR}/include
#                       )
# endif()

# if(USE_CG)
# if(USE_OPENSUBDIV)
# include_directories(
#   ${GLEW_DIR}/include
#   ${CG_DIR}/include
#   ${OPENSUBDIV_DIR}/include
#   )
# else(USE_OPENSUBDIV)
# include_directories(
#   ${GLEW_DIR}/include
#   ${CG_DIR}/include
#   )
# endif(USE_OPENSUBDIV)
# endif(USE_CG)

# if(USE_CG)
# set(HAVE_CG_FLAG 1)
# install(FILES
#   ${CG_DIR}/bin/cg.dll
#   ${CG_DIR}/bin/cgGL.dll
#   DESTINATION ${PROJECT_BINARY_DIR}/bin/Debug
# )
# install(FILES
#   ${CG_DIR}/bin/cg.dll
#   ${CG_DIR}/bin/cgGL.dll
#   DESTINATION ${PROJECT_BINARY_DIR}/bin/Release
# )
# else(USE_CG)
# set(HAVE_CG_FLAG 0)
# endif(USE_CG)

configure_file(.editorconfig ${CMAKE_CURRENT_BINARY_DIR}/.editorconfig COPYONLY)

# option(USE_PANA_CAMERA "USE PANASONIC CAMERA" OFF)
# if(USE_PANA_CAMERA)
#   set(USE_PANA_FLAG 1)
#   if(BUILD_UTIL_APPS)
#     include_directories(
#       ${OpenCV_SRC_DIR}/modules/highgui_pana/include
#       ${OpenCV_SRC_DIR}/modules/highgui_pana/common/highgui_pana
#     )
#   endif(BUILD_UTIL_APPS)
# else(USE_PANA_CAMERA)
#   set(USE_PANA_FLAG 0)
# endif(USE_PANA_CAMERA)

# option(USE_PUBLIC_SETTINGS "Enable encryption, time-bomb countdown, and disable various visualizer options" ON)
# if(USE_PUBLIC_SETTINGS)
#   set(USE_PUBLIC_FLAG 1)
#   set(BUILD_VIS_SANDBOX 0)
# else(USE_PUBLIC_SETTINGS)
#   set(USE_PUBLIC_FLAG 0)
#   option(BUILD_VIS_SANDBOX "Build VisSandBox application in the main solution" OFF)
# endif(USE_PUBLIC_SETTINGS)

if(BUILD_WINDOWS)
  option(UNSIGNED_BINARIES_EXPECTED "Binaries being built are not expected to be digitally signed" OFF)
endif()


add_subdirectory(source)
add_subdirectory(application)

