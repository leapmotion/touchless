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
  	LPIconMac.h
  	LPIconMac.cpp
  	LPImageMac.h
  	LPImageMac.cpp
    ${OVERLAY_SRCS}
  )
  # set_property(SOURCE LPIcon.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  # set_property(SOURCE LPImage.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  set_property(SOURCE LPImageMac.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  include_directories(${CMAKE_OSX_SYSROOT}/${APPKIT_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${FOUNDATION_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${QUARTZ_CORE_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${CARBON_FRAMEWORK}/Frameworks/HIToolbox.framework/Headers)
elseif(BUILD_WINDOWS)
  SET(OVERLAY_SRCS
    LPIconWin.h
    LPIconWin.cpp
    LPIconWindowClass.h
    LPIconWindowClass.cpp
    LPImageWin.h
    LPImageWin.cpp
    ${OVERLAY_SRCS}
  )
elseif(BUILD_LINUX)
  SET(OVERLAY_SRCS
    LPIconLinux.h
    LPIconLinux.cpp
    LPImageLinux.h
    LPImageLinux.cpp
    ${OVERLAY_SRCS}
  )
endif()

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" OVERLAY_SRCS)

add_library(Overlay STATIC ${OVERLAY_SRCS})
set_target_properties(Overlay PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(Overlay PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")
