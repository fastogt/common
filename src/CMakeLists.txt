IF (DEVELOPER_ENABLE_COVERALLS)
  INCLUDE(Coveralls)
  #COVERALLS_TURN_ON_COVERAGE()
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage")
ENDIF(DEVELOPER_ENABLE_COVERALLS)

SET(COMMON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/include)

SET(PATTERNS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/patterns/crtp_pattern.h
  ${CMAKE_SOURCE_DIR}/include/common/patterns/singleton_pattern.h
)

SET(PATTERNS_SOURCES
)

SET(CONTAINERS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/containers/stack_container.h
  ${CMAKE_SOURCE_DIR}/include/common/containers/span.h
  ${CMAKE_SOURCE_DIR}/include/common/containers/checked_iterators.h
)

SET(APPLIATION_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/application/application.h
)

SET(APPLIATION_SOURCES
  ${CMAKE_SOURCE_DIR}/src/application/application.cpp
)

SET(SERIALIZER_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/serializer/iserializer.h
)

SET(SERIALIZER_SOURCES
  ${CMAKE_SOURCE_DIR}/src/serializer/iserializer.cpp
)

FIND_PACKAGE(JSON-C QUIET)
IF(JSONC_FOUND)
  SET(PROTOCOLS_HEADERS ${PROTOCOLS_HEADERS}
    ${CMAKE_SOURCE_DIR}/include/common/protocols/json_rpc/json_rpc.h
    ${CMAKE_SOURCE_DIR}/include/common/protocols/json_rpc/json_rpc_response.h
    ${CMAKE_SOURCE_DIR}/include/common/protocols/json_rpc/json_rpc_request.h
    ${CMAKE_SOURCE_DIR}/include/common/protocols/json_rpc/json_rpc_types.h
    ${CMAKE_SOURCE_DIR}/include/common/protocols/json_rpc/protocol_client.h
  )

  SET(PROTOCOLS_SOURCES ${PROTOCOLS_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/protocols/json_rpc/json_rpc.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/json_rpc/json_rpc_response.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/json_rpc/json_rpc_request.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/json_rpc/json_rpc_types.cpp
    ${CMAKE_SOURCE_DIR}/src/protocols/json_rpc/protocol_client.cpp
  )

  SET(SERIALIZER_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/serializer/iserializer.h
    ${CMAKE_SOURCE_DIR}/include/common/serializer/json_serializer.h
  )

  SET(SERIALIZER_SOURCES
    ${CMAKE_SOURCE_DIR}/src/serializer/json_serializer.cpp
  )

  SET(DAEMON_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/daemon/client.h
    ${CMAKE_SOURCE_DIR}/include/common/daemon/server.h

    ${CMAKE_SOURCE_DIR}/include/common/daemon/commands/stop_info.h
    ${CMAKE_SOURCE_DIR}/include/common/daemon/commands/ping_info.h
    ${CMAKE_SOURCE_DIR}/include/common/daemon/commands/license_info.h
    ${CMAKE_SOURCE_DIR}/include/common/daemon/commands/activate_info.h
  )
  SET(DAEMON_SOURCES
    ${CMAKE_SOURCE_DIR}/src/daemon/client.cpp
    ${CMAKE_SOURCE_DIR}/src/daemon/server.cpp

    ${CMAKE_SOURCE_DIR}/src/daemon/commands/stop_info.cpp
    ${CMAKE_SOURCE_DIR}/src/daemon/commands/ping_info.cpp
    ${CMAKE_SOURCE_DIR}/src/daemon/commands/license_info.cpp
    ${CMAKE_SOURCE_DIR}/src/daemon/commands/activate_info.cpp
  )

  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${JSONC_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${JSONC_LIBRARIES})
ENDIF(JSONC_FOUND)

FIND_PACKAGE(Cpuid QUIET)
IF(CPUID_FOUND)
  SET(SYSTEM_INFO_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/system_info/cpu_info.h
  )

  SET(SYSTEM_INFO_SOURCES
    ${CMAKE_SOURCE_DIR}/src/system_info/cpu_info.cpp
  )

  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${CPUID_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${CPUID_LIBRARIES})
ENDIF(CPUID_FOUND)

SET(URI_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/uri/gurl.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_canon_stdstring.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_parse.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_canon.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_parse_internal.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_constants.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_util_internal.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_util.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_canon_ip.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_canon_internal.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_file.h
  ${CMAKE_SOURCE_DIR}/include/common/uri/url_canon_icu.h
)

SET(URI_SOURCES
  ${CMAKE_SOURCE_DIR}/src/uri/gurl.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_stdstring.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_parse.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_constants.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_util.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_ip.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_internal.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_stdurl.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_etc.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_parse_file.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_parse_dev.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_fileurl.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_devurl.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_pathurl.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_path.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_relative.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_query.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_canon_host.cpp
  ${CMAKE_SOURCE_DIR}/src/uri/url_idna_icu.cpp
)

SET(SYSTEM_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/system/system.h
)

SET(SYSTEM_SOURCES
  ${CMAKE_SOURCE_DIR}/src/system/system.cpp
)

SET(PROCESS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/process/process_metrics.h
)

SET(PROCESS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/process/process_metrics.cpp
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
  ${CMAKE_SOURCE_DIR}/include/common/threads/barrier.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/ts_queue.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/thread.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/thread_manager.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/platform_thread.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event_bus.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/types.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/event_dispatcher.h
  ${CMAKE_SOURCE_DIR}/include/common/threads/thread_pool.h
)

SET(THREADS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/threads/barrier.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/thread.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/thread_manager.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/event_bus.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/event_dispatcher.cpp
  ${CMAKE_SOURCE_DIR}/src/threads/thread_pool.cpp
)

SET(NET_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/net/types.h
  ${CMAKE_SOURCE_DIR}/include/common/net/ip_address.h
  ${CMAKE_SOURCE_DIR}/include/common/net/socket_info.h
  ${CMAKE_SOURCE_DIR}/include/common/net/net.h
  ${CMAKE_SOURCE_DIR}/include/common/net/isocket.h
  ${CMAKE_SOURCE_DIR}/include/common/net/isocket_fd.h
  ${CMAKE_SOURCE_DIR}/include/common/net/socket_tcp.h
  ${CMAKE_SOURCE_DIR}/include/common/net/http_client.h
)

SET(NET_SOURCES
  ${CMAKE_SOURCE_DIR}/src/net/types.cpp
  ${CMAKE_SOURCE_DIR}/src/net/ip_address.cpp
  ${CMAKE_SOURCE_DIR}/src/net/socket_info.cpp
  ${CMAKE_SOURCE_DIR}/src/net/net.cpp
  ${CMAKE_SOURCE_DIR}/src/net/isocket.cpp
  ${CMAKE_SOURCE_DIR}/src/net/isocket_fd.cpp
  ${CMAKE_SOURCE_DIR}/src/net/socket_tcp.cpp
  ${CMAKE_SOURCE_DIR}/src/net/http_client.cpp
)

SET(MEMORY_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/memory/free_deleter.h
)

SET(MEMORY_SOURCES
  ${CMAKE_SOURCE_DIR}/src/memory/free_deleter.cpp
)

SET(DRAW_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/draw/types.h
)

SET(DRAW_SOURCES
  ${CMAKE_SOURCE_DIR}/src/draw/types.cpp
)

SET(MEDIA_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/media/types.h
  ${CMAKE_SOURCE_DIR}/include/common/media/bandwidth_estimation.h
)

SET(MEDIA_SOURCES
  ${CMAKE_SOURCE_DIR}/src/media/types.cpp
  ${CMAKE_SOURCE_DIR}/src/media/bandwidth_estimation.cpp
)

SET(COMPRESS_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/compress/coding.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/hex.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/xhex.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/unicode.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/uunicode.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/base64.h

  ${CMAKE_SOURCE_DIR}/include/common/compress/zlib_compress.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/bzip2_compress.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/snappy_compress.h
  ${CMAKE_SOURCE_DIR}/include/common/compress/lz4_compress.h
)

SET(COMPRESS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/compress/coding.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/hex.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/xhex.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/unicode.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/uunicode.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/base64.cpp

  ${CMAKE_SOURCE_DIR}/src/compress/zlib_compress.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/bzip2_compress.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/snappy_compress.cpp
  ${CMAKE_SOURCE_DIR}/src/compress/lz4_compress.cpp
)

#dependencies ${ZLIB_LIBRARIES} ${SNAPPY_LIBRARIES} ${LZ4_LIBRARIES} ${BZIP2_LIBRARIES}
IF(ZLIB_FOUND)
  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${ZLIB_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${ZLIB_LIBRARIES})
ENDIF(ZLIB_FOUND)

IF(BZIP2_FOUND)
  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${BZIP2_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${BZIP2_LIBRARIES})
ENDIF(BZIP2_FOUND)

IF(SNAPPY_FOUND)
  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${SNAPPY_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${SNAPPY_LIBRARIES})
ENDIF(SNAPPY_FOUND)

IF(LZ4_FOUND)
  SET(COMMON_INCLUDE_DIRS ${COMMON_INCLUDE_DIRS} ${LZ4_INCLUDE_DIRS})
  SET(COMMON_LIBS ${COMMON_LIBS} ${LZ4_LIBRARIES})
ENDIF(LZ4_FOUND)

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
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/xhex_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/unicode_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/uunicode_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/html_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/iedcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/iedcoder_factory.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/base64_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_zlib_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_bzip2_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_lz4_edcoder.h
  ${CMAKE_SOURCE_DIR}/include/common/text_decoders/compress_snappy_edcoder.h
)

