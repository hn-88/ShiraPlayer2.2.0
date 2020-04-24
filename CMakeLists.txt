CMAKE_MINIMUM_REQUIRED(VERSION 2.8.11)

############ Project name ###########
PROJECT(SHIRAPLAYER)
SET(SHIRAPLAYER_MAJOR "1")
SET(SHIRAPLAYER_MINOR "8")
SET(SHIRAPLAYER_PATCH "0")
SET(SHIRAPLAYER_PATCHMIN "2")
SET(VERSION "${SHIRAPLAYER_MAJOR}.${SHIRAPLAYER_MINOR}.${SHIRAPLAYER_PATCH}.${SHIRAPLAYER_PATCHMIN}")
SET(PACKAGE SHIRAPLAYER)

SET(PACKAGE_VERSION "${VERSION}")
ADD_DEFINITIONS(-DPACKAGE_VERSION="${PACKAGE_VERSION}")

SET(MINIMAL_QT_VERSION "5.1.0")


## Find the QtWidgets library
find_package(Qt5 COMPONENTS Widgets Core Gui Xml XmlPatterns Declarative Network Multimedia REQUIRED )
include_directories(${Qt5Widgets_INCLUDE_DIRS})
# I add this test because on Windows with angle we should not link
# with OpenGL.  Maybe there is a better way to do that.
IF("${Qt5Gui_OPENGL_LIBRARIES}" MATCHES "opengl")
    FIND_PACKAGE(OpenGL REQUIRED)
ENDIF()

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# Use customized cmake macros
SET(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)

