cmake_minimum_required(VERSION 2.8.3)
project(sri_force_sensor_driver)

## Compile as C++11, supported in ROS Kinetic and newer
#  add_compile_options(-std=c++11)
# add_compile_options(-Wno-narrowing)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -pthread")

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  geometry_msgs
  roscpp
  rospy
  std_msgs
  tf
)

set(SOURCES
    src/sriSerialManager_linux.cpp
    src/sriSerial_linux.cpp
    src/sriCommManager.cpp
    src/sriCommParser.cpp
    src/sriCommM8218Parser.cpp
    src/sriCommCircularBuffer.cpp
    src/sriCommATParser.cpp
    src/filters.cpp
)
catkin_package(
    INCLUDE_DIRS include
    LIBRARIES ${PROJECT_NAME}
    CATKIN_DEPENDS
    std_msgs
)

include_directories(
  include
  ${catkin_INCLUDE_DIRS}
)

add_library(${PROJECT_NAME}
${SOURCES}
)
target_link_libraries(${PROJECT_NAME}
    ${catkin_LIBRARIES}
    )
add_dependencies(${PROJECT_NAME}
        ${${PROJECT_NAME}_EXPORTED_TARGETS}
        ${catkin_EXPORTED_TARGETS}
        )


add_executable(force_sensor_node src/sriRDSerialDemo.cpp)
# add_executable(force_sensor_node src/sriRDSerialDemo.cpp)
target_link_libraries(force_sensor_node ${PROJECT_NAME} ${catkin_LIBRARIES})

install(TARGETS ${PROJECT_NAME}
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        )

# Mark cpp header files for installation
install(DIRECTORY include/${PROJECT_NAME}/
        DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
        FILES_MATCHING PATTERN "*.h"
        )