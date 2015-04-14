cmake_minimum_required(VERSION 2.8.3)
project(audio_river)

set(CMAKE_VERBOSE_MAKEFILE ON)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  audio_drain
  audio
  audio_orchestra
  ejson
  )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if you package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
  INCLUDE_DIRS ../
  LIBRARIES ${PROJECT_NAME}
  CATKIN_DEPENDS audio_orchestra audio audio_drain ejson
  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
  ..
  ${catkin_INCLUDE_DIRS}
)

## Declare a cpp library
add_library(${PROJECT_NAME}
  ../audio/river/debug.cpp
  ../audio/river/river.cpp
  ../audio/river/Manager.cpp
  ../audio/river/Interface.cpp
  ../audio/river/io/Group.cpp
  ../audio/river/io/Node.cpp
  ../audio/river/io/NodeOrchestra.cpp
  ../audio/river/io/NodePortAudio.cpp
  ../audio/river/io/NodeAEC.cpp
  ../audio/river/io/NodeMuxer.cpp
  ../audio/river/io/Manager.cpp
)

add_definitions(-DAUDIO_RIVER_BUILD_ORCHESTRA)

## Specify libraries to link a library or executable target against
target_link_libraries(${PROJECT_NAME}
  ${catkin_LIBRARIES}
)

#############
## Install ##
#############

## Mark executables and/or libraries for installation
install(TARGETS ${PROJECT_NAME}
  ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
  LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
)

## Mark cpp header files for installation
install(DIRECTORY ../audio/river/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
)