########### Main global variables ###########
IF(NOT CMAKE_BUILD_TYPE)
  SET(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
ENDIF()

IF(${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  SET(APPLE 1)
ENDIF()

IF(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_COMPILER_IS_GNUCC)
  # The stars structs rely on gnu gcc packing of bit-fields.
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mno-ms-bitfields")
  SET(GCC_VERSION "${CMAKE_CXX_COMPILER_VERSION}")
ELSE()
  SET(GCC_VERSION "0.0")
ENDIF()

IF(WIN32)
  IF(NOT MSVC)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -fexceptions -fident -mthreads -Wpacked")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wsign-promo -fexceptions -fident -mthreads")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wsign-promo -fexceptions -fident -mthreads -mwindows")
  ELSE()
    # GZ: GCC flags unknown: exception handling should be default in MSVC.
    # it seems -fident does not exist any longer. -fno-ident is documented in gcc.
    # -mthreads and -mwindows are MinGW specific.
    # SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fexceptions -fident -mthreads -mwindows")

    # Prevent MSVC from complaining about M_PI no matter where <cmath> is included first
    add_definitions(-D_USE_MATH_DEFINES)
  ENDIF()
ELSE()
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wno-unused-parameter")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-unused-parameter -Wno-unused-result")
ENDIF()

IF("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
 SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Woverloaded-virtual -Wno-unused-private-field -Wno-uninitialized -Wno-tautological-constant-out-of-range-compare")
 # The cosmetic fix for Clang 3.4
 IF(${CMAKE_CXX_COMPILER_VERSION} VERSION_GREATER 3.3)
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-const-variable")
 ENDIF()
ENDIF()


SET(OPTIMIZE_INTEL_ATOM 0 CACHE BOOL "Activate optimizations for atom processor.")
IF(OPTIMIZE_INTEL_ATOM)
 IF (MSVC)
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /arch:SSE2 ")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /arch:SSE2 ")
 ELSE()
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mtune=generic -msse -msse2 -msse3 -mfpmath=sse")
  SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mtune=generic -msse -msse2 -msse3 -mfpmath=sse")
 ENDIF()
ENDIF()

# best way to get to APPLE?
IF(APPLE)
 SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fno-common -Wall -Wextra -Wno-unused-parameter")
 SET(CMAKE_CXX_FLAGS "${CMAKE_C_FLAGS} -mmacosx-version-min=10.4")
 SET(CMAKE_OSX_DEPLOYMENT_TARGET "10.4")
 SET(CMAKE_OSX_SYSROOT "/Developer/SDKs/MacOSX10.4u.sdk/")
 # Universal binary
 #SET(CMAKE_OSX_ARCHITECTURES "i386;ppc")
 SET(CMAKE_OSX_ARCHITECTURES "i386;ppc")
ENDIF()

########### Check headers ###########
INCLUDE(CheckIncludeFiles)
CHECK_INCLUDE_FILES(byteswap.h HAVE_BYTESWAP_H)

########### Check functions ###########
INCLUDE(CheckFunctionExists)
CHECK_FUNCTION_EXISTS(tzset HAVE_TZSET)
CHECK_FUNCTION_EXISTS(pow10 HAVE_POW10)
CHECK_FUNCTION_EXISTS(setlocale HAVE_SETLOCALE)
CHECK_FUNCTION_EXISTS(snprintf HAVE_SNPRINTF)

########### Others ###########
# Activate translation with gettext
SET(RELEASE_BUILD 1 CACHE BOOL "Flag this build as an official release.")
SET(ENABLE_NLS 0 CACHE BOOL "Define whether program translation should be supported.")

# Activate sound support
SET(ENABLE_SOUND 1 CACHE BOOL "Define whether sound support should be activated, options are.")

# (De-)Activate the script edit console
SET(ENABLE_SCRIPT_CONSOLE 1 CACHE BOOL "Define whether to build the script console feature, options are.")
IF(ENABLE_SCRIPT_CONSOLE)
  ADD_DEFINITIONS(-DENABLE_SCRIPT_CONSOLE)
ENDIF()

SET(ENABLE_STRATOSCRIPT_COMPAT 1)# CACHE BOOL "Set to 1 or true if you want to build the Stratoscript compatibility mode")
SET(GUI_MODE Standard CACHE STRING "Choose the type of GUI to build, options are: Standard, None, External")
SET(OPENGL_MODE Desktop CACHE STRING "Choose the type of openGL to use, options are: Desktop, ES2")

SET(GENERATE_PACKAGE_TARGET 1 CACHE BOOL "Set to 1 or true if you want to have make package target")

# On WIN32 we need to split the main binary into a small binary and a dll
# This is for allowing to link dynamic plug-ins afterward
IF(WIN32)
  SET(GENERATE_STELMAINLIB 1)
ELSE()
  SET(GENERATE_STELMAINLIB 0)
ENDIF()

# For Windows, the default back end for the graphics system should be "native", as "raster"
# causes problems for many ATI users.  However, for other platforms we should use raster
# as the default as it is faster.
# Mac 10.6 requires native as well.
IF(WIN32 OR APPLE)
  ADD_DEFINITIONS(-DDEFAULT_GRAPHICS_SYSTEM="native")
ELSE()
  ADD_DEFINITIONS(-DDEFAULT_GRAPHICS_SYSTEM="raster")
ENDIF()

########### Plugin setup #############
SET(BUILD_STATIC_PLUGINS 1 CACHE BOOL "Define whether plugins should be compiled statically and linked to the main program")
SET(BUILD_DYNAMIC_PLUGINS 0 CACHE BOOL "Define whether plugins should be compiled dynamically")

SET(USE_PLUGIN_HELLOSTELMODULE 0 CACHE BOOL "Define whether the HelloStelModule plugin should be created.")
SET(USE_PLUGIN_ANGLEMEASURE 0 CACHE BOOL "Define whether the AngleMeasure plugin should be created.")
SET(USE_PLUGIN_COMPASSMARKS 0 CACHE BOOL "Define whether the CompassMarks plugin should be created.")
SET(USE_PLUGIN_SATELLITES 1 CACHE BOOL "Define whether the Satellites plugin should be created.")
SET(USE_PLUGIN_TELESCOPECONTROL 0 CACHE BOOL "Define whether the TelescopeControl plug-in should be created.")
SET(USE_PLUGIN_LOGBOOK 0 CACHE BOOL "Define whether the LogBook plugin should be created.")
SET(USE_PLUGIN_OCULARS 0 CACHE BOOL "Define whether the Oculars plugin should be created.")
SET(USE_PLUGIN_SVMT 0 CACHE BOOL "Define whether the SVMT plugin should be created.")
SET(USE_PLUGIN_TEXTUSERINTERFACE 1 CACHE BOOL "Define whether the TextUserInterface plugin should be created.")
SET(USE_PLUGIN_VIRGO 0 CACHE BOOL "Define whether the VirGO plugin should be created.")

########## Static plugins need to define includes and libraries
########## for the compilation of Stellarium itself
IF (BUILD_STATIC_PLUGINS)
  IF (USE_PLUGIN_OCULARS)
        SET(QT_USE_QTSQL TRUE)
  ENDIF()

#  IF (USE_PLUGIN_SVMT)
#        ## Tokyo Cabinet
#        FIND_PATH(TOKYOC_INCLUDE_DIR tcadb.h
#          /usr/include
#          /usr/local/include
#          DOC "Should point to the directory where the include files for tokyo cabinet are installed")
#        FIND_LIBRARY(TOKYOC_LIBRARIES tokyocabinet
#          PATHS
#          /usr/lib/
#          /usr/local/lib
#          DOC "tokyo cabinet link options")
#        SET(QT_USE_QTXML TRUE) # For SVMT we also need QtXml module
#        SET(QT_USE_QTDECLARATIVE TRUE) # For SVMT we also need QML module, this line doesn't seem to work right now
#  ENDIF()

#  IF (USE_PLUGIN_VIRGO)
#        ## Library for loading FITS image files
#        FIND_PATH(CFITSIO_INCLUDE_DIR fitsio.h
#          /usr/include
#          /usr/local/include
#          DOC "Should point to the directory where the include files for libcfitsio are installed")
#        FIND_LIBRARY(CFITSIO_LIBRARIES cfitsio
#          PATHS
#          /usr/lib/
#          /usr/local/lib
#          /opt/local/lib
#          DOC "cfitsio link options")
#        SET(QT_USE_QTXML TRUE) # For VirGO we also need QtXml module
#  ENDIF()
ENDIF()

# Custom target used to manage dependencies of SHIRAPLAYER -> Static plugins
# It is important that static plugins are compiled before stellarium main executable is linked
ADD_CUSTOM_TARGET(AllStaticPlugins ALL)



# Get rid of the QtTest dependency for the main binary. It is re-added manually when compiling tests.
STRING(REPLACE "${QT_QTTEST_LIBRARY}" "" QT_LIBRARIES "${QT_LIBRARIES}")
# Get rid of the GLU dependency, it's not needed in stellarium.
STRING(REPLACE "-lGLU " "" QT_LIBRARIES "${QT_LIBRARIES}")

FIND_PACKAGE(ZLIB)

IF(ENABLE_NLS)
  FIND_PACKAGE(ICONV REQUIRED)
  FIND_PATH(INTL_INCLUDE_DIR libintl.h
        PATHS
        /usr/include
        /sw/include
        /usr/local/include)
  FIND_LIBRARY(INTL_LIBRARIES intl c
        PATHS
        /usr/lib/
        /sw/lib
        /usr/local/lib) # NO_DEFAULT_PATH
  FIND_PACKAGE(GETTEXT REQUIRED)
ENDIF()


IF (OPENGL_MODE STREQUAL "Desktop")
  FIND_PACKAGE(OpenGL REQUIRED)
ENDIF()
IF(OPENGL_MODE STREQUAL "ES2")
  ADD_DEFINITIONS(-DUSE_OPENGL_ES2 -DQT_OPENGL_LIB -D_REENTRANT -DQT_GUI_LIB -DQT_CORE_LIB -DQT_SHARED)
  # Make sure that no desktop OpenGL libraries are linked when OpenGLES is used
  STRING(REPLACE "-lGL" "-lGLESv2" QT_LIBRARIES "${QT_LIBRARIES}")
ENDIF()

########### Set some global variables ###########
IF(UNIX AND NOT WIN32)
IF(APPLE)
  SET(CMAKE_INSTALL_PREFIX "${PROJECT_BINARY_DIR}/SHIRAPLAYER.app/Contents")
ELSE(APPLE)
  SET(INSTALL_DATADIR ${CMAKE_INSTALL_PREFIX}/share/SHIRAPLAYER)
  SET(INSTALL_LOCALEDIR ${CMAKE_INSTALL_PREFIX}/share/locale)
ENDIF()
ELSE()
  SET(INSTALL_DATADIR .)
  SET(INSTALL_LOCALEDIR ./locale)
ENDIF()

########### Get SVN revision for non-release builds ###########
#IF(NOT RELEASE_BUILD)
#  FIND_PACKAGE(Subversion)
#  IF(Subversion_FOUND)
#        Subversion_WC_INFO(${PROJECT_SOURCE_DIR} StelSvn)
#        SET(SVN_REVISION ${StelSvn_WC_REVISION})
#  ENDIF()
#ENDIF()

IF(ENABLE_STRATOSCRIPT_COMPAT)
  ADD_DEFINITIONS(-DENABLE_STRATOSCRIPT_COMPAT)
ENDIF()

########### Generate a config.h ###########
SET(PACKAGE_VERSION ${VERSION})
CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/config.h.cmake ${CMAKE_BINARY_DIR}/config.h)
ADD_DEFINITIONS(-DHAVE_CONFIG_H)

