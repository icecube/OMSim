# Minimum version of CMake required
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Project name
project(OMSim)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add these lines for debug information
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g")

# Option to build example with Geant4 UI and Vis drivers
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)

# Find Geant4 and set required variables
if(WITH_GEANT4_UIVIS)
    find_package(Geant4 REQUIRED ui_all vis_all)
else()
    find_package(Geant4 REQUIRED)
endif()

add_definitions(-DBOOST_BIND_GLOBAL_PLACEHOLDERS) 

# Set path to Geant4 libraries
list(APPEND CMAKE_MODULE_PATH ${Geant4_DIR}/Modules)

# Set paths for OpenSSL
set(OPENSSL_ROOT_DIR /usr/lib/x86_64-linux-gnu)
set(OPENSSL_LIBRARIES /usr/lib/x86_64-linux-gnu)
find_package(OpenSSL REQUIRED)

# For logging with spdlog
find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
    message(STATUS "spdlog not found. Will download and build it.")
    
    set(SPDLOG_INSTALL_DIR ${CMAKE_BINARY_DIR}/spdlog-install)
    
    ExternalProject_Add(
        spdlog_external
        GIT_REPOSITORY https://github.com/gabime/spdlog.git
        GIT_TAG v1.11.0  # Specify the version you want
        CMAKE_ARGS
            -DCMAKE_INSTALL_PREFIX=${SPDLOG_INSTALL_DIR}
            -DSPDLOG_BUILD_EXAMPLES=OFF
            -DSPDLOG_BUILD_TESTS=OFF
        )
    
    set(spdlog_INCLUDE_DIRS ${SPDLOG_INSTALL_DIR}/include)
    set(spdlog_LIBRARIES ${SPDLOG_INSTALL_DIR}/lib/libspdlog.a)
    
    add_library(spdlog::spdlog STATIC IMPORTED)
    set_target_properties(spdlog::spdlog PROPERTIES
        IMPORTED_LOCATION ${spdlog_LIBRARIES}
        INTERFACE_INCLUDE_DIRECTORIES ${spdlog_INCLUDE_DIRS}
    )
    add_dependencies(spdlog::spdlog spdlog_external)
else()
    message(STATUS "Found spdlog: ${spdlog_VERSION}")
endif()


# Include OpenSSL's headers in the project
include_directories(${OPENSSL_INCLUDE_DIR})

# Find Boost library and specify which components to use
find_package(Boost REQUIRED COMPONENTS program_options)

# Include Geant4 headers in the project
include(${Geant4_USE_FILE})

# Find ROOT and include its headers in the project
find_package(ROOT)
include(${ROOT_USE_FILE})

# Define a variable with all the libraries
set(COMMON_LIBRARIES 
    ${Geant4_LIBRARIES}
    ${ROOT_LIBRARIES}
    ${OPENSSL_LIBRARIES}
    Boost::program_options
    spdlog::spdlog
    $<$<BOOL:${MINGW}>:ws2_32>
)

# Make this variable available to subdirectories
set(COMMON_LIBRARIES ${COMMON_LIBRARIES} CACHE INTERNAL "")

# Include the subdirectories
add_subdirectory(common)
#add_subdirectory(effective_area)
add_subdirectory(QE_calibration)
#add_subdirectory(radioactive_decays)
#add_subdirectory(supernova)
