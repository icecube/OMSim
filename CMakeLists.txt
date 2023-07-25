# Minimum version of CMake required
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Project name
project(OMSim_xyz)

# Option to build example with Geant4 UI and Vis drivers
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)

# Find Geant4 and set required variables
if(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED ui_all vis_all)
else()
    find_package(Geant4 REQUIRED)
endif()


# Set path to Geant4 libraries
set (CMAKE_MODULE_PATH /data/geant4/geant4-v11.1.1/install/lib/Geant4-10.7.1/Modules/)

# Set paths for OpenSSL
set(OPENSSL_ROOT_DIR /usr/lib/x86_64-linux-gnu)
set(OPENSSL_LIBRARIES /usr/lib/x86_64-linux-gnu)
find_package(OpenSSL REQUIRED)

# Include OpenSSL's headers in the project
include_directories(${OPENSSL_INCLUDE_DIR})

# Find Boost library and specify which components to use
find_package(Boost REQUIRED COMPONENTS program_options)

# Include Geant4 headers in the project
include(${Geant4_USE_FILE})

# Find ROOT and include its headers in the project
find_package(ROOT)
include(${ROOT_USE_FILE})

# Create lists of source and header files for each executable
file(GLOB COMMON_SOURCES "${PROJECT_SOURCE_DIR}/common/src/*.cc")
file(GLOB COMMON_HEADERS "${PROJECT_SOURCE_DIR}/common/include/*.hh")

# Add the executables
file(GLOB EFFECTIVE_AREA_SOURCES "${PROJECT_SOURCE_DIR}/effective_area/src/*.cc")
file(GLOB RADIOACTIVE_DECAYS_SOURCES "${PROJECT_SOURCE_DIR}/radioactive_decays/src/*.cc")

add_executable(OMSim_effective_area OMSim_effective_area.cc ${COMMON_SOURCES} ${EFFECTIVE_AREA_SOURCES})
#add_executable(OMSim_radioactive_decays OMSim_radioactive_decays.cc ${COMMON_SOURCES} ${RADIOACTIVE_DECAYS_SOURCES})

# Specify include directories for each executable
target_include_directories(OMSim_effective_area PUBLIC 
    ${PROJECT_SOURCE_DIR}/common/include
    ${PROJECT_SOURCE_DIR}/effective_area/include
)

# target_include_directories(OMSim_radioactive_decays PUBLIC 
#     ${PROJECT_SOURCE_DIR}/common/include
#     ${PROJECT_SOURCE_DIR}/radioactive_decays/include
# )

# Link the libraries
target_link_libraries(OMSim_effective_area ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options $<$<BOOL:${MINGW}>:ws2_32>)
#target_link_libraries(OMSim_radioactive_decays ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options $<$<BOOL:${MINGW}>:ws2_32>)

# Copy auxiliary files from source directory to binary directory
set(mdom_aux
    aux/vis.mac
    aux/vis.ogl
)

foreach(_script ${mdom_aux})
    configure_file(
        ${PROJECT_SOURCE_DIR}/${_script}
        ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