SET(TEXT_DECODERS_SOURCES
  ${CMAKE_SOURCE_DIR}/src/text_decoders/hex_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/xhex_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/unicode_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/uunicode_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/html_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/iedcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/iedcoder_factory.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/base64_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_zlib_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_bzip2_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_lz4_edcoder.cpp
  ${CMAKE_SOURCE_DIR}/src/text_decoders/compress_snappy_edcoder.cpp
)

SET(HASH_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/hash/md5.h
  ${CMAKE_SOURCE_DIR}/include/common/hash/sha1.h
  ${CMAKE_SOURCE_DIR}/include/common/hash/sha256.h
)

SET(HASH_SOURCES
  ${CMAKE_SOURCE_DIR}/src/hash/md5.cpp
  ${CMAKE_SOURCE_DIR}/src/hash/sha1.cpp
  ${CMAKE_SOURCE_DIR}/src/hash/sha256.cpp
)

SET(MODP_B64_SOURCES
  ${CMAKE_SOURCE_DIR}/src/third-party/modp_b64/modp_b64.cpp
  ${CMAKE_SOURCE_DIR}/src/third-party/modp_b64/modp_b64.h
  ${CMAKE_SOURCE_DIR}/src/third-party/modp_b64/modp_b64_data.h
)

SET(SYSTEM_INFO_HEADERS ${SYSTEM_INFO_HEADERS}
  ${CMAKE_SOURCE_DIR}/include/common/system_info/system_info.h
  ${CMAKE_SOURCE_DIR}/include/common/system_info/types.h
)

