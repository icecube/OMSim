# Create lists of source files
file(GLOB QE_CALI_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Create an executable
add_executable(OMSim_efficiency_calibration ${CMAKE_CURRENT_SOURCE_DIR}/OMSim_efficiency_calibration.cc ${COMMON_SOURCES} ${QE_CALI_SOURCES})

# Specify include directories for this executable
target_include_directories(OMSim_efficiency_calibration PUBLIC
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(OMSim_efficiency_calibration ${COMMON_LIBRARIES})
