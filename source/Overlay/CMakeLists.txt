include_directories(
  ${LEAP_INCLUDE_DIR}
  ../Utility
  ../DataStructures
  ..
)

SET(OVERLAY_SRCS
  Overlay.h
  Overlay.cpp
  LPIcon.h
  LPIcon.cpp
  LPImage.h
  LPImage.cpp
)

if(BUILD_MAC)
  SET(OVERLAY_SRCS
    LPOverlay.h
    LPOverlay.mm
    ${OVERLAY_SRCS}
  )
  set_property(SOURCE LPIcon.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  set_property(SOURCE LPImage.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  include_directories(${CMAKE_OSX_SYSROOT}/${APPKIT_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${FOUNDATION_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${QUARTZ_CORE_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${CARBON_FRAMEWORK}/Frameworks/HIToolbox.framework/Headers)
endif()

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" OVERLAY_SRCS)

add_library(Overlay STATIC ${OVERLAY_SRCS})
set_target_properties(Overlay PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(Overlay PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")
