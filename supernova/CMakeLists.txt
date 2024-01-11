# Create lists of source files
file(GLOB SN_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

## Create an executable
add_executable(OMSim_supernova ${CMAKE_CURRENT_SOURCE_DIR}/OMSim_supernova.cc ${COMMON_SOURCES} ${SN_SOURCES})
#
## Specify include directories for this executable
target_include_directories(OMSim_supernova PUBLIC
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
#target_link_libraries(OMSim_radioactive_decays ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options $<$<BOOL:${MINGW}>:ws2_32>)
target_link_libraries(OMSim_supernova ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options spdlog::spdlog fmt::fmt $<$<BOOL:${MINGW}>:ws2_32>)
