cmake_minimum_required(VERSION 3.0)

project (ShoutVST_Solution)

set(CMAKE_BUILD_TYPE Release)

if((${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC))
  set(MYFLAGS "/O2 /WX- /MT")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MYFLAGS}")
  set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${MYFLAGS}")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MYFLAGS}")
  set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${MYFLAGS}")
  include(CheckSymbolExists)
  check_symbol_exists(snprintf "stdio.h" HAVE_SNPRINTF)
  if(NOT HAVE_SNPRINTF)
  add_definitions(-Dsnprintf=_snprintf)
  endif()
endif()

IF(CMAKE_COMPILER_IS_GNUCC)
	set(MYFLAGS "-Ofast -Wl,--no-undefined")
    set(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${MYFLAGS}")
    set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${MYFLAGS}")
	set(CMAKE_C_FLAGS_RELEASE  "${CMAKE_C_FLAGS_RELEASE} ${MYFLAGS}")
    set(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} ${MYFLAGS}")
ENDIF(CMAKE_COMPILER_IS_GNUCC)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/comp_definitions.h.in ${CMAKE_CURRENT_BINARY_DIR}/comp_definitions.h @ONLY)
include_directories (${CMAKE_CURRENT_BINARY_DIR})

#some properties
SET(WITH_OGG ON CACHE BOOL "libshout: WITH_OGG option")
SET(WITH_VORBIS ON CACHE BOOL "libshout: WITH_VORBIS option")

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/deps)

set_property(TARGET fltk PROPERTY POSITION_INDEPENDENT_CODE ON)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/FLUID)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CMake)
