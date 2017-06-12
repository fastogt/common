CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0)
PROJECT(cpuid)

IF(OS_WINDOWS)
  SET(CONFIG_IN config_win.h.in)
ELSEIF(OS_MACOSX)
  SET(CONFIG_IN config_macos.h.in)
ELSEIF(OS_LINUX)
  IF(PLATFORM_ARM)
    SET(CONFIG_IN config_arm.h.in)
  ELSE()
    SET(CONFIG_IN config_linux.h.in)
  ENDIF()
ELSEIF(OS_FREEBSD)
  SET(CONFIG_IN config_freebsd.h.in)
ELSEIF(OS_ANDROID)
  SET(CONFIG_IN config_android.h.in)
ELSE()
  MESSAGE(FATAL_ERROR "Not have config.in: ${CMAKE_SYSTEM_NAME}")
ENDIF(OS_WINDOWS)

CONFIGURE_FILE(${CONFIG_IN} ${CMAKE_CURRENT_LIST_DIR}/libcpuid/libcpuid/config.h @ONLY IMMEDIATE)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_LIST_DIR}/libcpuid/src)

SET(HEADERS_CPUID
  ${CMAKE_CURRENT_LIST_DIR}/libcpuid/libcpuid/config.h
  libcpuid/libcpuid/asm-bits.h
  libcpuid/libcpuid/libcpuid_constants.h
  libcpuid/libcpuid/libcpuid.h
  libcpuid/libcpuid/libcpuid_types.h
  libcpuid/libcpuid/libcpuid_util.h
  libcpuid/libcpuid/rdtsc.h
  libcpuid/libcpuid/recog_amd.h
  libcpuid/libcpuid/recog_intel.h
)

SET(SOURCES_CPUID
  libcpuid/libcpuid/asm-bits.c
  libcpuid/libcpuid/cpuid_main.c
  #libcpuid/libcpuid/embed_drivers.cpp
  libcpuid/libcpuid/libcpuid_util.c
  libcpuid/libcpuid/msrdriver.c
  libcpuid/libcpuid/rdmsr.c
  libcpuid/libcpuid/rdtsc.c
  libcpuid/libcpuid/recog_amd.c
  libcpuid/libcpuid/recog_intel.c
)

SET(LIBS_CPUID

)

ADD_LIBRARY(${PROJECT_NAME} OBJECT ${HEADERS_CPUID} ${SOURCES_CPUID})
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${LIBS_CPUID})
