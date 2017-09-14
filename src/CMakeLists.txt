IF (DEVELOPER_ENABLE_COVERALLS)
  INCLUDE(Coveralls)
  #COVERALLS_TURN_ON_COVERAGE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
ENDIF(DEVELOPER_ENABLE_COVERALLS)

SET(PATTERNS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/patterns/crtp_pattern.h
  ${CMAKE_SOURCE_DIR}/include/common/patterns/singleton_pattern.h
)

SET(PATTERNS_SOURCES
)

SET(APPLIATION_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/application/application.h
)

SET(APPLIATION_SOURCES
  ${CMAKE_SOURCE_DIR}/src/application/application.cpp
)

SET(URI_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/uri/upath.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url.h
)

SET(URI_SOURCES
  ${CMAKE_SOURCE_DIR}/src/uri/upath.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url.cpp
)


SET(SYSTEM_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/system/system.h
)

SET(SYSTEM_SOURCES
  ${CMAKE_SOURCE_DIR}/src/system/system.cpp
)

SET(FILE_SYSTEM_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/file_system/types.h

  ${CMAKE_SOURCE_DIR}/include/common/file_system/string_path_utils.h

  ${CMAKE_SOURCE_DIR}/include/common/file_system/file_system.h
  ${CMAKE_SOURCE_DIR}/include/common/file_system/file_system_utils.h

  ${CMAKE_SOURCE_DIR}/include/common/file_system/path.h
  ${CMAKE_SOURCE_DIR}/include/common/file_system/descriptor_holder.h
  ${CMAKE_SOURCE_DIR}/include/common/file_system/file.h
)

SET(FILE_SYSTEM_SOURCES
  ${CMAKE_SOURCE_DIR}/src/file_system/file_system.cpp
  ${CMAKE_SOURCE_DIR}/src/file_system/file_system_utils.cpp

  ${CMAKE_SOURCE_DIR}/src/file_system/path.cpp
  ${CMAKE_SOURCE_DIR}/src/file_system/descriptor_holder.cpp
  ${CMAKE_SOURCE_DIR}/src/file_system/file.cpp
  ${CMAKE_SOURCE_DIR}/src/file_system/string_path_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/file_system/types.cpp
)

SET(THREADS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/threads/thread.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/thread_manager.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/platform_thread.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event_bus.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/types.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event_dispatcher.h
)

SET(THREADS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/threads/thread.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/thread_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/event_bus.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/event_dispatcher.cpp
)

SET(NET_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/net/types.h
  ${CMAKE_SOURCE_DIR}/include/common/net/socket_info.h
  ${CMAKE_SOURCE_DIR}/include/common/net/net.h
  ${CMAKE_SOURCE_DIR}/include/common/net/socket_tcp.h
)

SET(NET_SOURCES
  ${CMAKE_SOURCE_DIR}/src/net/types.cpp
  ${CMAKE_SOURCE_DIR}/src/net/socket_info.cpp
  ${CMAKE_SOURCE_DIR}/src/net/net.cpp
  ${CMAKE_SOURCE_DIR}/src/net/socket_tcp.cpp
)

SET(DRAW_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/draw/types.h
)

SET(DRAW_SOURCES
  ${CMAKE_SOURCE_DIR}/src/draw/types.cpp
)

SET(MEDIA_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/media/types.h
)

SET(MEDIA_SOURCES
  ${CMAKE_SOURCE_DIR}/src/media/types.cpp
)

SET(COMPRESS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/compress/hex.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/base64.h
)

SET(COMPRESS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/compress/hex.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/base64.cpp
)

#dependencies ${ZLIB_LIBRARIES} ${SNAPPY_LIBRARIES}
IF(ZLIB_FOUND)
  SET(COMPRESS_HEADERS ${COMPRESS_HEADERS} ${CMAKE_SOURCE_DIR}/include/common/compress/zlib_compress.h)
  SET(COMPRESS_SOURCES ${COMPRESS_SOURCES} ${CMAKE_SOURCE_DIR}/src/compress/zlib_compress.cpp)
  SET(COMMON_LIBS ${COMMON_LIBS} ${ZLIB_LIBRARIES})
ENDIF(ZLIB_FOUND)

IF(SNAPPY_FOUND)
  SET(COMPRESS_HEADERS ${COMPRESS_HEADERS} ${CMAKE_SOURCE_DIR}/include/common/compress/snappy_compress.h)
  SET(COMPRESS_SOURCES ${COMPRESS_SOURCES} ${CMAKE_SOURCE_DIR}/src/compress/snappy_compress.cpp)
  SET(COMMON_LIBS ${COMMON_LIBS} ${SNAPPY_LIBRARIES})
ENDIF(SNAPPY_FOUND)

