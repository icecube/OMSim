# CMakeLists.txt for WavePID simulation study
# This study uses enhanced photon origin tracking

# Get the COMMON_SOURCES from parent scope
set(WAVEPID_COMMON_SOURCES ${COMMON_SOURCES})

# Filter out files we're replacing with our enhanced versions
list(FILTER WAVEPID_COMMON_SOURCES EXCLUDE REGEX ".*OMSimTrackingAction\\.cc$")
list(FILTER WAVEPID_COMMON_SOURCES EXCLUDE REGEX ".*OMSimSensitiveDetector\\.cc$")
list(FILTER WAVEPID_COMMON_SOURCES EXCLUDE REGEX ".*OMSimHitManager\\.cc$")
list(FILTER WAVEPID_COMMON_SOURCES EXCLUDE REGEX ".*OMSim\\.cc$")
list(FILTER WAVEPID_COMMON_SOURCES EXCLUDE REGEX ".*OMSimActionInitialization\\.cc$")

# Create list of WavePID-specific source files (our enhanced versions)
file(GLOB WAVEPID_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Create the executable
add_executable(OMSim_WavePID_study
    ${CMAKE_CURRENT_SOURCE_DIR}/OMSim_WavePID_study.cc
    ${WAVEPID_COMMON_SOURCES}
    ${WAVEPID_SOURCES}
)

# Specify include directories - our local includes take priority
target_include_directories(OMSim_WavePID_study PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
)

# Link the libraries
target_link_libraries(OMSim_WavePID_study ${COMMON_LIBRARIES})
