include_directories(
${LEAP_INCLUDE_DIR}
)

SET(DataStructures_SRCS
  AxisAlignedBox.h
  BoundedQueue.h
  SSEfoo.h
  TimedHistory.h
  Value.h
  Value.cpp
)

ADD_MSVC_PRECOMPILED_HEADER("stdafx.h" "stdafx.cpp" DataStructures_SRCS)


if(BUILD_WINDOWS)
  if(BUILD_64_BIT)
    add_library(DataStructures ${Project_Libary_Mode} ${DataStructures_SRCS})
    target_link_libraries(DataStructures)
  else()
    add_library(DataStructures ${Project_Libary_Mode} ${DataStructures_SRCS})
    set_target_properties(DataStructures PROPERTIES COMPILE_FLAGS "${CMAKE_CXX_FLAGS} /D_SCL_SECURE_NO_WARNINGS=1")
  endif()
else()
  add_library(DataStructures ${Project_Libary_Mode} ${DataStructures_SRCS})
endif()

if(BUILD_WINDOWS)
  set_target_properties(DataStructures PROPERTIES STATIC_LIBRARY_FLAGS "${STATIC_LIB_FLAGS}")
else()
  set_target_properties(DataStructures PROPERTIES STATIC_LIBRARY_FLAGS "${STATIC_LIB_FLAGS}")
endif()
