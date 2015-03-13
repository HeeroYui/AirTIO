cmake_minimum_required(VERSION 2.8.3)
project(river)

set(CMAKE_VERBOSE_MAKEFILE ON)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  drain
  audio
  airtaudio
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
  CATKIN_DEPENDS airtaudio audio drain ejson
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
  ../${PROJECT_NAME}/debug.cpp
  ../${PROJECT_NAME}/river.cpp
  ../${PROJECT_NAME}/Manager.cpp
  ../${PROJECT_NAME}/Interface.cpp
  ../${PROJECT_NAME}/CircularBuffer.cpp
  ../${PROJECT_NAME}/io/Group.cpp
  ../${PROJECT_NAME}/io/Node.cpp
  ../${PROJECT_NAME}/io/NodeAirTAudio.cpp
  ../${PROJECT_NAME}/io/NodePortAudio.cpp
  ../${PROJECT_NAME}/io/NodeAEC.cpp
  ../${PROJECT_NAME}/io/NodeMuxer.cpp
  ../${PROJECT_NAME}/io/Manager.cpp
)

set(CMAKE_CXX_FLAGS "-std=c++11 -DDEBUG_LEVEL=3 -DDEBUG=1 -D__CPP_VERSION__=2011")

add_definitions(-D__AIRTAUDIO_INFERFACE__)

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
install(DIRECTORY ../${PROJECT_NAME}/
  DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
  FILES_MATCHING PATTERN "*.h"
)