### Generate an Inno Setup project file ###
IF(WIN32)
  SET(CONFIGURED_FILE_WARNING "Do not edit this file! It has been automatically generated by CMake. Your changes will be lost the next time CMake is run.")
  CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/SHIRAPLAYER.iss.cmake ${CMAKE_SOURCE_DIR}/SHIRAPLAYER.iss @ONLY)
ENDIF(WIN32)

IF(CMAKE_BUILD_TYPE STREQUAL "Release" OR CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
  ADD_DEFINITIONS(-DQT_NO_DEBUG)
  ADD_DEFINITIONS(-DNDEBUG)
ENDIF()

########### uninstall files ###############
CONFIGURE_FILE(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake_uninstall.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
  IMMEDIATE @ONLY)

ADD_CUSTOM_TARGET(uninstall
  "${CMAKE_COMMAND}" -P "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake")

########### Packaging info for CPack ###########

IF(GENERATE_PACKAGE_TARGET)
  INCLUDE(InstallRequiredSystemLibraries)
  SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SHIRAPLAYER is a free open source planetarium for your computer. It shows a realistic sky in 3D, just like what you see with the naked eye, binoculars or a telescope.")
  SET(CPACK_PACKAGE_VENDOR "SHIRAPLAYER's team")
  SET(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
  SET(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
  SET(CPACK_PACKAGE_VERSION_MAJOR ${SHIRAPLAYER_MAJOR})
  SET(CPACK_PACKAGE_VERSION_MINOR ${SHIRAPLAYER_MINOR})
  SET(CPACK_PACKAGE_VERSION_PATCH ${SHIRAPLAYER_PATCH})
  SET(CPACK_PACKAGE_INSTALL_DIRECTORY "SHIRAPLAYER")
  SET(CPACK_SOURCE_PACKAGE_FILE_NAME "SHIRAPLAYER-${VERSION}")
  SET(CPACK_SOURCE_GENERATOR "TGZ")
  SET(CPACK_GENERATOR "TGZ")
  SET(CPACK_STRIP_FILES "bin/SHIRAPLAYER")
  SET(CPACK_PACKAGE_EXECUTABLES "SHIRAPLAYER" "SHIRAPLAYER")
  SET(CPACK_SOURCE_IGNORE_FILES "/CVS/" "/.svn/" "/.git/" "builds/" "SHIRAPLAYER.tag$" "SHIRAPLAYER.kdevelop.pcs$" "~$" "\\\\.swp$" "\\\\.#" "/#")
  INCLUDE(CPack)
ENDIF()

########### Macosx Bundling ###############
IF(APPLE)
ADD_CUSTOM_TARGET(macosx_bundle bash util/macosx_bundle.sh ${CMAKE_INSTALL_PREFIX} ${PROJECT_SOURCE_DIR} DEPENDS install WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "making the macosx bundle." VERBATIM)
ENDIF()


########### Generate doxygen doc ###############
ADD_CUSTOM_TARGET(apidoc doxygen Doxyfile WORKING_DIRECTORY ${PROJECT_SOURCE_DIR} COMMENT "Generate the doxygen documentation into the builds/doc directory.")


########### Top level include directories ###########
# This will be used for all compilations in sub-directories
INCLUDE_DIRECTORIES(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/gui
        ${CMAKE_SOURCE_DIR}/src/modules
        ${CMAKE_SOURCE_DIR}/src/planetsephems
        ${CMAKE_SOURCE_DIR}/src/external
        ${CMAKE_SOURCE_DIR}/src/scripting
        ${CMAKE_SOURCE_DIR}/src/external/kfilter
        ${CMAKE_SOURCE_DIR}/src/external/glues_stel/source/
        ${CMAKE_SOURCE_DIR}/src/external/glues_stel/source/libtess
        ${OPENGL_INCLUDE_DIR}
        ${ICONV_INCLUDE_DIR}
        ${INTL_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        ${QT_INCLUDE_DIR}/QtScript
        ${QT_QTSCRIPT_INCLUDE_DIR}
        ${CMAKE_BINARY_DIR}/src
        ${QT_QTTEST_INCLUDE_DIR}
        ${QT_DECLARATIVE_INCLUDE_DIR}
        ${FFMPEG_INCLUDE_DIR}
        ${SDL_INCLUDE_DIR}
        ${SERIAL_INCLUDE_DIR}
)

############ Subdirectories ###############
ADD_SUBDIRECTORY( data )
ADD_SUBDIRECTORY( src )
ADD_SUBDIRECTORY( textures )
##ADD_SUBDIRECTORY( doc )
IF(ENABLE_NLS)
  ADD_SUBDIRECTORY( po )
ENDIF()
ADD_SUBDIRECTORY( landscapes )
ADD_SUBDIRECTORY( skycultures )
ADD_SUBDIRECTORY( nebulae )
ADD_SUBDIRECTORY( scripts )
ADD_SUBDIRECTORY( stars )
ADD_SUBDIRECTORY( plugins )


