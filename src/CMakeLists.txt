SET(PATTERNS_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/patterns/crtp_pattern.h
  ${PROJECT_SOURCE_DIR}/include/common/patterns/singleton_pattern.h
)

SET(PATTERNS_SOURCES
)

SET(APPLIATION_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/application/application.h
)

SET(APPLIATION_SOURCES
  application/application.cpp
)

SET(SYSTEM_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/system/system.h
)

SET(SYSTEM_SOURCES
  system/system.cpp
)

SET(THREADS_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/threads/thread.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/thread_manager.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/platform_thread.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/event_bus.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/types.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/event.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/event_dispatcher.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/thread_pool.h
  ${PROJECT_SOURCE_DIR}/include/common/threads/std_thread_pool_impl.hpp
)

SET(THREADS_SOURCES
  threads/thread.cpp
  threads/thread_manager.cpp
  threads/event_bus.cpp
  threads/event_dispatcher.cpp
)

SET(NET_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/net/types.h
  ${PROJECT_SOURCE_DIR}/include/common/net/net.h
  ${PROJECT_SOURCE_DIR}/include/common/net/socket_tcp.h
)

SET(NET_SOURCES
  net/types.cpp
  net/net.cpp
  net/socket_tcp.cpp
)

SET(LIBEV_TCP_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/libev/tcp/tcp_client.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/tcp/tcp_server.h
)

SET(LIBEV_SOURCES
  libev/tcp/tcp_client.cpp
  libev/tcp/tcp_server.cpp
)

SET(LIBEV_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/libev/types.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/loop_controller.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/io_loop.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/io_loop_observer.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/io_client.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/event_loop.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/event_async.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/event_io.h
  ${PROJECT_SOURCE_DIR}/include/common/libev/event_timer.h

  ${LIBEV_TCP_HEADERS}
)

SET(LIBEV_SOURCES
  libev/event_loop.cpp
  libev/io_client.cpp
  libev/io_loop.cpp
  libev/io_loop_observer.cpp
  libev/loop_controller.cpp
  libev/types.cpp
  libev/event_async.cpp
  libev/event_io.cpp
  libev/event_timer.cpp

  ${LIBEV_SOURCES}
)

SET(HTTP_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/http/http.h
  ${PROJECT_SOURCE_DIR}/include/common/http/http2.h
  ${PROJECT_SOURCE_DIR}/include/common/http/http2_huffman.h
)

SET(HTTP_SOURCES
  http/http.cpp
  http/http2.cpp
  http/http2_huffman.cpp
)

SET(TEXT_DECODERS_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/hex_edcoder.h
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/html_edcoder.h
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/iedcoder.h
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/base64_edcoder.h
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/compress_edcoder.h
  ${PROJECT_SOURCE_DIR}/include/common/text_decoders/msgpack_edcoder.h
)

SET(TEXT_DECODERS_SOURCES
  text_decoders/hex_edcoder.cpp
  text_decoders/html_edcoder.cpp
  text_decoders/iedcoder.cpp
  text_decoders/base64_edcoder.cpp
  text_decoders/compress_edcoder.cpp
  text_decoders/msgpack_edcoder.cpp
)

SET(HASH_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/hash/md5.h
  ${PROJECT_SOURCE_DIR}/include/common/hash/sha1.h
)

SET(HASH_SOURCES
  hash/md5.cpp
  hash/sha1.cpp
)

SET(CMP_SOURCES third-party/cmp/cmp.c)

SET(SYSTEM_INFO_HEADERS
  ${PROJECT_SOURCE_DIR}/include/common/system_info/cpu_info.h
  ${PROJECT_SOURCE_DIR}/include/common/system_info/system_info.h
  ${PROJECT_SOURCE_DIR}/include/common/system_info/types.h
)

SET(SYSTEM_INFO_SOURCES
  system_info/cpu_info.cpp
  system_info/system_info.cpp
  system_info/types.cpp
)

IF(OS_WINDOWS)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${PROJECT_SOURCE_DIR}/include/common/string_util_win.h)
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_win.cpp system/system_win.cpp)
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ws2_32)
ELSEIF(OS_POSIX)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${PROJECT_SOURCE_DIR}/include/common/string_util_posix.h)
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_posix.cpp system/system_posix.cpp)
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})

  IF(OS_MACOSX)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_mac.cpp
        system_info/system_info_mac.mm system/system_mac.mm file_system_mac.mm
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${COCOA_LIBRARY})
  ELSEIF(OS_LINUX)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_linux.cpp)
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} rt)
  ELSEIF(OS_ANDROID)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_android.cpp)
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ELSEIF(OS_FREEBSD)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} system_info/system_info_freebsd.cpp)
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ENDIF(OS_MACOSX)
ELSE()
  MESSAGE(FATAL_ERROR "Not have system_info impl for: ${CMAKE_SYSTEM_NAME}")