SET(SYSTEM_INFO_SOURCES ${SYSTEM_INFO_SOURCES}
  ${CMAKE_SOURCE_DIR}/src/system_info/system_info.cpp
  ${CMAKE_SOURCE_DIR}/src/system_info/types.cpp
)

SET(LICENSE_HW_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/license/hardware_hash.h
  ${CMAKE_SOURCE_DIR}/include/common/license/utils.h
)
SET(LICENSE_HW_SOURCES
  ${CMAKE_SOURCE_DIR}/src/license/hardware_hash.cpp
  ${CMAKE_SOURCE_DIR}/src/license/utils.cpp
)

SET(LICENSE_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/license/expire_license.h
  ${CMAKE_SOURCE_DIR}/include/common/license/types.h
)
SET(LICENSE_SOURCES
  ${CMAKE_SOURCE_DIR}/src/license/expire_license.cpp
  ${CMAKE_SOURCE_DIR}/src/license/types.cpp
)

SET(LICENSE_HW_LIBRARIES)
SET(LICENSE_HW_INCLUDE_DIRS)
SET(LICENSE_HW_COMPILE_DEFINITIONS)

IF(OS_WINDOWS)
  SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/utils_win.cpp)
  SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS} ${CMAKE_SOURCE_DIR}/include/common/string_util_win.h)
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/system/system_win.cpp
    ${CMAKE_SOURCE_DIR}/src/system_info/system_info_win.cpp
    ${CMAKE_SOURCE_DIR}/src/process/process_metrics_win.cpp
  )
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} ws2_32)
  SET(COMMON_PLATFORM_DEFINITIONS)
  SET(COMMON_PLATFORM_INCLUDE_DIRS)
