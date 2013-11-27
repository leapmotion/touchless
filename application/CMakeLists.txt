include_directories(
  ${PROJECT_SOURCE_DIR}/application
  ${PROJECT_SOURCE_DIR}/source
  ${PROJECT_SOURCE_DIR}/source/API
  ${PROJECT_SOURCE_DIR}/source/Utility
  ${PROJECT_SOURCE_DIR}/source/FilterMethods
  ${PROJECT_SOURCE_DIR}/source/DataStructures
  ${LEAP_INCLUDE_DIR}
  )

if(USE_QT)
  include_directories(${QT_QTOPENGL_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_HEADERS_DIR})
endif()

add_subdirectory(Touchless)
