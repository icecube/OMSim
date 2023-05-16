# Find the manual here:
# http://geant4.web.cern.ch/geant4/UserDocumentation/UsersGuides/InstallationGuide/html/ch03s02.html

# check for cmake version, initialize project -> sets name and paths
cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
project(OMSim_xyz)

# choose option
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)

# locate geant4 and set all required variables
if(WITH_GEANT4_UIVIS)
	find_package(Geant4 REQUIRED ui_all vis_all)
else()
	find_package(Geant4 REQUIRED)
endif()

find_package(spdlog REQUIRED)

# location of Geant4 libraries
set (CMAKE_MODULE_PATH /data/geant4/geant4.10.7_ubuntu_20.04/install/lib/Geant4-10.7.1/Modules/)


set(OPENSSL_ROOT_DIR /usr/lib/x86_64-linux-gnu)
set(OPENSSL_LIBRARIES /usr/lib/x86_64-linux-gnu)

find_package(OpenSSL REQUIRED)
include_directories(${OPENSSL_INCLUDE_DIR})

# Find the Boost library
find_package(Boost REQUIRED COMPONENTS program_options)

# sets search paths for include
include(${Geant4_USE_FILE})

find_package(ROOT)
include(${ROOT_USE_FILE})

include_directories(${PROJECT_SOURCE_DIR}/include)

# create list of all the files to compile
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)


#add_executable(OMSim_xyz OMSim_xyz.cc ${sources})# ${headers})
add_executable(OMSim OMSim.cc ${sources})# ${headers})
#target_link_libraries(OMSim_xyz ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0)
target_link_libraries(OMSim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options spdlog::spdlog $<$<BOOL:${MINGW}>:ws2_32>)


# which auxiliary files out of the source directory should be copied to the directory of the binary
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