ELSEIF(OS_POSIX)
  SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS}
    ${CMAKE_SOURCE_DIR}/include/common/string_util_posix.h
  )
  SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/system_info/system_info_posix.cpp
    ${CMAKE_SOURCE_DIR}/src/system/system_posix.cpp
  )
  SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  SET(COMMON_PLATFORM_DEFINITIONS)
  SET(COMMON_PLATFORM_INCLUDE_DIRS)

  IF(OS_MACOSX)
    SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/license/utils_mac.cpp
      ${CMAKE_SOURCE_DIR}/src/license/utils_mac.mm
    )
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

    FIND_LIBRARY(FOUNDATION_LIBRARY Foundation)
    FIND_LIBRARY(IOKIT_LIBRARY IOKit)
    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_mac.cpp
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_mac.mm
      ${CMAKE_SOURCE_DIR}/src/system/system_mac.mm
      ${CMAKE_SOURCE_DIR}/src/file_system/string_path_utils.mm
      ${CMAKE_SOURCE_DIR}/src/process/process_metrics_mac.cpp
    )
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES} ${FOUNDATION_LIBRARY} ${IOKIT_LIBRARY})
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ELSEIF(OS_IOS)
    SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/utils_ios.cpp)
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES})
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ELSEIF(OS_LINUX)
    FIND_LIBRARY(UDEV_LIBRARY udev)
    FIND_PATH(UDEV_INCLUDE_DIR NAMES libudev.h)

    IF(UDEV_LIBRARY AND UDEV_INCLUDE_DIR)
      SET(LICENSE_HW_INCLUDE_DIRS ${LICENSE_HW_INCLUDE_DIRS} ${UDEV_INCLUDE_DIR})
      SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES} ${UDEV_LIBRARY})
      SET(LICENSE_HW_COMPILE_DEFINITIONS ${LICENSE_HW_COMPILE_DEFINITIONS} -DHAVE_UDEV)
    ENDIF(UDEV_LIBRARY AND UDEV_INCLUDE_DIR)
    SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/utils_linux.cpp)
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_linux.cpp
      ${CMAKE_SOURCE_DIR}/src/process/process_metrics_linux.cpp
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES} dl)
  ELSEIF(OS_ANDROID)
    SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/utils_android.cpp)
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_android.cpp
    )
    SET(COMMON_PLATFORM_LIBRARIES ${COMMON_PLATFORM_LIBRARIES})
  ELSEIF(OS_FREEBSD)
    FIND_LIBRARY(UDEV_LIBRARY udev)
    FIND_PATH(UDEV_INCLUDE_DIR NAMES libudev.h)

    IF(UDEV_LIBRARY AND UDEV_INCLUDE_DIR)
      SET(LICENSE_HW_INCLUDE_DIRS ${LICENSE_HW_INCLUDE_DIRS} ${UDEV_INCLUDE_DIR})
      SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES} ${UDEV_LIBRARY})
      SET(LICENSE_HW_COMPILE_DEFINITIONS ${LICENSE_HW_COMPILE_DEFINITIONS} -DHAVE_UDEV)
    ENDIF(UDEV_LIBRARY AND UDEV_INCLUDE_DIR)
    SET(LICENSE_HW_SOURCES ${LICENSE_HW_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/utils_freebsd.cpp)
    SET(LICENSE_HW_LIBRARIES ${LICENSE_HW_LIBRARIES})

    SET(COMMON_PLATFORM_HEADERS ${COMMON_PLATFORM_HEADERS})
    SET(COMMON_PLATFORM_SOURCES ${COMMON_PLATFORM_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/system_info/system_info_freebsd.cpp
      ${CMAKE_SOURCE_DIR}/src/process/process_metrics_freebsd.cpp
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
    ${CMAKE_SOURCE_DIR}/include/common/qt/gui/icon_combobox.h
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
    ${CMAKE_SOURCE_DIR}/src/qt/gui/icon_combobox.cpp
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

# common base
SET(COMMON_CONFIG_GEN_PATH ${CMAKE_SOURCE_DIR}/include/common/config.h)
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/src/config.h.in ${COMMON_CONFIG_GEN_PATH} @ONLY IMMEDIATE)

SET(COMMON_HEADERS
  ${CMAKE_SOURCE_DIR}/include/common/log_levels.h
  ${CMAKE_SOURCE_DIR}/include/common/logger.h
  ${CMAKE_SOURCE_DIR}/include/common/macros.h
  ${CMAKE_SOURCE_DIR}/include/common/template_util.h
  ${CMAKE_SOURCE_DIR}/include/common/parse_number.h
  ${CMAKE_SOURCE_DIR}/include/common/string_number_conversions.h
  ${CMAKE_SOURCE_DIR}/include/common/bit_cast.h
  ${CMAKE_SOURCE_DIR}/include/common/eintr_wrapper.h
  ${CMAKE_SOURCE_DIR}/include/common/optional.h
  ${CMAKE_SOURCE_DIR}/include/common/utf_string_conversion_utils.h
  ${CMAKE_SOURCE_DIR}/include/common/event.h
  ${CMAKE_SOURCE_DIR}/include/common/sprintf.h
  ${CMAKE_SOURCE_DIR}/include/common/byte_writer.h
  ${CMAKE_SOURCE_DIR}/include/common/utf_string_conversions.h
  ${CMAKE_SOURCE_DIR}/include/common/icu_utf.h
  ${CMAKE_SOURCE_DIR}/include/common/string_piece.h
  ${CMAKE_SOURCE_DIR}/include/common/string16.h
  ${CMAKE_SOURCE_DIR}/include/common/string_util.h
  ${CMAKE_SOURCE_DIR}/include/common/string_split.h
  ${CMAKE_SOURCE_DIR}/include/common/strcat.h
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
  ${CONTAINERS_HEADERS}
  ${SYSTEM_HEADERS}
  ${PROCESS_HEADERS}
  ${FILE_SYSTEM_HEADERS}
  ${APPLIATION_HEADERS}
  ${PROTOCOLS_HEADERS}
  ${SERIALIZER_HEADERS}
  ${URI_HEADERS}
  ${THREADS_HEADERS}
  ${NET_HEADERS}
  ${DRAW_HEADERS}
  ${MEMORY_HEADERS}
  ${MEDIA_HEADERS}
  ${TEXT_DECODERS_HEADERS}
  ${HASH_HEADERS}
  ${SYSTEM_INFO_HEADERS}
  ${LICENSE_HEADERS}
  ${LICENSE_HW_HEADERS}
  ${HTTP_HEADERS}
  ${COMMON_PLATFORM_HEADERS}
  ${COMPRESS_HEADERS}
  ${DAEMON_HEADERS}

  ${COMMON_CONFIG_GEN_PATH}
)

SET(COMMON_SOURCES
  ${CMAKE_SOURCE_DIR}/src/log_levels.cpp
  ${CMAKE_SOURCE_DIR}/src/logger.cpp
  ${CMAKE_SOURCE_DIR}/src/parse_number.cpp
  ${CMAKE_SOURCE_DIR}/src/string_number_conversions.cpp
  ${CMAKE_SOURCE_DIR}/src/event.cpp
  ${CMAKE_SOURCE_DIR}/src/sprintf.cpp
  ${CMAKE_SOURCE_DIR}/src/byte_writer.cpp
  ${CMAKE_SOURCE_DIR}/src/utf_string_conversions.cpp
  ${CMAKE_SOURCE_DIR}/src/utf_string_conversion_utils.cpp
  ${CMAKE_SOURCE_DIR}/src/string_util.cpp
  ${CMAKE_SOURCE_DIR}/src/string_split.cpp
  ${CMAKE_SOURCE_DIR}/src/strcat.cpp
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
  ${PROTOCOLS_SOURCES}  
  ${SERIALIZER_SOURCES}
  ${URI_SOURCES}
  ${SYSTEM_SOURCES}
  ${PROCESS_SOURCES}
  ${FILE_SYSTEM_SOURCES}
  ${THREADS_SOURCES}
  ${NET_SOURCES}
  ${MEMORY_SOURCES}
  ${DRAW_SOURCES}
  ${MEDIA_SOURCES}
  ${TEXT_DECODERS_SOURCES}
  ${HASH_SOURCES}
  ${SETTINGS_SOURCES}
  ${SYSTEM_INFO_SOURCES}
  ${LICENSE_SOURCES}
  ${LICENSE_HW_SOURCES}
  ${HTTP_SOURCES}
  ${COMMON_PLATFORM_SOURCES}
  ${COMPRESS_SOURCES}
  ${DAEMON_SOURCES}
  ${MODP_B64_SOURCES}
)

SET(COMMON_LIBS ${COMMON_LIBS} ${COMMON_PLATFORM_LIBRARIES} ${LICENSE_HW_LIBRARIES})

SET(COMMON_PROJECT_NAME ${PROJECT_NAME_LOWERCASE})
SET(LICENSE_GEN_NAME license_gen)

SET(PRIVATE_INCLUDE_DIRECTORIES ${PRIVATE_INCLUDE_DIRECTORIES} ${COMMON_INCLUDE_DIRS} ${COMMON_PLATFORM_INCLUDE_DIRS} ${LICENSE_HW_INCLUDE_DIRS} third-party)
SET(PRIVATE_COMPILE_DEFINITIONS ${PRIVATE_COMPILE_DEFINITIONS} ${COMMON_PLATFORM_DEFINITIONS} ${LICENSE_HW_COMPILE_DEFINITIONS})

ADD_LIBRARY(${COMMON_PROJECT_NAME} ${COMMON_HEADERS} ${COMMON_SOURCES})
TARGET_INCLUDE_DIRECTORIES(${COMMON_PROJECT_NAME}
  PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
)
TARGET_COMPILE_DEFINITIONS(${COMMON_PROJECT_NAME} PRIVATE ${PRIVATE_COMPILE_DEFINITIONS})
#SET_TARGET_PROPERTIES(${COMMON_PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${COMMON_HEADERS}")
TARGET_LINK_LIBRARIES(${COMMON_PROJECT_NAME} PRIVATE ${COMMON_LIBS})

# bins
SET(PRIVATE_COMPILE_DEFINITIONS ${PRIVATE_COMPILE_DEFINITIONS} -DLICENSE_GEN_NAME="${LICENSE_GEN_NAME}")
ADD_EXECUTABLE(${LICENSE_GEN_NAME} ${LICENSE_HW_SOURCES} ${HASH_SOURCES} ${CMAKE_SOURCE_DIR}/src/license/main.cpp)
TARGET_INCLUDE_DIRECTORIES(${LICENSE_GEN_NAME} PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES})
TARGET_COMPILE_DEFINITIONS(${LICENSE_GEN_NAME} PRIVATE ${PRIVATE_COMPILE_DEFINITIONS})
TARGET_LINK_LIBRARIES(${LICENSE_GEN_NAME} PRIVATE ${LICENSE_HW_LIBRARIES})

SET(TARGET_INSTALL_DESTINATION bin)
SET(LIBRARIES_INSTALL_DESTINATION lib)
SET(HEADERS_INSTALL_DESTINATION include)

INSTALL(TARGETS ${COMMON_PROJECT_NAME}
    LIBRARY DESTINATION ${LIBRARIES_INSTALL_DESTINATION}
    ARCHIVE DESTINATION ${LIBRARIES_INSTALL_DESTINATION}
#   PUBLIC_HEADER DESTINATION include/common
)
INSTALL(TARGETS ${LICENSE_GEN_NAME} DESTINATION ${TARGET_INSTALL_DESTINATION} COMPONENT RUNTIME)
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/include/common DESTINATION ${HEADERS_INSTALL_DESTINATION})
#INSTALL(FILES  ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION})

