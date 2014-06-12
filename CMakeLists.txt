#************************************************************************
#
# CMakeLists.txt
# 
# Anar Manafov A.Manafov@gsi.de
# 
#
#        version number:    $LastChangedRevision$
#        created by:        Anar Manafov
#                           2010-11-03
#        last changed by:   $LastChangedBy$ $LastChangedDate$
#
#        Copyright (c) 2010 GSI, Scientific Computing group. All rights reserved.
#*************************************************************************
project( pod-web-stat )

cmake_minimum_required( VERSION 2.6 FATAL_ERROR )

#
# Where to lookup modules
#
set (CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}")

#
# configure files
# 
#configure_file( ${PROJECT_SOURCE_DIR}/src/version.h.in ${PROJECT_BINARY_DIR}/src/version.h @ONLY )
#
# Source files
#
set( SOURCE_FILES
    ./src/main.cpp 
    ./src/MonthlyDownloads.cpp
    ./src/DownloadsByCountry.cpp
    ./src/DownloadByVersion.cpp)
set( SRC_HDRS
     ./src/MonthlyDownloads.h
     ./src/DownloadsByCountry.h
     ./src/DownloadByVersion.h)


find_package(Boost COMPONENTS  program_options)

include_directories(
    ${PROJECT_BINARY_DIR}/src
    ${PROJECT_SOURCE_DIR}/Include
    ${Boost_INCLUDE_DIRS} )

#
# pod-web-stat executable
#
if (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-web-stat ${SOURCE_FILES} ${SRC_HDRS})
else (CMAKE_GENERATOR STREQUAL "Xcode")
    add_executable(pod-web-stat ${SOURCE_FILES})
endif (CMAKE_GENERATOR STREQUAL "Xcode")


target_link_libraries (
    pod-web-stat
    ${Boost_PROGRAM_OPTIONS_LIBRARY}
)

install(TARGETS pod-web-stat DESTINATION bin)

