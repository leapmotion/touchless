include_directories(
  ${LEAP_INCLUDE_DIR}
  ../Utility
  ../DataStructures
  ../Peripherals
  ..
)

SET(OS_INTERACTION_SRCS
  OSInteraction.h
  OSInteraction.cpp
  Touch.h
  TouchManager.cpp
  TouchManager.h
)

if(BUILD_WINDOWS)
  SET(OS_INTERACTION_SRCS
  GdiPlusInitializer.h
  GdiPlusInitializer.cpp
  TouchManagerWin7Driver.cpp
  TouchManagerWin7Driver.h
  TouchManagerWin7Undocumented.cpp
  TouchManagerWin7Undocumented.h
  TouchManagerWin8.cpp
  TouchManagerWin8.h
  TouchPeripheral.cpp
  TouchPeripheral.h
  Win8TouchStructures.h
  ${OS_INTERACTION_SRCS}
)
endif()

add_library(OSInteraction STATIC ${OS_INTERACTION_SRCS})
set_target_properties(OSInteraction PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(OSInteraction PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")
