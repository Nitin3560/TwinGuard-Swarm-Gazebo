cmake_minimum_required(VERSION 3.16)
project(twinguard_swarm_nav2_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(behaviortree_cpp_v3 REQUIRED)
find_package(nav2_costmap_2d REQUIRED)
find_package(nav2_behavior_tree REQUIRED)
find_package(pluginlib REQUIRED)

include_directories(include)

add_library(twinguard_bt_nodes SHARED
  src/is_agent_trustworthy.cpp
)

target_include_directories(twinguard_bt_nodes PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(twinguard_bt_nodes
  rclcpp
  geometry_msgs
  behaviortree_cpp_v3
  nav2_behavior_tree
)

add_library(twinguard_nav2_cpp SHARED
  src/integrity_costmap_layer.cpp
)

target_include_directories(twinguard_nav2_cpp PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

ament_target_dependencies(twinguard_nav2_cpp
  rclcpp
  geometry_msgs
  nav2_costmap_2d
  pluginlib
)

pluginlib_export_plugin_description_file(nav2_costmap_2d twinguard_costmap_layer.xml)

install(
  DIRECTORY include/
  DESTINATION include
)

install(
  DIRECTORY params/
  DESTINATION share/${PROJECT_NAME}/params
)

install(
  FILES twinguard_bt_nodes.xml twinguard_costmap_layer.xml
  DESTINATION share/${PROJECT_NAME}
)

install(
  TARGETS twinguard_bt_nodes twinguard_nav2_cpp
  EXPORT export_twinguard_nav2_cpp
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

ament_export_include_directories(include)
ament_export_libraries(twinguard_bt_nodes twinguard_nav2_cpp)
ament_export_targets(export_twinguard_nav2_cpp HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp geometry_msgs behaviortree_cpp_v3 nav2_costmap_2d pluginlib)

ament_package()
