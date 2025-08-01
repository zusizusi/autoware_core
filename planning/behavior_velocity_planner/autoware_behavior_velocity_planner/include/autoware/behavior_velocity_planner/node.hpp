// Copyright 2019 Autoware Foundation
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef AUTOWARE__BEHAVIOR_VELOCITY_PLANNER__NODE_HPP_
#define AUTOWARE__BEHAVIOR_VELOCITY_PLANNER__NODE_HPP_

#include "autoware/behavior_velocity_planner/planner_manager.hpp"

#include <autoware/behavior_velocity_planner_common/planner_data.hpp>
#include <autoware_utils_debug/published_time_publisher.hpp>
#include <autoware_utils_logging/logger_level_configure.hpp>
#include <autoware_utils_rclcpp/polling_subscriber.hpp>
#include <rclcpp/rclcpp.hpp>

#include <autoware_internal_planning_msgs/msg/path_with_lane_id.hpp>
#include <autoware_internal_planning_msgs/msg/velocity_limit.hpp>
#include <autoware_internal_planning_msgs/srv/load_plugin.hpp>
#include <autoware_internal_planning_msgs/srv/unload_plugin.hpp>
#include <autoware_map_msgs/msg/lanelet_map_bin.hpp>
#include <autoware_perception_msgs/msg/predicted_objects.hpp>
#include <autoware_planning_msgs/msg/path.hpp>
#include <nav_msgs/msg/occupancy_grid.hpp>
#include <nav_msgs/msg/odometry.hpp>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <visualization_msgs/msg/marker_array.hpp>

#include <tf2_ros/buffer.h>
#include <tf2_ros/transform_listener.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace autoware::behavior_velocity_planner
{
using autoware_internal_planning_msgs::msg::VelocityLimit;
using autoware_internal_planning_msgs::srv::LoadPlugin;
using autoware_internal_planning_msgs::srv::UnloadPlugin;
using autoware_map_msgs::msg::LaneletMapBin;

class BehaviorVelocityPlannerNode : public rclcpp::Node
{
public:
  explicit BehaviorVelocityPlannerNode(const rclcpp::NodeOptions & node_options);

private:
  // tf
  tf2_ros::Buffer tf_buffer_;
  tf2_ros::TransformListener tf_listener_;

  // subscriber
  rclcpp::Subscription<autoware_internal_planning_msgs::msg::PathWithLaneId>::SharedPtr
    trigger_sub_path_with_lane_id_;

  // polling subscribers
  autoware_utils_rclcpp::InterProcessPollingSubscriber<
    autoware_perception_msgs::msg::PredictedObjects>
    sub_predicted_objects_{this, "~/input/dynamic_objects"};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<sensor_msgs::msg::PointCloud2>
    sub_no_ground_pointcloud_{
      this, "~/input/no_ground_pointcloud", autoware_utils_rclcpp::single_depth_sensor_qos()};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<nav_msgs::msg::Odometry>
    sub_vehicle_odometry_{this, "~/input/vehicle_odometry"};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<
    geometry_msgs::msg::AccelWithCovarianceStamped>
    sub_acceleration_{this, "~/input/accel"};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<
    autoware_perception_msgs::msg::TrafficLightGroupArray>
    sub_traffic_signals_{this, "~/input/traffic_signals"};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<nav_msgs::msg::OccupancyGrid>
    sub_occupancy_grid_{this, "~/input/occupancy_grid"};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<
    LaneletMapBin, autoware_utils_rclcpp::polling_policy::Newest>
    sub_lanelet_map_{this, "~/input/vector_map", rclcpp::QoS{1}.transient_local()};

  autoware_utils_rclcpp::InterProcessPollingSubscriber<VelocityLimit> sub_external_velocity_limit_{
    this, "~/input/external_velocity_limit_mps", rclcpp::QoS{1}.transient_local()};

  void onTrigger(
    const autoware_internal_planning_msgs::msg::PathWithLaneId::ConstSharedPtr input_path_msg);

  void onParam();

  void processNoGroundPointCloud(const sensor_msgs::msg::PointCloud2::ConstSharedPtr msg);
  void processOdometry(const nav_msgs::msg::Odometry::ConstSharedPtr msg);
  void processTrafficSignals(
    const autoware_perception_msgs::msg::TrafficLightGroupArray::ConstSharedPtr msg);
  bool processData(rclcpp::Clock clock);

  // publisher
  rclcpp::Publisher<autoware_planning_msgs::msg::Path>::SharedPtr path_pub_;
  rclcpp::Publisher<visualization_msgs::msg::MarkerArray>::SharedPtr debug_viz_pub_;

  void publishDebugMarker(const autoware_planning_msgs::msg::Path & path);

  //  parameter
  double forward_path_length_;
  double backward_path_length_;
  double behavior_output_path_interval_;

  // member
  PlannerData planner_data_;
  BehaviorVelocityPlannerManager planner_manager_;
  bool is_driving_forward_{true};

  rclcpp::Service<LoadPlugin>::SharedPtr srv_load_plugin_;
  rclcpp::Service<UnloadPlugin>::SharedPtr srv_unload_plugin_;
  void onUnloadPlugin(
    const UnloadPlugin::Request::SharedPtr request,
    const UnloadPlugin::Response::SharedPtr response);
  void onLoadPlugin(
    const LoadPlugin::Request::SharedPtr request, const LoadPlugin::Response::SharedPtr response);

  // mutex for planner_data_
  std::mutex mutex_;

  // function
  bool isDataReady(rclcpp::Clock clock);
  autoware_planning_msgs::msg::Path generatePath(
    const autoware_internal_planning_msgs::msg::PathWithLaneId::ConstSharedPtr input_path_msg,
    const PlannerData & planner_data);

  std::unique_ptr<autoware_utils_logging::LoggerLevelConfigure> logger_configure_;

  std::unique_ptr<autoware_utils_debug::PublishedTimePublisher> published_time_publisher_;

  static constexpr int logger_throttle_interval = 3000;
};
}  // namespace autoware::behavior_velocity_planner

#endif  // AUTOWARE__BEHAVIOR_VELOCITY_PLANNER__NODE_HPP_