ENDIF(OS_WINDOWS)

IF(QT_ENABLED)
  DETECT_QT()
  SET(QT_COMPONENTS_TO_USE Qt5Core Qt5Gui Qt5Widgets)
  IF(OS_MACOSX)
    SET(QT_COMPONENTS_TO_USE ${QT_COMPONENTS_TO_USE} Qt5MacExtras)
  ELSEIF(OS_LINUX)
    SET(QT_COMPONENTS_TO_USE ${QT_COMPONENTS_TO_USE} Qt5DBus) #for libqxcb
  ENDIF(OS_MACOSX)

  INTEGRATE_QT(${QT_COMPONENTS_TO_USE})
  SET(COMMON_QT_HEADERS
    ${PROJECT_SOURCE_DIR}/include/common/qt/utils_qt.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/convert2string.h
  )

  SET(COMMON_QT_SOURCES
   qt/utils_qt.cpp
   qt/convert2string.cpp
  )

  SET(FASTO_QT_HEADERS
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/shortcuts.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/base/tree_item.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/base/table_item.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/utils_qt.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/translations/translations.h
  )

  SET(FASTO_QT_TOMOC_HEADERS
    ${PROJECT_SOURCE_DIR}/include/common/qt/logger.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/glass_widget.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/icon_label.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/regexp_input_dialog.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/app_style.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/base/tree_model.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/base/table_model.h
    ${PROJECT_SOURCE_DIR}/include/common/qt/gui/base/graph_widget.h
  )

  SET(FASTO_QT_SOURCES
    qt/gui/shortcuts.cpp
    qt/utils_qt.cpp
    qt/logger.cpp
    qt/gui/base/tree_model.cpp
    qt/gui/base/tree_item.cpp
    qt/gui/base/table_model.cpp
    qt/gui/base/table_item.cpp
    qt/gui/base/graph_widget.cpp
    qt/gui/glass_widget.cpp
    qt/gui/icon_label.cpp
    qt/gui/regexp_input_dialog.cpp
    qt/gui/app_style.cpp
    qt/translations/translations.cpp
  )

  #SET(CMAKE_AUTOMOC ON)
  QT5_WRAP_CPP(FASTO_QT_MOC_FILES ${FASTO_QT_TOMOC_HEADERS})
  SET(COMMON_QT_HEADERS
    ${COMMON_QT_HEADERS}
    ${FASTO_QT_HEADERS}
    ${FASTO_QT_TOMOC_HEADERS}
  )

  SET(COMMON_QT_SOURCES
    ${COMMON_QT_SOURCES}
    ${FASTO_QT_SOURCES}
    ${FASTO_QT_MOC_FILES}
  )
ENDIF(QT_ENABLED)

IF(USE_PTHREAD)
  SET(THREADS_HDRS)
  SET(THREADS_SRC threads/posix_thread.cpp threads/posix_event.cpp)
  IF(NOT OS_ANDROID)
   SET(THREADS_LIBRARIES ${THREADS_LIBRARIES} pthread)
  ENDIF(NOT OS_ANDROID)
ELSE()
  SET(THREADS_SRC threads/win_thread.cpp)
  SET(THREADS_HDRS)
  SET(THREADS_LIBRARIES)
ENDIF(USE_PTHREAD)

IF(OS_WINDOWS)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${THREADS_HDRS})
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} ${THREADS_SRC})
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${THREADS_LIBRARIES})
ELSEIF(OS_MACOSX)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${THREADS_HDRS})
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} ${THREADS_SRC})
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${THREADS_LIBRARIES})
ELSEIF(OS_LINUX)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${THREADS_HDRS})
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} ${THREADS_SRC})
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${THREADS_LIBRARIES})
ELSEIF(OS_FREEBSD)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${THREADS_HDRS})
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} ${THREADS_SRC})
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${THREADS_LIBRARIES})
ELSEIF(OS_ANDROID)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${THREADS_HDRS})
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES} ${THREADS_SRC})
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${THREADS_LIBRARIES})
ENDIF(OS_WINDOWS)

