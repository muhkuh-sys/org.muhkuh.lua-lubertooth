cmake_minimum_required(VERSION 3.7)

PROJECT("lubertooth")

INCLUDE(ExternalProject)

SET(PROJECT_VERSION  "1.0.1")

# Get the VCS version for the jonchki configuration.
INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/version.cmake)

# Filter the jonchki configuration.
CONFIGURE_FILE(installer/lua${BUILDCFG_LUA_VERSION}/lubertooth.xml
               ${CMAKE_BINARY_DIR}/lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}.xml
               @ONLY)

#----------------------------------------------------------------------------
#
# Build the project.
#

IF((${BUILDCFG_ONLY_JONCHKI_CFG} STREQUAL "OFF"))
	# Set some paths.
	SET(LIBUSB_INCLUDE       "${CMAKE_BINARY_DIR}/external/libusb/install/include/")
	SET(LIBUSB_LIB           "${CMAKE_BINARY_DIR}/external/libusb/install/lib/libusb-1.0.a")
	SET(LIBBTBB_INCLUDE      "${CMAKE_BINARY_DIR}/external/libbtbb/install/usr/local/include/")
	SET(LIBBTBB_LIB          "${CMAKE_BINARY_DIR}/external/libbtbb/install/usr/local/lib/libbtbb.a")
	SET(LIBUBERTOOTH_INCLUDE "${CMAKE_BINARY_DIR}/external/ubertooth/install/usr/local/include/")
	SET(LIBUBERTOOTH_LIB     "${CMAKE_BINARY_DIR}/external/ubertooth/install/usr/local/lib/libubertooth.a")

	# Select the LUA version.
	INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/muhkuh_lua.cmake)

	# Enable tests (see here for details: http://www.vtk.org/Wiki/CMake/Testing_With_CTest)
	ENABLE_TESTING()

	# Set some installation folders.
	INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/muhkuh_installation_folders.cmake)

	FIND_PACKAGE(SWIG REQUIRED)

	# Add a custom target for the swig runtime for LUA.
	INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/muhkuh_swig_lua_runtime.cmake)

	# Select the LUA version.
	INCLUDE(${CMAKE_HOME_DIRECTORY}/cmake/muhkuh_lua.cmake)

	INCLUDE(${SWIG_USE_FILE})

	INCLUDE_DIRECTORIES(${LUA_INCLUDE_DIR})
	INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/swig_runtime)
	INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
	INCLUDE_DIRECTORIES(${LIBUSB_INCLUDE})
	INCLUDE_DIRECTORIES(${LIBBTBB_INCLUDE})
	INCLUDE_DIRECTORIES(${LIBUBERTOOTH_INCLUDE})

	SET_SOURCE_FILES_PROPERTIES(lubertooth.i PROPERTIES CPLUSPLUS ON)
	SET_SOURCE_FILES_PROPERTIES(lubertooth.i PROPERTIES SWIG_FLAGS "")
	SWIG_ADD_MODULE(TARGET_lubertooth lua lubertooth.i wrapper.cpp)
	SWIG_LINK_LIBRARIES(TARGET_lubertooth ${LUA_LIBRARIES} ${LIBUBERTOOTH_LIB} ${LIBBTBB_LIB} ${LIBUSB_LIB} -ludev)
	ADD_DEPENDENCIES(TARGET_lubertooth TARGET_swigluarun TARGET_libubertooth TARGET_libusb TARGET_libbtbb)

	# Set the name of the output file to "archive".
	SET_TARGET_PROPERTIES(TARGET_lubertooth PROPERTIES PREFIX "" OUTPUT_NAME "lubertooth")

	#----------------------------------------------------------------------------
	#
	# Build a package for the selected platform.
	#

	SET(CPACK_OUTPUT_FILE_PREFIX ${CMAKE_BINARY_DIR}/../../repository/org/muhkuh/lua/lubertooth/${PROJECT_VERSION})
	SET(CPACK_INCLUDE_TOPLEVEL_DIRECTORY FALSE)
	SET(CPACK_PACKAGE_FILE_NAME "lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}-${JONCHKI_PLATFORM}")

	SET(CPACK_GENERATOR "TXZ")
	INCLUDE(CPack)

	IF(${JONCHKI_PLATFORM_DIST_ID} STREQUAL "windows")
		# Package a windows build.
		CONFIGURE_FILE(installer/lua${BUILDCFG_LUA_VERSION}/install_windows.lua
		               ${CMAKE_CURRENT_BINARY_DIR}/package/install.lua
		               @ONLY)

		INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/package/install.lua
		        DESTINATION .)
	ELSE(${JONCHKI_PLATFORM_DIST_ID} STREQUAL "windows")
		# Package a linux build.
		CONFIGURE_FILE(installer/lua${BUILDCFG_LUA_VERSION}/install_linux.lua
		               ${CMAKE_CURRENT_BINARY_DIR}/package/install.lua
		               @ONLY)

		INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/package/install.lua
		        DESTINATION .)
	ENDIF(${JONCHKI_PLATFORM_DIST_ID} STREQUAL "windows")

	#INSTALL(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/install/lua
	#        DESTINATION .)
	INSTALL(TARGETS TARGET_lubertooth
	        DESTINATION lua_plugins)

	CONFIGURE_FILE(installer/lua${BUILDCFG_LUA_VERSION}/lubertooth.xml
	               ${CPACK_OUTPUT_FILE_PREFIX}/lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}-${JONCHKI_PLATFORM}.xml
	               @ONLY)


	#-----------------------------------------------------------------------------
	#
	# Create a custom target named "pack".
	# It builds the package and creates the hashes.
	#
	ADD_CUSTOM_TARGET(pack
	                  COMMAND ${CMAKE_MAKE_PROGRAM}
	                  COMMAND ${CMAKE_MAKE_PROGRAM} package
	                  COMMAND "${PYTHON_EXE}" ${CMAKE_HOME_DIRECTORY}/cmake/tools/generate_hash.py ${CPACK_OUTPUT_FILE_PREFIX}/lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}-${JONCHKI_PLATFORM}.xml
	                  COMMAND "${PYTHON_EXE}" ${CMAKE_HOME_DIRECTORY}/cmake/tools/generate_hash.py ${CPACK_OUTPUT_FILE_PREFIX}/lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}-${JONCHKI_PLATFORM}.tar.xz
	                  COMMENT "Pack the artifact and generate the hashes...")


	#-----------------------------------------------------------------------------
	#
	# Filter the pom.xml file.
	#
	SET(POM_ARTIFACT_ID  "lubertooth")
	SET(POM_GROUP_ID     "org.muhkuh.lua")
	SET(POM_VERSION      "${PROJECT_VERSION}")
	SET(POM_PACKAGING    "tar.xz")
	CONFIGURE_FILE(installer/lua${BUILDCFG_LUA_VERSION}/pom.xml
	               ${CPACK_OUTPUT_FILE_PREFIX}/lua${BUILDCFG_LUA_VERSION}-lubertooth-${PROJECT_VERSION}.pom
	               @ONLY)
ENDIF((${BUILDCFG_ONLY_JONCHKI_CFG} STREQUAL "OFF"))