SET(LIBEV_TCP_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/libev/tcp/tcp_client.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/tcp/tcp_server.h
)

SET(LIBEV_TCP_SOURCES
  ${CMAKE_SOURCE_DIR}/src/libev/tcp/tcp_client.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/tcp/tcp_server.cpp
)

SET(LIBEV_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/libev/types.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/loop_controller.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/io_loop.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/io_loop_observer.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/io_client.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/event_loop.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/event_async.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/event_io.h
  ${CMAKE_SOURCE_DIR}/include/common/libev/event_timer.h

  ${LIBEV_TCP_HEADERS}
)

SET(LIBEV_SOURCES
  ${CMAKE_SOURCE_DIR}/src/libev/event_loop.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/io_client.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/io_loop.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/io_loop_observer.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/loop_controller.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/types.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/event_async.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/event_io.cpp
  ${CMAKE_SOURCE_DIR}/src/libev/event_timer.cpp

  ${LIBEV_TCP_SOURCES}
)

SET(HTTP_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/http/http.h
  ${CMAKE_SOURCE_DIR}/include/common/http/http2.h
  ${CMAKE_SOURCE_DIR}/include/common/http/http2_huffman.h
)

SET(HTTP_SOURCES
  ${CMAKE_SOURCE_DIR}/src/http/http.cpp
  ${CMAKE_SOURCE_DIR}/src/http/http2.cpp
  ${CMAKE_SOURCE_DIR}/src/http/http2_huffman.cpp
)

SET(TEXT_DECODERS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/hex_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/html_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/iedcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/iedcoder_factory.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/base64_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_zlib_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_snappy_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/msgpack_edcoder.h
)

SET(TEXT_DECODERS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/text_decoders/hex_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/html_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/iedcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/iedcoder_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/base64_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_zlib_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_snappy_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/msgpack_edcoder.cpp
)

SET(HASH_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/hash/md5.h
  ${CMAKE_SOURCE_DIR}/include/common/hash/sha1.h
)

SET(HASH_SOURCES
  ${CMAKE_SOURCE_DIR}/src/hash/md5.cpp
  ${CMAKE_SOURCE_DIR}/src/hash/sha1.cpp
)

SET(CMP_SOURCES third-party/cmp/cmp.c)

SET(SYSTEM_INFO_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/system_info/cpu_info.h
  ${CMAKE_SOURCE_DIR}/include/common/system_info/system_info.h
  ${CMAKE_SOURCE_DIR}/include/common/system_info/types.h
)

SET(SYSTEM_INFO_SOURCES
  ${CMAKE_SOURCE_DIR}/src/system_info/cpu_info.cpp
  ${CMAKE_SOURCE_DIR}/src/system_info/system_info.cpp
  ${CMAKE_SOURCE_DIR}/src/system_info/types.cpp
)

