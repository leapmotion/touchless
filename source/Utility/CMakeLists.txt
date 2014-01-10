include_directories(
${LEAP_INCLUDE_DIR}
)

SET(Utility_SRCS
  FileSystemUtil.h
  FileSystemUtil.cpp
  Heartbeat.h
  Heartbeat.cpp
  StateMachine.h
)

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" Utility_SRCS)

if(NOT BUILD_WINDOWS)
  set(Boost_DATE_TIME_LIBRARY libboost_date_time)
  set(Boost_CHRONO_LIBRARY libboost_chrono)
  set(Boost_SYSTEM_LIBRARY libboost_system)
  set(Boost_FILESYSTEM_LIBRARY libboost_filesystem)
  add_library(${Boost_DATE_TIME_LIBRARY} STATIC IMPORTED)
  add_library(${Boost_CHRONO_LIBRARY} STATIC IMPORTED)
  add_library(${Boost_SYSTEM_LIBRARY} STATIC IMPORTED)
  add_library(${Boost_FILESYSTEM_LIBRARY} STATIC IMPORTED)
  set_property(TARGET ${Boost_DATE_TIME_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_DATE_TIME_LIBRARY}${STATIC_LIBRARY_SUFFIX})
  set_property(TARGET ${Boost_CHRONO_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_CHRONO_LIBRARY}${STATIC_LIBRARY_SUFFIX})
  set_property(TARGET ${Boost_SYSTEM_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_SYSTEM_LIBRARY}${STATIC_LIBRARY_SUFFIX})
  set_property(TARGET ${Boost_FILESYSTEM_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_FILESYSTEM_LIBRARY}${STATIC_LIBRARY_SUFFIX})
endif()

add_library(Utility ${Project_Libary_Mode} ${Utility_SRCS})
set_target_properties(Utility PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(Utility PROPERTIES STATIC_LIBRARY_FLAGS "${STATIC_LIB_FLAGS}")

target_link_libraries(Utility DataStructures)
target_link_libraries(Utility Configuration)
if(BUILD_WINDOWS)
  target_link_libraries(Utility psapi)
else()
  target_link_libraries(Utility ${Boost_DATE_TIME_LIBRARY} ${Boost_CHRONO_LIBRARY} ${Boost_SYSTEM_LIBRARY} ${Boost_FILESYSTEM_LIBRARY})
  if(BUILD_MAC)
    include_directories(${CMAKE_OSX_SYSROOT}/${APPKIT_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${FOUNDATION_FRAMEWORK}/Headers)
    set_property(SOURCE CurrentApplication.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
    target_link_libraries(Utility ${CORE_SERVICES_FRAMEWORK} ${APPKIT_FRAMEWORK} ${IOKIT_FRAMEWORK})
  else()
    include(FindPkgConfig)
    pkg_check_modules(DBUS REQUIRED dbus-1)
    include_directories(${DBUS_INCLUDE_DIRS})
    link_directories(${DBUS_LIBRARY_DIRS})
    target_link_libraries(Utility -lrt -ldl ${DBUS_LIBRARIES})
  endif()
endif()
