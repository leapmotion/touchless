include_directories(
  ${PROJECT_SOURCE_DIR}/application
  ${PROJECT_SOURCE_DIR}/source
  ${PROJECT_SOURCE_DIR}/source/API
  ${PROJECT_SOURCE_DIR}/source/Utility
  ${PROJECT_SOURCE_DIR}/source/FilterMethods
  ${PROJECT_SOURCE_DIR}/source/DataStructures
  ${PROJECT_SOURCE_DIR}/source/Peripherals
  ${PROJECT_SOURCE_DIR}/source/OSInteraction
  ${PROJECT_SOURCE_DIR}/source/Overlay
  ${PROJECT_SOURCE_DIR}/source/GestureInteraction
  ${LEAP_INCLUDE_DIR}
  )

file(COPY touchless-icon.ico DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/)

if(USE_QT)
  include_directories(${QT_QTOPENGL_INCLUDE_DIR} ${QT_QTGUI_INCLUDE_DIR} ${QT_HEADERS_DIR})
  set(QT_USE_QTNETWORK TRUE)
  set(QT_USE_QTWEBKIT TRUE)
  set(QT_USE_QTSCRIPT TRUE)
  include(${QT_USE_FILE})
  set(Touchless_HEADERS Touchless.h TouchlessListener.h TouchlessUI.h)
  set(Touchless_SOURCES Touchless.cpp TouchlessListener.cpp TouchlessUI.cpp)
  Qt4_Wrap_Cpp(Touchless_HEADERS_MOC ${Touchless_HEADERS})
  add_definitions(${QT_DEFINITIONS})
  if(BUILD_WINDOWS)
    if(USE_PUBLIC_SETTINGS AND NOT UNSIGNED_BINARIES_EXPECTED)
      set(UIACCESS "true")
    else()
      set(UIACCESS "false")
    endif()
    configure_file(Touchless.rc.in ${CMAKE_CURRENT_BINARY_DIR}/Touchless.rc @ONLY)
    configure_file(Touchless.exe.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/Touchless.exe.manifest @ONLY)
    add_executable(Touchless WIN32 ${Touchless_HEADERS_MOC} ${Touchless_SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/Touchless.rc)
    set_target_properties(Touchless PROPERTIES LINK_FLAGS "/MANIFEST:NO")
  else()
    add_executable(Touchless ${Touchless_HEADERS} ${Touchless_HEADERS_MOC} ${Touchless_SOURCES})
  endif()
  target_strip(Touchless)
  target_link_libraries(Touchless Utility Overlay OSInteraction GestureInteraction ${LEAP_LIBRARIES} ${QT_LIBRARIES} )
endif()