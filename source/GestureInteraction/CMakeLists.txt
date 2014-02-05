include_directories(
  ${LEAP_INCLUDE_DIR}
  ../Configuration/
  ../DataStructures/
  ../Networking/
  ../Utility/
  ../API/
  ../Overlay/
  ../OSInteraction/
  ..
)

SET(GESTURE_INTERACTION_API_SRCS
  OutputPeripheralBasic.cpp
  OutputPeripheralBasic.h
  OutputPeripheralFingerMouse.cpp
  OutputPeripheralFingerMouse.h
  OutputPeripheralGestureOnly.cpp
  OutputPeripheralGestureOnly.h
  GestureInteractionManager.cpp
  GestureInteractionManager.h
)

if(BUILD_WINDOWS)
  SET(GESTURE_INTERACTION_API_SRCS
    GestureInteractionManagerWin.cpp
    ${GESTURE_INTERACTION_API_SRCS}
  )
elseif(BUILD_MAC)
  SET(GESTURE_INTERACTION_API_SRCS
    GestureInteractionManagerMac.cpp
    ${GESTURE_INTERACTION_API_SRCS}
  )
elseif(BUILD_LINUX)
SET(GESTURE_INTERACTION_API_SRCS
  GestureInteractionManagerLinux.cpp
  ${GESTURE_INTERACTION_API_SRCS}
)
endif()

# Internal static library for now
add_library(GestureInteraction STATIC ${GESTURE_INTERACTION_API_SRCS})
set_target_properties(GestureInteraction PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(GestureInteraction PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")