#INSTALL(FILES ${COMMON_HEADERS} DESTINATION ${HEADERS_INSTALL_DESTINATION} COMPONENT HEADERS)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/LICENSE DESTINATION ${LIBRARIES_INSTALL_DESTINATION} COMPONENT LICENSE RENAME LICENSE OPTIONAL)
#INSTALL(FILES ${CMAKE_SOURCE_DIR}/COPYRIGHT DESTINATION ${LIBRARIES_INSTALL_DESTINATION} COMPONENT LICENSE RENAME COPYRIGHT OPTIONAL)
#INSTALL(FILES ${PROJECT_CHANGELOG_FILE} DESTINATION ${LIBRARIES_INSTALL_DESTINATION} COMPONENT LICENSE RENAME CHANGELOG OPTIONAL)
SET(COMMON_INSTALL_LIBS ${COMMON_PROJECT_NAME} ${COMMON_INSTALL_LIBS})

IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")
  STRIP_TARGET(${COMMON_PROJECT_NAME})
ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")

# common libev
FIND_PACKAGE(LibEv QUIET)
IF(LIBEV_FOUND)
  SET(LIBEV_TCP_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/libev/tcp/tcp_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/tcp/tcp_server.h
  )

  SET(LIBEV_TCP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/libev/tcp/tcp_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/tcp/tcp_server.cpp
  )

  SET(LIBEV_HTTP_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http_server_info.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http2_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http_server.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http2_server.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/http/http_streams.h
  )

  SET(LIBEV_HTTP_SOURCES
    ${CMAKE_SOURCE_DIR}/src/libev/http/http_server_info.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/http/http_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/http/http2_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/http/http_server.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/http/http2_server.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/http/http_streams.cpp
  )

  SET(LIBEV_WEBSOCKET_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/libev/websocket/websocket_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/websocket/websocket_server.h
  )

  SET(LIBEV_WEBSOCKET_SOURCES
    ${CMAKE_SOURCE_DIR}/src/libev/websocket/websocket_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/websocket/websocket_server.cpp
  )

  SET(LIBEV_HEADERS
    ${CMAKE_SOURCE_DIR}/include/common/libev/types.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/loop_controller.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/io_loop.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/io_loop_observer.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/io_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/descriptor_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/pipe_client.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/event_loop.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/default_event_loop.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/event_async.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/event_io.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/event_timer.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/event_child.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/io_child.h
    ${CMAKE_SOURCE_DIR}/include/common/libev/io_base.h

    ${LIBEV_TCP_HEADERS}
    ${LIBEV_HTTP_HEADERS}
    ${LIBEV_WEBSOCKET_HEADERS}
  )

  SET(LIBEV_SOURCES
    ${CMAKE_SOURCE_DIR}/src/libev/event_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/default_event_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/io_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/descriptor_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/pipe_client.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/io_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/io_loop_observer.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/loop_controller.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/types.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/event_async.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/event_io.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/event_timer.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/event_child.cpp
    ${CMAKE_SOURCE_DIR}/src/libev/io_child.cpp

    ${LIBEV_TCP_SOURCES}
    ${LIBEV_HTTP_SOURCES}
    ${LIBEV_WEBSOCKET_SOURCES}
  )

  IF(OS_LINUX)
    SET(LIBEV_HEADERS ${LIBEV_HEADERS}
      ${CMAKE_SOURCE_DIR}/include/common/libev/inotify/types.h
      ${CMAKE_SOURCE_DIR}/include/common/libev/inotify/inotify_client.h
      ${CMAKE_SOURCE_DIR}/include/common/libev/inotify/inotify_client_observer.h
    )
    SET(LIBEV_SOURCES ${LIBEV_SOURCES}
      ${CMAKE_SOURCE_DIR}/src/libev/inotify/inotify_client.cpp
      ${CMAKE_SOURCE_DIR}/src/libev/inotify/inotify_client_observer.cpp
    )
  ENDIF(OS_LINUX)

  SET(COMMON_LIBEV_PROJECT_NAME ${PROJECT_NAME_LOWERCASE}_ev)
  ADD_LIBRARY(${COMMON_LIBEV_PROJECT_NAME} ${LIBEV_HEADERS} ${LIBEV_SOURCES})
  TARGET_INCLUDE_DIRECTORIES(${COMMON_LIBEV_PROJECT_NAME}
    PRIVATE ${LIBEV_INCLUDE_DIRS}
    PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>
  )
  TARGET_LINK_LIBRARIES(${COMMON_LIBEV_PROJECT_NAME} PRIVATE ${LIBEV_LIBRARIES})
  SET(COMMON_INSTALL_LIBS ${COMMON_LIBEV_PROJECT_NAME} ${COMMON_INSTALL_LIBS})
  INSTALL(TARGETS ${COMMON_LIBEV_PROJECT_NAME} DESTINATION ${LIBRARIES_INSTALL_DESTINATION} COMPONENT APPLICATIONS)

  IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")
    STRIP_TARGET(${COMMON_LIBEV_PROJECT_NAME})
  ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")
