# - Find CppUnit
# Find the CppUnit libraries
#
#  This module defines the following variables:
#     CPPUNIT_FOUND    - True if CPPUNIT_INCLUDE_DIR & CPPUNIT_LIBRARY are found
#     CPPUNIT_LIBRARIES - Set when CPPUNIT_LIBRARY is found
#     CPPUNIT_INCLUDE_DIRS - Set when CPPUNIT_INCLUDE_DIR is found
#
#     CPPUNIT_INCLUDE_DIR - where to find asoundlib.h, etc.
#     CPPUNIT_LIBRARY  - the CppUnit library
#

#=============================================================================
# Adapted from FindALSA.cmake by Remi Rampin, 2014.
#=============================================================================
# Copyright 2009-2011 Kitware, Inc.
# Copyright 2009-2011 Philip Lowman <philip@yhbt.com>
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================
# (To distribute this file outside of CMake, substitute the full
#  License text for the above reference.)

find_path(CPPUNIT_INCLUDE_DIR NAMES cppunit/ui/text/TestRunner.h
          DOC "The CppUnit include directory"
)

find_library(CPPUNIT_LIBRARY NAMES cppunit
          DOC "The CppUnit library"
)

# handle the QUIETLY and REQUIRED arguments and set CPPUNIT_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CppUnit
                                  REQUIRED_VARS CPPUNIT_LIBRARY CPPUNIT_INCLUDE_DIR)

if(CPPUNIT_FOUND)
  set( CPPUNIT_LIBRARIES ${CPPUNIT_LIBRARY} )
  set( CPPUNIT_INCLUDE_DIRS ${CPPUNIT_INCLUDE_DIR} )
endif()

mark_as_advanced(CPPUNIT_INCLUDE_DIR CPPUNIT_LIBRARY)
