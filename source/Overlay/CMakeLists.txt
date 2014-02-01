include_directories(
  ${LEAP_INCLUDE_DIR}
  ../Utility
  ../DataStructures
  ..
)

SET(OVERLAY_SRCS
  Overlay.h
  Overlay.cpp
)

add_library(Overlay STATIC ${OVERLAY_SRCS})
set_target_properties(Overlay PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(Overlay PROPERTIES LINK_FLAGS "${STATIC_LIB_FLAGS}")
