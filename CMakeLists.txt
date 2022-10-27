cmake_minimum_required(VERSION 3.0.0)

project(libbootenv VERSION 1.0.1 LANGUAGES C)

set(DESCRIPTION "AMLOGIC bootloader environment access library.")
set(DEFAULT_BUILD_TYPE "Release")

if(NOT DEFINED BUILD_SHARED_LIBS)
	option(BUILD_SHARED_LIBS "Build a shared library form of libsparse." ON)
endif()


if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	message(STATUS "Setting build type to '${DEFAULT_BUILD_TYPE}' as none was specified.")
	set(CMAKE_BUILD_TYPE "${DEFAULT_BUILD_TYPE}" CACHE STRING "Choose the type of build." FORCE)
	set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/")

# Dependencies
find_package(PkgConfig REQUIRED)
find_package(ZLIB REQUIRED zlib)
set(PKGCONF_REQ_PUB "zlib")

find_package(TINYALSA REQUIRED tinyalsa)
set(PKGCONF_ALSA_REQ_PUB "tinyalsa")

include(GNUInstallDirs)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Section: libbootenv
set(LIBBOOTENV_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ubootenv.c)
set(LIBBOOTENV_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ubootenv.h ${CMAKE_CURRENT_SOURCE_DIR}/mfrPowerMgr.h)
add_library(bootenv SHARED ${LIBBOOTENV_SRC_FILES})
target_link_libraries(bootenv ${ZLIB_LIBRARIES})
set_target_properties(bootenv PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION})

# Section: libavctrl
set(PROJECT_AVCTRL_NAME "AVcontrol")
set(PROJECT_AVCTRL_DESCRIPTION "Amlogic Audio & Video control")
set(PROJECT_AVCTRL_VERSION 1.0.0)
set(LIBAVCTRL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/AVControl.c)
set(LIBAVCTRL_HEADER_FILES ${CMAKE_CURRENT_SOURCE_DIR}/mfrAVCtrl.h)
add_library(avctrl SHARED ${LIBAVCTRL_SRC_FILES})
target_link_libraries(avctrl ${TINYALSA_LIBRARIES})
set_target_properties(avctrl PROPERTIES
    VERSION ${PROJECT_AVCTRL_VERSION}
    SOVERSION ${PROJECT_AVCTRL_VERSION})

# Section: uenvTool
set(UENVTOOL_SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/ubootenvtest.c)
add_executable(uenvTool ${UENVTOOL_SRC_FILES})
target_link_libraries(uenvTool bootenv avctrl ${TINYALSA_LIBRARIES})

# Section: kBootNotifier
add_executable(kBootNotifier ${CMAKE_CURRENT_SOURCE_DIR}/kBootNotifier.c)

# Install products
install(TARGETS bootenv uenvTool avctrl kBootNotifier
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(FILES ${LIBBOOTENV_HEADER_FILES} ${LIBAVCTRL_HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Install pkg-config files
configure_file(libpwrmgr.pc.in libpwrmgr.pc @ONLY)
configure_file(avctrl.pc.in avctrl.pc @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/libpwrmgr.pc ${CMAKE_BINARY_DIR}/avctrl.pc
	DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pkgconfig)
