project (deps)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libmp3lame-CMAKE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/OGG-Vorbis-CMAKE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/libshout-CMAKE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/VST_SDK_2.4)

SET(OPTION_BUILD_EXAMPLES OFF CACHE BOOL "FLTK: Builds the many fine example programs.")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/FLTK)
