# Create lists of source files
file(GLOB DECAY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Create an executable
add_executable(OMSim_scintillation_yield ${CMAKE_CURRENT_SOURCE_DIR}/OMSim_scintillation_yield.cc ${COMMON_SOURCES} ${DECAY_SOURCES})

# Specify include directories for this executable
target_include_directories(OMSim_scintillation_yield PUBLIC
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(OMSim_scintillation_yield ${COMMON_LIBRARIES})
