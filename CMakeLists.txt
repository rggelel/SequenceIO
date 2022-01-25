CMAKE_MINIMUM_REQUIRED(VERSION 3.5)

project( SequenceIO )

#----------------------------------------
# Tell the compiler to use c++11
set( CMAKE_CXX_STANDARD 11 )
#----------------------------------------

#-----------------------------------------------------------------------------
# GIT - Let's check if a valid version of GIT is available
#-----------------------------------------------------------------------------
OPTION(USE_GIT_PROTOCOL "If behind a firewall turn this off to use https instead." OFF)

SET(GIT_PROTOCOL "git")
IF(NOT USE_GIT_PROTOCOL)
   SET(GIT_PROTOCOL "https")
ENDIF()

#==================================================================
# Look for VTK, only using vtkTransform, so any version should work
#==================================================================
find_package(VTK REQUIRED)

#==================================================================
# Look for ITK, version should be > 4.12
#==================================================================
find_package( ITK REQUIRED )
include( ${ITK_USE_FILE} )


#==================================================================
# Define sources
#==================================================================

set( SRC_files
        SequenceIO.cpp 
	usacquisitionobject.cpp)

set( HDR_files 
        SequenceIO.h
        usacquisitionobject.h)

#================================
# Create custom commands to
# C string literal in a header
# file.
#================================
# Find vtkEncodeString utility executable. Look in built or installed dir structure
if( ${VTK_MAJOR_VERSION} LESS "9" )

  include(${VTK_USE_FILE})
  set( vtkEncodeStringExeFile vtkEncodeString-${VTK_MAJOR_VERSION}.${VTK_MINOR_VERSION} )
  find_program( VTK_ENCODESTRING_EXE ${vtkEncodeStringExeFile} PATHS
                ${VTK_DIR}/bin/
                ${VTK_DIR}/bin/Debug/
                ${VTK_DIR}/bin/Release/
                ${VTK_INSTALL_PREFIX}/bin
                NO_DEFAULT_PATH )
  if( NOT VTK_ENCODESTRING_EXE )
      message( FATAL_ERROR "the vtkEncodeString executable could not be found in the VTK directory.")
  endif()

#================================
# Define output
#================================
set( libName SequenceIOLib )
include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_library( ${libName} ${SRC_files} ${HDR_files})
target_link_libraries( ${libName} ${ITK_LIBRARIES}  ${VTK_LIBRARIES} )
target_include_directories( ${libName} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} )

# Build vtkMNI utility lib (only needed by the executable)
add_subdirectory(utils)

add_executable( SequenceIO main.cpp )
target_link_libraries( SequenceIO ${libName} vtkMNI )

#================================
# Define tests
#================================

option(BUILD_TEST "Enable testing for SequenceIO." ON)

if(BUILD_TEST)
   include(ExternalProject)
   enable_testing()
   add_subdirectory(tests)
endif()