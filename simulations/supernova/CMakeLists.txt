# Create lists of source files
file(GLOB SN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

## Create an executable
add_executable(OMSimSupernova ${CMAKE_CURRENT_SOURCE_DIR}/OMSimSupernova.cc ${COMMON_SOURCES} ${SN_SOURCES})
#
## Specify include directories for this executable
target_include_directories(OMSimSupernova PUBLIC
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(OMSimSupernova ${COMMON_LIBRARIES})
