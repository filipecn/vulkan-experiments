include(ExternalProject)
ExternalProject_Add(
        ponos PREFIX ponos
        URL "https://github.com/filipecn/ponos/archive/master.zip"
        # URL_HASH SHA1=fe17a0610a239311a726ecabcd2dbd669fb24ca8
        CMAKE_ARGS
        "-DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>"
        "-DCMAKE_BUILD_TYPE=Release"
        CMAKE_CACHE_ARGS
        "-DCMAKE_C_COMPILER:FILEPATH=${CMAKE_C_COMPILER}"
        "-DCMAKE_CXX_COMPILER:FILEPATH=${CMAKE_CXX_COMPILER}"
)

ExternalProject_Get_Property(ponos INSTALL_DIR)
set(PONOS_INCLUDE_DIR
        ${INSTALL_DIR}/src/ponos-build/include
        )
set(PONOS_LIBRARIES
        ${INSTALL_DIR}/src/ponos-build/lib/${CMAKE_STATIC_LIBRARY_PREFIX}ponos${CMAKE_STATIC_LIBRARY_SUFFIX}
        )

set(PONOS_INCLUDE_DIR ${PONOS_INCLUDE_DIR} CACHE STRING "")
set(PONOS_LIBRARIES ${PONOS_LIBRARIES} CACHE STRING "")