SET(COMMON_HEADERS
    ${PROJECT_SOURCE_DIR}/include/common/log_levels.h
    ${PROJECT_SOURCE_DIR}/include/common/logger.h
    ${PROJECT_SOURCE_DIR}/include/common/string_compress.h
    ${PROJECT_SOURCE_DIR}/include/common/utf_string_conversion_utils.h
    ${PROJECT_SOURCE_DIR}/include/common/event.h
    ${PROJECT_SOURCE_DIR}/include/common/safe_sprintf.h
    ${PROJECT_SOURCE_DIR}/include/common/sprintf.h
    ${PROJECT_SOURCE_DIR}/include/common/utf_string_conversions.h
    ${PROJECT_SOURCE_DIR}/include/common/icu_utf.h
    ${PROJECT_SOURCE_DIR}/include/common/string_piece.h
    ${PROJECT_SOURCE_DIR}/include/common/string16.h
    ${PROJECT_SOURCE_DIR}/include/common/hash_tables.h
    ${PROJECT_SOURCE_DIR}/include/common/string_util.h
    ${PROJECT_SOURCE_DIR}/include/common/convert2string.h
    ${PROJECT_SOURCE_DIR}/include/common/types.h
    ${PROJECT_SOURCE_DIR}/include/common/lexical_cast.h
    ${PROJECT_SOURCE_DIR}/include/common/smart_ptr.h
    ${PROJECT_SOURCE_DIR}/include/common/macros.h
    ${PROJECT_SOURCE_DIR}/include/common/utils.h
    ${PROJECT_SOURCE_DIR}/include/common/bind_simple.h
    ${PROJECT_SOURCE_DIR}/include/common/file_system.h
    ${PROJECT_SOURCE_DIR}/include/common/portable_endian.h
    ${PROJECT_SOURCE_DIR}/include/common/error.h
    ${PROJECT_SOURCE_DIR}/include/common/value.h
    ${PROJECT_SOURCE_DIR}/include/common/intrusive_ptr.h
    ${PROJECT_SOURCE_DIR}/include/common/url.h
    ${PROJECT_SOURCE_DIR}/include/common/time.h

    ${PATTERNS_HEADERS}
    ${SYSTEM_HEADERS}
    ${APPLIATION_HEADERS}
    ${THREADS_HEADERS}
    ${LIBEV_HEADERS}
    ${NET_HEADERS}
    ${TEXT_DECODERS_HEADERS}
    ${HASH_HEADERS}
    ${SYSTEM_INFO_HEADERS}
    ${HTTP_HEADERS}
    ${COMMON_PLATFORM_HEADERS}
)

SET(COMMON_SOURCES
  log_levels.cpp logger.cpp string_compress.cpp event.cpp
  safe_sprintf.cpp sprintf.cpp utf_string_conversions.cpp utf_string_conversion_utils.cpp
  string_util.cpp icu_utf.cpp string_piece.cpp string16.cpp string_util_constants.cpp
  convert2string.cpp types.cpp lexical_cast.cpp
  macros.cpp file_system.cpp value.cpp error.cpp
  url.cpp time.cpp utils.cpp

  ${PATTERNS_SOURCES} ${APPLIATION_SOURCES} ${SYSTEM_SOURCES} ${THREADS_SOURCES}
  ${NET_SOURCES} ${LIBEV_SOURCES}
  ${TEXT_DECODERS_SOURCES} ${HASH_SOURCES} ${SETTINGS_SOURCES} ${SYSTEM_INFO_SOURCES}
  ${HTTP_SOURCES} ${COMMON_PLATFORM_SOURCES}
)

ADD_DEFINITIONS(-DHAVE_CONFIG_H) # need to refactor
ADD_SUBDIRECTORY(third-party/cpuid)
ADD_SUBDIRECTORY(third-party/libev)
SET(COMMON_LIBS ${COMMON_LIBS} ${COMMON_PLATFORM_LIBRARIES})

SET(COMMON_PROJECT_NAME ${PROJECT_NAME_LOWERCASE})
SET(PRIVATE_INCLUDE_DIRECTORIES ${PRIVATE_INCLUDE_DIRECTORIES} third-party)
ADD_LIBRARY(${COMMON_PROJECT_NAME} ${COMMON_HEADERS} ${COMMON_SOURCES} ${CMP_SOURCES}
  $<TARGET_OBJECTS:libev> $<TARGET_OBJECTS:cpuid>)
