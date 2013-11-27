set(
  Cpp11_SRCS
  boost_exception_ptr.h
  boost_functional.h
  boost_future.h
  boost_rvalue.h
  boost_shared_ptr.h
  boost_type_traits.h
  cpp11.h
  custom_type_index.h
  stdafx.h
  stdafx.cpp
)

if(BUILD_WINDOWS)
  # warngs: stdafx.cpp.o has no symbols
  # test-compile on Windows, although not necessary there either
  add_library(Cpp11 STATIC ${Cpp11_SRCS})
endif()
