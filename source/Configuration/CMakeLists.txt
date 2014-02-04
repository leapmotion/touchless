include_directories(
${LEAP_INCLUDE_DIR}
../GestureInteraction/
../OSInteraction/
../Overlay/
../Utility/
../DataStructures/
)

set(
  Configuration_SRCS
  Config.h
  Config.cpp
)

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" Configuration_SRCS)
add_library(Configuration STATIC ${Configuration_SRCS})
target_link_libraries(Configuration Utility)

if(NOT BUILD_WINDOWS)
  set(Boost_SYSTEM_LIBRARY libboost_system)
  add_library(${Boost_SYSTEM_LIBRARY} STATIC IMPORTED)
  set_property(TARGET ${Boost_SYSTEM_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_SYSTEM_LIBRARY}${STATIC_LIBRARY_SUFFIX})

  set(Boost_THREAD_LIBRARY libboost_thread)
  add_library(${Boost_THREAD_LIBRARY} STATIC IMPORTED)
  set_property(TARGET ${Boost_THREAD_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_THREAD_LIBRARY}${STATIC_LIBRARY_SUFFIX})

  set(Boost_CHRONO_LIBRARY libboost_chrono)
  add_library(${Boost_CHRONO_LIBRARY} STATIC IMPORTED)
  set_property(TARGET ${Boost_CHRONO_LIBRARY} PROPERTY IMPORTED_LOCATION ${Boost_LIBRARY_DIRS}/${Boost_CHRONO_LIBRARY}${STATIC_LIBRARY_SUFFIX})

  if(NOT BUILD_MAC)
    target_link_libraries(Configuration -lrt)
  endif()
  target_link_libraries(Configuration -lpthread ${Boost_SYSTEM_LIBRARY} ${Boost_THREAD_LIBRARY} ${Boost_CHRONO_LIBRARY})
endif()