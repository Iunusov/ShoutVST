project (fluid-gui)

set (CMAKE_CXX_STANDARD 11)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/ SOURCES)

add_library(${PROJECT_NAME}  STATIC ${SOURCES})

target_include_directories(${PROJECT_NAME} 
PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/ ${CMAKE_CURRENT_SOURCE_DIR}/../deps/FLTK ${CMAKE_CURRENT_BINARY_DIR}/../deps/FLTK)

TARGET_LINK_LIBRARIES(${PROJECT_NAME} fltk)

set_property(TARGET ${PROJECT_NAME} PROPERTY POSITION_INDEPENDENT_CODE ON)
