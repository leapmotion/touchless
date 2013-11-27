include_directories(
  ${LEAP_INCLUDE_DIR}
  ../Configuration/
  ../DataStructures/
  ../Networking/
  ../Peripherals/
  ../Utility/
  ..
)

SET(LEAP_PLUGIN_PLUS_API_SRCS
  LeapPlugin.h
  LeapPluginPlus.h
  OutputPeripheralImplementation.h
  OutputPeripheral.cpp
  OutputPeripheralBasic.h
  OutputPeripheralBasic.cpp
  OutputPeripheralFingerMouse.h
  OutputPeripheralFingerMouse.cpp
  OutputPeripheralGestureOnly.h
  OutputPeripheralGestureOnly.cpp
  OutputPeripheralMode.h
  OutputPeripheralMode.cpp
  TouchPeripheral.h
  TouchPeripheral.cpp
  PluginImplementation.h
  PluginPlus.cpp
)

# Internal static library for now
add_library(LeapPluginPlus STATIC ${LEAP_PLUGIN_PLUS_API_SRCS})
set_target_properties(LeapPluginPlus PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS} -DLEAP_API_IMPLEMENTATION=1")
set_target_properties(LeapPluginPlus PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")
