# Minimum version of CMake required
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Project name
project(OMSim)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

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

# For logging with spdlog
find_package(spdlog REQUIRED)
find_package(fmt REQUIRED) # Find fmt library


# Include OpenSSL's headers in the project
include_directories(${OPENSSL_INCLUDE_DIR})

# Find Boost library and specify which components to use
find_package(Boost REQUIRED COMPONENTS program_options)

# Include Geant4 headers in the project
include(${Geant4_USE_FILE})

# Find ROOT and include its headers in the project
find_package(ROOT)
include(${ROOT_USE_FILE})

# Include the subdirectories

add_subdirectory(common)
add_subdirectory(effective_area)
#add_subdirectory(radioactive_decays)
#add_subdirectory(supernova)

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