IF(OS_WINDOWS)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${CMAKE_SOURCE_DIR}/include/common/string_util_win.h)
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/system_info/system_info_win.cpp
    ${CMAKE_SOURCE_DIR}/src/system/system_win.cpp
  )
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ws2_32)
ELSEIF(OS_POSIX)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${CMAKE_SOURCE_DIR}/include/common/string_util_posix.h)
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/system_info/system_info_posix.cpp
    ${CMAKE_SOURCE_DIR}/src/system/system_posix.cpp
  )
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})

  IF(OS_MACOSX)
    FIND_LIBRARY(COCOA_LIBRARY Cocoa)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_mac.cpp
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_mac.mm
      ${CMAKE_SOURCE_DIR}/src/system/system_mac.mm
      ${CMAKE_SOURCE_DIR}/src/file_system/file_system_mac.mm
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ${COCOA_LIBRARY})
  ELSEIF(OS_LINUX)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_linux.cpp
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} rt)
  ELSEIF(OS_ANDROID)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_android.cpp
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ELSEIF(OS_FREEBSD)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_freebsd.cpp
    )
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
    ${CMAKE_SOURCE_DIR}/include/common/qt/utils_qt.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/convert2string.h
  )

  SET(COMMON_QT_SOURCES
   ${CMAKE_SOURCE_DIR}/src/qt/utils_qt.cpp
   ${CMAKE_SOURCE_DIR}/src/qt/convert2string.cpp
  )

  SET(FASTO_QT_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/shortcuts.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/base/tree_item.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/base/table_item.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/utils_qt.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/translations/translations.h
  )

  SET(FASTO_QT_TOMOC_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/qt/logger.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/glass_widget.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/icon_label.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/regexp_input_dialog.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/app_style.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/base/tree_model.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/base/table_model.h
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/base/graph_widget.h
  )

  SET(FASTO_QT_SOURCES
    ${CMAKE_SOURCE_DIR}/src/qt/gui/shortcuts.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/utils_qt.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/logger.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/base/tree_model.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/base/tree_item.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/base/table_model.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/base/table_item.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/base/graph_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/glass_widget.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/icon_label.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/regexp_input_dialog.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/gui/app_style.cpp
    ${CMAKE_SOURCE_DIR}/src/qt/translations/translations.cpp
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
  SET(THREADS_SRC
    ${CMAKE_SOURCE_DIR}/src/threads/posix_thread.cpp
    ${CMAKE_SOURCE_DIR}/src/threads/posix_event.cpp
  )
  IF(NOT OS_ANDROID)
   SET(THREADS_LIBRARIES ${THREADS_LIBRARIES} pthread)
  ENDIF(NOT OS_ANDROID)
ELSE()
  SET(THREADS_SRC
    ${CMAKE_SOURCE_DIR}/src/threads/win_thread.cpp
  )
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
    ${CMAKE_SOURCE_DIR}/include/common/log_levels.h
    ${CMAKE_SOURCE_DIR}/include/common/logger.h
    ${CMAKE_SOURCE_DIR}/include/common/macros.h
    ${CMAKE_SOURCE_DIR}/include/common/optional.h
    ${CMAKE_SOURCE_DIR}/include/common/utf_string_conversion_utils.h
    ${CMAKE_SOURCE_DIR}/include/common/event.h
    ${CMAKE_SOURCE_DIR}/include/common/safe_sprintf.h
    ${CMAKE_SOURCE_DIR}/include/common/sprintf.h
    ${CMAKE_SOURCE_DIR}/include/common/byte_writer.h
    ${CMAKE_SOURCE_DIR}/include/common/utf_string_conversions.h
    ${CMAKE_SOURCE_DIR}/include/common/icu_utf.h
    ${CMAKE_SOURCE_DIR}/include/common/string_piece.h
    ${CMAKE_SOURCE_DIR}/include/common/string16.h
    ${CMAKE_SOURCE_DIR}/include/common/hash_tables.h
    ${CMAKE_SOURCE_DIR}/include/common/string_util.h
    ${CMAKE_SOURCE_DIR}/include/common/convert2string.h
    ${CMAKE_SOURCE_DIR}/include/common/types.h
    ${CMAKE_SOURCE_DIR}/include/common/bounded_value.h
    ${CMAKE_SOURCE_DIR}/include/common/utils.h
    ${CMAKE_SOURCE_DIR}/include/common/bind_simple.h
    ${CMAKE_SOURCE_DIR}/include/common/portable_endian.h
    ${CMAKE_SOURCE_DIR}/include/common/sys_byteorder.h
    ${CMAKE_SOURCE_DIR}/include/common/error.h
    ${CMAKE_SOURCE_DIR}/include/common/value.h
    ${CMAKE_SOURCE_DIR}/include/common/intrusive_ptr.h
    ${CMAKE_SOURCE_DIR}/include/common/time.h

    ${PATTERNS_HEADERS}
    ${SYSTEM_HEADERS}
    ${FILE_SYSTEM_HEADERS}
    ${APPLIATION_HEADERS}
    ${URI_HEADERS}
    ${THREADS_HEADERS}
    ${LIBEV_HEADERS}
    ${NET_HEADERS}
    ${DRAW_HEADERS}
    ${MEDIA_HEADERS}
    ${TEXT_DECODERS_HEADERS}
    ${HASH_HEADERS}
    ${SYSTEM_INFO_HEADERS}
    ${HTTP_HEADERS}
    ${COMMON_PLATFORM_HEADERS}
    ${COMPRESS_HEADERS}
)