TARGET_INCLUDE_DIRECTORIES(${COMMON_PROJECT_NAME}
  PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES}
  PUBLIC $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)
#SET_TARGET_PROPERTIES(${COMMON_PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${COMMON_HEADERS}")
TARGET_LINK_LIBRARIES(${COMMON_PROJECT_NAME} PRIVATE ${COMMON_LIBS})

SET(TARGET_INSTALL_DESTINATION lib)
SET(HEADERS_INSTALL_DESTINATION include)

INSTALL(TARGETS ${COMMON_PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
#    PUBLIC_HEADER DESTINATION include/common
)
INSTALL(DIRECTORY ${PROJECT_SOURCE_DIR}/include/common DESTINATION ${HEADERS_INSTALL_DESTINATION})
#INSTALL(FILES  ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION})

#INSTALL(FILES ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION} COMPONENT HEADERS)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME LICENSE OPTIONAL)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
#INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)

IF(QT_ENABLED)
  SET(COMMON_QT_LIBS ${COMMON_LIBS} ${QT_LIBRARIES})
  SET(COMMON_QT_PROJECT_NAME ${COMMON_PROJECT_NAME}_qt)
  ADD_LIBRARY(${COMMON_QT_PROJECT_NAME} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES})
  TARGET_INCLUDE_DIRECTORIES(${COMMON_QT_PROJECT_NAME}
    PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)
  TARGET_LINK_LIBRARIES(${COMMON_QT_PROJECT_NAME} PRIVATE ${COMMON_QT_LIBS})

  INSTALL(TARGETS ${COMMON_QT_PROJECT_NAME} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT APPLICATIONS)
ENDIF(QT_ENABLED)

IF (DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES ${COMMON_HEADERS} ${COMMON_SOURCES} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${COMMON_PROJECT_NAME})
  IF(QT_ENABLED)
    REGISTER_CHECK_INCLUDES_TARGET(${COMMON_QT_PROJECT_NAME})
  ENDIF(QT_ENABLED)
ENDIF(DEVELOPER_CHECK_STYLE)

IF(DEVELOPER_ENABLE_TESTS)
  FIND_PACKAGE(GTest REQUIRED)
  ADD_DEFINITIONS(-DPROJECT_TEST_SOURCES_DIR="${CMAKE_SOURCE_DIR}/tests")

  SET(UNIT_TESTS_PROJECT_NAME ${COMMON_PROJECT_NAME}_unit_tests)
  ADD_EXECUTABLE(${UNIT_TESTS_PROJECT_NAME}
    ${CMAKE_SOURCE_DIR}/tests/unit_test_error.cpp

    ${CMAKE_SOURCE_DIR}/tests/unit_test_libev.cpp

    ${CMAKE_SOURCE_DIR}/tests/unit_test_url.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_path.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_filesystem.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_http.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_net.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_strings.cpp
  )

  TARGET_LINK_LIBRARIES(${UNIT_TESTS_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} common)

  ADD_TEST_TARGET(${UNIT_TESTS_PROJECT_NAME})
  SET_PROPERTY(TARGET ${UNIT_TESTS_PROJECT_NAME} PROPERTY FOLDER "Unit tests")

  SET(OTHER_TESTS_PROJECT_NAME ${COMMON_PROJECT_NAME}_other_tests)
  ADD_EXECUTABLE(${OTHER_TESTS_PROJECT_NAME}
    #
    ${CMAKE_SOURCE_DIR}/tests/test_cpu.cpp
    ${CMAKE_SOURCE_DIR}/tests/test_thread_common.cpp
    ${CMAKE_SOURCE_DIR}/tests/test_thread_func.h
    ${CMAKE_SOURCE_DIR}/tests/test_thread_func.cpp

    # performance

    ${CMAKE_SOURCE_DIR}/tests/test_thread.cpp
    ${CMAKE_SOURCE_DIR}/tests/test_thread_empty.cpp
    ${CMAKE_SOURCE_DIR}/tests/test_thread_many.cpp
    ${CMAKE_SOURCE_DIR}/tests/test_thread_many_empty.cpp
  )

  TARGET_LINK_LIBRARIES(${OTHER_TESTS_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} common)

  ADD_TEST_TARGET(${OTHER_TESTS_PROJECT_NAME})
  SET_PROPERTY(TARGET ${OTHER_TESTS_PROJECT_NAME} PROPERTY FOLDER "Other tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