ENDIF(LIBEV_FOUND)

# common qt
IF(QT_ENABLED)
  SET(COMMON_QT_LIBS ${COMMON_LIBS} ${QT_LIBRARIES})
  SET(COMMON_QT_PROJECT_NAME ${COMMON_PROJECT_NAME}_qt)
  ADD_LIBRARY(${COMMON_QT_PROJECT_NAME} ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES} ${FASTO_QT_MOC_FILES})
  TARGET_INCLUDE_DIRECTORIES(${COMMON_QT_PROJECT_NAME}
    PRIVATE ${PRIVATE_INCLUDE_DIRECTORIES}
    PUBLIC
      $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
      $<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}>)
  TARGET_LINK_LIBRARIES(${COMMON_QT_PROJECT_NAME} PRIVATE ${COMMON_QT_LIBS})
  SET(COMMON_INSTALL_LIBS ${COMMON_QT_PROJECT_NAME} ${COMMON_INSTALL_LIBS})
  INSTALL(TARGETS ${COMMON_QT_PROJECT_NAME} DESTINATION ${LIBRARIES_INSTALL_DESTINATION} COMPONENT APPLICATIONS)

  IF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")
    STRIP_TARGET(${COMMON_QT_PROJECT_NAME})
  ENDIF(PROJECT_BUILD_TYPE_VERSION STREQUAL "release" AND CMAKE_BUILD_TYPE_STABLE MATCHES "RELEASE")
