# Create lists of source files
file(GLOB DECAY_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc")

# Create an executable
add_executable(OMSim_radioactive_decays ${CMAKE_CURRENT_SOURCE_DIR}/OMSim_radioactive_decays.cc ${COMMON_SOURCES} ${DECAY_SOURCES})

# Specify include directories for this executable
target_include_directories(OMSim_radioactive_decays PUBLIC
    ${PROJECT_SOURCE_DIR}/common/framework/include
    ${PROJECT_SOURCE_DIR}/common/geometry_construction/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Link the libraries
target_link_libraries(OMSim_radioactive_decays ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${OPENSSL_LIBRARIES} /usr/lib/x86_64-linux-gnu/libargtable2.so.0 Boost::program_options spdlog::spdlog fmt::fmt $<$<BOOL:${MINGW}>:ws2_32>)
