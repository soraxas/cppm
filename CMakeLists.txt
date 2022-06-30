cmake_minimum_required(VERSION 3.11)

project(cppm VERSION 1.0.0)

set(CPPM_HEADERS
    cppm.hpp
)

add_library(cppm INTERFACE ${CPPM_HEADERS})
target_include_directories(cppm INTERFACE .)

set_target_properties(cppm PROPERTIES PUBLIC_HEADER ${CPPM_HEADERS})
INSTALL(TARGETS cppm
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