ENDIF(QT_ENABLED)

IF (DEVELOPER_CHECK_STYLE)
  SET(CHECK_SOURCES
    ${COMMON_HEADERS} ${COMMON_SOURCES}
    ${LIBEV_HEADERS} ${LIBEV_SOURCES}
    ${COMMON_QT_HEADERS} ${COMMON_QT_SOURCES}
  )
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
  SET(UNIT_TESTS_DEFINITIONS -DPROJECT_TEST_SOURCES_DIR="${CMAKE_SOURCE_DIR}/tests")
  SET(UNIT_TESTS_LIBRARIES)

  FIND_PACKAGE(OpenSSL)
  IF(OPENSSL_FOUND)
    SET(UNIT_TESTS_DEFINITIONS ${UNIT_TESTS_DEFINITIONS} -DHAVE_OPENSSL)
    SET(UNIT_TESTS_LIBRARIES ${UNIT_TESTS_LIBRARIES} ${OPENSSL_LIBRARIES})
  ENDIF(OPENSSL_FOUND)

  SET(UNIT_TESTS_PROJECT_NAME ${COMMON_PROJECT_NAME}_unit_tests)
  SET(UNIT_TESTS_SOURCES
    ${CMAKE_SOURCE_DIR}/tests/unit_test_error.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_url.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_path.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_filesystem_path.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_filesystem.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_http.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_net.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_media.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_strings.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_text_edecoders.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_value.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_logger.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_threads.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_hash.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_bounded_value.cpp
    ${CMAKE_SOURCE_DIR}/tests/unit_test_license.cpp
  )

  IF(CPUID_FOUND)
    SET(UNIT_TESTS_SOURCES ${UNIT_TESTS_SOURCES} ${CMAKE_SOURCE_DIR}/tests/unit_test_cpu.cpp)
  ENDIF(CPUID_FOUND)

  IF(JSON_ENABLED)
    SET(UNIT_TESTS_SOURCES ${UNIT_TESTS_SOURCES} ${CMAKE_SOURCE_DIR}/tests/unit_test_json_rpc.cpp)
  ENDIF(JSON_ENABLED)

  IF(QT_ENABLED)
    SET(UNIT_TESTS_SOURCES ${UNIT_TESTS_SOURCES} ${CMAKE_SOURCE_DIR}/tests/unit_test_qt.cpp)
  ENDIF(QT_ENABLED)

  IF(LIBEV_FOUND)
    SET(UNIT_TESTS_SOURCES ${UNIT_TESTS_SOURCES}
      ${CMAKE_SOURCE_DIR}/tests/unit_test_libev.cpp
      ${CMAKE_SOURCE_DIR}/tests/unit_test_websockets.cpp
    )
  ENDIF(LIBEV_FOUND)
  ADD_EXECUTABLE(${UNIT_TESTS_PROJECT_NAME} ${UNIT_TESTS_SOURCES})
  TARGET_COMPILE_DEFINITIONS(${UNIT_TESTS_PROJECT_NAME} PRIVATE ${UNIT_TESTS_DEFINITIONS})
  TARGET_LINK_LIBRARIES(${UNIT_TESTS_PROJECT_NAME} ${GTEST_BOTH_LIBRARIES} ${COMMON_INSTALL_LIBS} ${UNIT_TESTS_LIBRARIES})

  ADD_TEST_TARGET(${UNIT_TESTS_PROJECT_NAME})
  SET_PROPERTY(TARGET ${UNIT_TESTS_PROJECT_NAME} PROPERTY FOLDER "Unit tests")
ENDIF(DEVELOPER_ENABLE_TESTS)
