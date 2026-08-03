find_package(PkgConfig)

PKG_CHECK_MODULES(PC_GR_BB60C gnuradio-bb60c)

FIND_PATH(
    GR_BB60C_INCLUDE_DIRS
    NAMES gnuradio/bb60c/api.h
    HINTS $ENV{BB60C_DIR}/include
        ${PC_BB60C_INCLUDEDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/include
          /usr/local/include
          /usr/include
)

FIND_LIBRARY(
    GR_BB60C_LIBRARIES
    NAMES gnuradio-bb60c
    HINTS $ENV{BB60C_DIR}/lib
        ${PC_BB60C_LIBDIR}
    PATHS ${CMAKE_INSTALL_PREFIX}/lib
          ${CMAKE_INSTALL_PREFIX}/lib64
          /usr/local/lib
          /usr/local/lib64
          /usr/lib
          /usr/lib64
          )

include("${CMAKE_CURRENT_LIST_DIR}/gnuradio-bb60cTarget.cmake")

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(GR_BB60C DEFAULT_MSG GR_BB60C_LIBRARIES GR_BB60C_INCLUDE_DIRS)
MARK_AS_ADVANCED(GR_BB60C_LIBRARIES GR_BB60C_INCLUDE_DIRS)