SET(COMMON_SOURCES
  ${CMAKE_SOURCE_DIR}/src/log_levels.cpp
  ${CMAKE_SOURCE_DIR}/src/logger.cpp
  ${CMAKE_SOURCE_DIR}/src/event.cpp
  ${CMAKE_SOURCE_DIR}/src/safe_sprintf.cpp
  ${CMAKE_SOURCE_DIR}/src/sprintf.cpp
  ${CMAKE_SOURCE_DIR}/src/byte_writer.cpp
  ${CMAKE_SOURCE_DIR}/src/utf_string_conversions.cpp
  ${CMAKE_SOURCE_DIR}/src/utf_string_conversion_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/string_util.cpp
  ${CMAKE_SOURCE_DIR}/src/icu_utf.cpp
  ${CMAKE_SOURCE_DIR}/src/string_piece.cpp
  ${CMAKE_SOURCE_DIR}/src/string16.cpp
  ${CMAKE_SOURCE_DIR}/src/string_util_constants.cpp
  ${CMAKE_SOURCE_DIR}/src/convert2string.cpp
  ${CMAKE_SOURCE_DIR}/src/types.cpp
  ${CMAKE_SOURCE_DIR}/src/bounded_value.cpp
  ${CMAKE_SOURCE_DIR}/src/macros.cpp
  ${CMAKE_SOURCE_DIR}/src/value.cpp
  ${CMAKE_SOURCE_DIR}/src/error.cpp
  ${CMAKE_SOURCE_DIR}/src/time.cpp
  ${CMAKE_SOURCE_DIR}/src/utils.cpp

  ${PATTERNS_SOURCES}
  ${APPLIATION_SOURCES}
  ${URI_SOURCES}
  ${SYSTEM_SOURCES}
  ${FILE_SYSTEM_SOURCES}
  ${THREADS_SOURCES}
  ${NET_SOURCES}
  ${DRAW_SOURCES}
  ${MEDIA_SOURCES}
  ${LIBEV_SOURCES}
  ${TEXT_DECODERS_SOURCES}
  ${HASH_SOURCES}
  ${SETTINGS_SOURCES}
  ${SYSTEM_INFO_SOURCES}
  ${HTTP_SOURCES}
  ${COMMON_PLATFORM_SOURCES}
  ${COMPRESS_SOURCES}
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
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)
#SET_TARGET_PROPERTIES(${COMMON_PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${COMMON_HEADERS}")
TARGET_LINK_LIBRARIES(${COMMON_PROJECT_NAME} PRIVATE ${COMMON_LIBS})

SET(TARGET_INSTALL_DESTINATION lib)
SET(HEADERS_INSTALL_DESTINATION include)

INSTALL(TARGETS ${COMMON_PROJECT_NAME}
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
#   PUBLIC_HEADER DESTINATION include/common
)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/common DESTINATION ${HEADERS_INSTALL_DESTINATION})
#INSTALL(FILES  ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION})

#INSTALL(FILES ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION} COMPONENT HEADERS)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME LICENSE OPTIONAL)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
#INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
SET(COMMON_INSTALL_LIBS ${COMMON_INSTALL_LIBS} ${COMMON_PROJECT_NAME})

IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")
  STRIP_TARGET(${COMMON_PROJECT_NAME})
ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")

IF(QT_ENABLED)
  SET(COMMON_QT_LIBS ${COMMON_LIBS} ${QT_LIBRARIES})
  SET(COMMON_QT_PROJECT_NAME ${COMMON_PROJECT_NAME}_qt)
  ADD_LIBRARY(${COMMON_QT_PROJECT_NAME} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES})
  TARGET_INCLUDE_DIRECTORIES(${COMMON_QT_PROJECT_NAME}
    PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)
  TARGET_LINK_LIBRARIES(${COMMON_QT_PROJECT_NAME} PRIVATE ${COMMON_QT_LIBS})

  INSTALL(TARGETS ${COMMON_QT_PROJECT_NAME} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT APPLICATIONS)
  SET(COMMON_INSTALL_LIBS ${COMMON_INSTALL_LIBS} ${COMMON_QT_PROJECT_NAME})

  IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")
    STRIP_TARGET(${COMMON_QT_PROJECT_NAME})
  ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release")
ENDIF(QT_ENABLED)

IF (DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES ${COMMON_HEADERS} ${COMMON_SOURCES} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES})
  REGISTER_CHECK_STYLE_TARGET(check_style "${CHECK_SOURCES}")
  REGISTER_CHECK_INCLUDES_TARGET(${COMMON_PROJECT_NAME})
  IF(QT_ENABLED)
    REGISTER_CHECK_INCLUDES_TARGET(${COMMON_QT_PROJECT_NAME})
  ENDIF(QT_ENABLED)
ENDIF(DEVELOPER_CHECK_STYLE)

IF (DEVELOPER_ENABLE_COVERALLS)
  SET(COVERAGE_SRCS ${COMMON_HEADERS} ${COMMON_SOURCES} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES})

  # Create the coveralls target.
  COVERALLS_SETUP("${COVERAGE_SRCS}" ${DEVELOPER_COVERALLS_UPLOAD})
ENDIF(DEVELOPER_ENABLE_COVERALLS)

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
    ${CMAKE_SOURCE_DIR}/tests/unit_test_text_edecoders.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_value.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_logger.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_threads.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_hash.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_bounded_value.cpp
  )

  TARGET_LINK_LIBRARIES(${UNIT_TESTS_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} ${COMMON_INSTALL_LIBS} ${COMMON_LIBS})

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

  TARGET_LINK_LIBRARIES(${OTHER_TESTS_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} ${COMMON_INSTALL_LIBS} ${COMMON_LIBS})

  #ADD_TEST_TARGET(${OTHER_TESTS_PROJECT_NAME})
  #SET_PROPERTY(TARGET ${OTHER_TESTS_PROJECT_NAME} PROPERTY FOLDER "Other tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
