include_directories(
${LEAP_INCLUDE_DIR}
)


SET(FilterMethods_SRCS
  CategoricalFilter.h
  FilterBase.h
  RollingMean.h

  # These allow compilation on Mac, where the absence of source files would
  # otherwise preclude the generation of a library
  stdafx.h
  stdafx.cpp
)

## The following is used ONLY when compiled (.cpp) files are included
add_library(FilterMethods ${Project_Libary_Mode} ${FilterMethods_SRCS})
set_target_properties(FilterMethods PROPERTIES COMPILE_FLAGS "${ADDITIONAL_COMPILER_FLAGS}")
set_target_properties(FilterMethods PROPERTIES STATIC_LIBRARY_FLAGS "${STATIC_LIB_FLAGS}")
target_link_libraries(FilterMethods Utility DataStructures)

if(NOT BUILD_WINDOWS AND NOT BUILD_MAC)
  target_link_Libraries(FilterMethods -lrt)
endif()
