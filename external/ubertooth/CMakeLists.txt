cmake_minimum_required(VERSION 3.2.3)

PROJECT("libubertooth")

INCLUDE(ExternalProject)

SET(VERSION_EXTERNAL "2020-12-R1")


#----------------------------------------------------------------------------
#
# Build the project.
#

SET(PRJ_CMAKE_ARGS "")

LIST(APPEND PRJ_CMAKE_ARGS "-DBUILD_SHARED_LIB=OFF")
LIST(APPEND PRJ_CMAKE_ARGS "-DBUILD_STATIC_LIB=ON")
LIST(APPEND PRJ_CMAKE_ARGS "-DENABLE_PYTHON=OFF")

# Set the location of libbtbb.
LIST(APPEND PRJ_CMAKE_ARGS "-DLIBBTBB_LIBRARIES=${CMAKE_BINARY_DIR}/external/libbtbb/install/usr/local/lib/libbtbb.a")
LIST(APPEND PRJ_CMAKE_ARGS "-DLIBBTBB_INCLUDE_DIR=${CMAKE_BINARY_DIR}/external/libbtbb/install/usr/local/include")

# Set the location of libusb.
LIST(APPEND PRJ_CMAKE_ARGS "-DLIBUSB_LIBRARIES=${CMAKE_BINARY_DIR}/external/libusb/install/lib/libusb-1.0.a")
LIST(APPEND PRJ_CMAKE_ARGS "-DLIBUSB_INCLUDE_DIR=${CMAKE_BINARY_DIR}/external/libusb/install/include")

LIST(APPEND PRJ_CMAKE_ARGS "-DPRJ_DIR=${PRJ_DIR}")
LIST(APPEND PRJ_CMAKE_ARGS "-DWORKING_DIR=${WORKING_DIR}")
LIST(APPEND PRJ_CMAKE_ARGS "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}")

ExternalProject_Add(TARGET_libubertooth
                    PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ubertooth-${VERSION_EXTERNAL}
                    URL ${CMAKE_CURRENT_SOURCE_DIR}/ubertooth-${VERSION_EXTERNAL}.tar.xz
                    URL_HASH SHA1=3c4d85100e3db5488f08338267309f9f16df7d03
                    PATCH_COMMAND "${PYTHON_EXE}" ${CMAKE_HOME_DIRECTORY}/cmake/tools/apply_patches.py --working-folder ${CMAKE_CURRENT_BINARY_DIR}/ubertooth-${VERSION_EXTERNAL}/src/TARGET_libubertooth --patch-folder ${CMAKE_CURRENT_SOURCE_DIR}/patches --strip 1
                    CMAKE_ARGS ${PRJ_CMAKE_ARGS}
                    SOURCE_SUBDIR host
                    INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}/install
)
# libubertooth depends on libusb and libbtbb.
ADD_DEPENDENCIES(TARGET_libubertooth TARGET_libusb TARGET_libbtbb)
