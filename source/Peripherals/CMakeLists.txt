#
#   Create a library here
#

include_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ../DataStructures/
  ../Utility/
  ${LEAP_INCLUDE_DIR}
  )


SET(Peripherals_SRCS
  LPGeometry.h
  LPGesture.h
  LPGesture.cpp
  LPIcon.h
  LPIcon.cpp
  LPImage.h
  LPImage.cpp
  LPScreen.h
  LPScreen.cpp
  LPVirtualScreen.h
  LPVirtualScreen.cpp
  Touch.h
  TouchManager.h
  TouchManager.cpp
)

if(BUILD_MAC)
  SET(Peripherals_SRCS
    LPMac.h
    LPMac.mm
    LPOverlay.h
    LPOverlay.mm
    ${Peripherals_SRCS}
  )
  set_property(SOURCE LPIcon.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  set_property(SOURCE LPImage.cpp PROPERTY COMPILE_FLAGS "-x objective-c++")
  include_directories(${CMAKE_OSX_SYSROOT}/${APPKIT_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${FOUNDATION_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${QUARTZ_CORE_FRAMEWORK}/Headers ${CMAKE_OSX_SYSROOT}/${CARBON_FRAMEWORK}/Frameworks/HIToolbox.framework/Headers)
elseif(BUILD_LINUX)
  SET(Peripherals_SRCS
    LPLinux.h
    LPLinux.cpp
    ${Peripherals_SRCS}
  )
endif()

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" Peripherals_SRCS)

add_library(Peripherals ${Project_Libary_Mode} ${Peripherals_SRCS})
set_target_properties(Peripherals PROPERTIES STATIC_LIBRARY_FLAGS "${STATIC_LIB_FLAGS}")

if(BUILD_WINDOWS)
  add_library(ocu-drv-int_debug STATIC IMPORTED)
  add_library(ocu-drv-int_release STATIC IMPORTED)
  add_library(hidlib STATIC IMPORTED)
  if(BUILD_64_BIT)
    set_property(TARGET ocu-drv-int_debug PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/x64/OcuInterfaced.lib)
    set_property(TARGET ocu-drv-int_release PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/x64/OcuInterface.lib)
    set_property(TARGET hidlib PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/x64/hid.lib)
  else(BUILD_64_BIT)
    set_property(TARGET ocu-drv-int_debug PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/win32/OcuInterfaced.lib)
    set_property(TARGET ocu-drv-int_release PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/win32/OcuInterface.lib)
    set_property(TARGET hidlib PROPERTY IMPORTED_LOCATION ${DRV_INT_DIR}/lib/win32/hid.lib)
  endif(BUILD_64_BIT)
  target_link_libraries(Peripherals GdiPlus.lib SetupAPI.Lib ShLwApi.Lib Psapi.Lib hidlib)
  target_link_libraries(Peripherals debug ocu-drv-int_debug)
  target_link_libraries(Peripherals optimized ocu-drv-int_release)
elseif(BUILD_MAC)
  target_link_libraries(Peripherals ${APPKIT_FRAMEWORK} ${FOUNDATION_FRAMEWORK} ${QUARTZ_CORE_FRAMEWORK} ${OPENGL_FRAMEWORK} ${CARBON_FRAMEWORK})
elseif(BUILD_LINUX)
  target_link_libraries(Peripherals Utility)
endif()
