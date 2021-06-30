cmake_minimum_required(VERSION 3.2.3)

PROJECT("libbtbb")

INCLUDE(ExternalProject)

# Python is used for some test scripts.
FIND_PACKAGE(PythonInterp 2.7 REQUIRED)

SET(VERSION_EXTERNAL "2020-12-R1")


#----------------------------------------------------------------------------
#
# Build the project.
#

SET(PRJ_CMAKE_ARGS "")

LIST(APPEND PRJ_CMAKE_ARGS "-DBUILD_SHARED_LIB=OFF")
LIST(APPEND PRJ_CMAKE_ARGS "-DBUILD_STATIC_LIB=ON")
LIST(APPEND PRJ_CMAKE_ARGS "-DENABLE_PYTHON=OFF")

LIST(APPEND PRJ_CMAKE_ARGS "-DPRJ_DIR=${PRJ_DIR}")
LIST(APPEND PRJ_CMAKE_ARGS "-DWORKING_DIR=${WORKING_DIR}")
LIST(APPEND PRJ_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

ExternalProject_Add(TARGET_libbtbb
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/libbtbb-${VERSION_EXTERNAL}
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/libbtbb-${VERSION_EXTERNAL}.tar.gz
                    URL_HASH SHA1=b4da5731b0fba74b69a7346ecd7afc77bb92495a
                    PATCH_COMMAND "${PYTHON_EXECUTABLE}" ${CMAKE_HOME_DIRECTORY}/cmake/tools/apply_patches.py --working-folder ${CMAKE_CURRENT_BINARY_DIR}/libbtbb-${VERSION_EXTERNAL}/src/TARGET_libbtbb --patch-folder ${CMAKE_CURRENT_SOURCE_DIR}/patches --strip 1
                    CMAKE_ARGS ${PRJ_CMAKE_ARGS}
                    INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/install
)
# libbtbb depends on ...
#ADD_DEPENDENCIES(TARGET_libftdi1 TARGET_libusb)
