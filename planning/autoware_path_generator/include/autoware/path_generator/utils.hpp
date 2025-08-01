// Copyright 2024 TIER IV, Inc.
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

#ifndef AUTOWARE__PATH_GENERATOR__UTILS_HPP_
#define AUTOWARE__PATH_GENERATOR__UTILS_HPP_

#include "autoware/path_generator/common_structs.hpp"
#include "autoware/trajectory/path_point_with_lane_id.hpp"

#include <autoware_internal_planning_msgs/msg/path_with_lane_id.hpp>
#include <autoware_vehicle_msgs/msg/turn_indicators_command.hpp>

#include <optional>
#include <utility>
#include <vector>

namespace autoware::path_generator
{
using autoware_internal_planning_msgs::msg::PathPointWithLaneId;
using autoware_internal_planning_msgs::msg::PathWithLaneId;
using autoware_vehicle_msgs::msg::TurnIndicatorsCommand;

struct WaypointGroup
{
  struct Waypoint
  {
    lanelet::ConstPoint3d point;
    lanelet::Id lane_id;
  };

  struct Interval
  {
    double start;
    double end;
  };

  std::vector<Waypoint> waypoints;
  Interval interval;
};

template <typename T>
struct PathRange
{
  T left;
  T right;
};

namespace utils
{
/**
 * @brief get lanelets within route that are in specified distance backward from target
 * lanelet
 * @param lanelet target lanelet
 * @param planner_data planner data
 * @param distance backward distance from beginning of target lanelet
 * @return lanelets in range (std::nullopt if target lanelet is not within route)
 */
std::optional<lanelet::ConstLanelets> get_lanelets_within_route_up_to(
  const lanelet::ConstLanelet & lanelet, const PlannerData & planner_data, const double distance);

/**
 * @brief get lanelets within route that are in specified distance forward from target
 * lanelet
 * @param lanelet target lanelet
 * @param planner_data planner data
 * @param distance forward distance from end of target lanelet
 * @return lanelets in range (std::nullopt if target lanelet is not within route)
 */
std::optional<lanelet::ConstLanelets> get_lanelets_within_route_after(
  const lanelet::ConstLanelet & lanelet, const PlannerData & planner_data, const double distance);

/**
 * @brief get previous lanelet within route
 * @param lanelet target lanelet
 * @param planner_data planner data
 * @return lanelets in range (std::nullopt if previous lanelet is not found or not
 * within route)
 */
std::optional<lanelet::ConstLanelet> get_previous_lanelet_within_route(
  const lanelet::ConstLanelet & lanelet, const PlannerData & planner_data);

/**
 * @brief get next lanelet within route
 * @param lanelet target lanelet
 * @param planner_data planner data
 * @return lanelets in range (std::nullopt if next lanelet is not found or not
 * within route)
 */
std::optional<lanelet::ConstLanelet> get_next_lanelet_within_route(
  const lanelet::ConstLanelet & lanelet, const PlannerData & planner_data);

/**
 * @brief get waypoints in lanelet sequence and group them
 * @param lanelet_sequence lanelet sequence
 * @param lanelet_map lanelet map to get waypoints
 * @param connection_gradient_from_centerline gradient for connecting centerline and user-defined
 * waypoints (see figure in README)
 * @return waypoint groups
 */
std::vector<WaypointGroup> get_waypoint_groups(
  const lanelet::LaneletSequence & lanelet_sequence, const lanelet::LaneletMap & lanelet_map,
  const double connection_gradient_from_centerline);

/**
 * @brief get position of first intersection (including self-intersection) in lanelet sequence in
 * arc length
 * @param lanelet_sequence target lanelet sequence
 * @param s_start longitudinal distance of point to start searching for intersections
 * @param s_end longitudinal distance of point to end search
 * @param vehicle_length vehicle length
 * @return longitudinal distance of intersecting point (std::nullopt if no intersection)
 */
std::optional<double> get_first_intersection_arc_length(
  const lanelet::LaneletSequence & lanelet_sequence, const double s_start, const double s_end,
  const double vehicle_length);

/**
 * @brief get position of first self-intersection (point where return
 * path intersects outward path) of line string in arc length
 * @param line_string target line string
 * @return longitudinal distance of self-intersecting point (std::nullopt if no
 * self-intersection)
 */
std::optional<double> get_first_self_intersection_arc_length(
  const lanelet::BasicLineString2d & line_string);

/**
 * @brief get position of given point on centerline projected to path in arc length
 * @param lanelet_sequence lanelet sequence
 * @param path target path
 * @param s_centerline longitudinal distance of point on centerline
 * @return longitudinal distance of projected point
 */
double get_arc_length_on_path(
  const lanelet::LaneletSequence & lanelet_sequence, const std::vector<PathPointWithLaneId> & path,
  const double s_centerline);

/**
 * @brief get path bounds for PathWithLaneId cropped within specified range
 * @param lanelet_sequence lanelet sequence
 * @param s_start longitudinal distance of start of bound on centerline
 * @param s_end longitudinal distance of end of bound on centerline
 * @return cropped bounds (left / right)
 */
PathRange<std::vector<geometry_msgs::msg::Point>> get_path_bounds(
  const lanelet::LaneletSequence & lanelet_sequence, const double s_start, const double s_end);

/**
 * @brief crop line string
 * @param line_string line string
 * @param s_start longitudinal distance to crop from
 * @param s_end longitudinal distance to crop to
 * @return cropped line string
 */
std::vector<geometry_msgs::msg::Point> crop_line_string(
  const std::vector<geometry_msgs::msg::Point> & line_string, const double s_start,
  const double s_end);

/**
 * @brief get positions of given point on centerline projected to left / right bound in arc length
 * @param lanelet_sequence lanelet sequence
 * @param s_centerline longitudinal distance of point on centerline
 * @return longitudinal distance of projected point (left / right)
 */
PathRange<double> get_arc_length_on_bounds(
  const lanelet::LaneletSequence & lanelet_sequence, const double s_centerline);

/**
 * @brief get positions of given point on left / right bound projected to centerline in arc length
 * @param lanelet_sequence lanelet sequence
 * @param s_left_bound longitudinal distance of point on left bound
 * @param s_right_bound longitudinal distance of point on left bound
 * @return longitudinal distance of projected point (left / right)
 */
PathRange<std::optional<double>> get_arc_length_on_centerline(
  const lanelet::LaneletSequence & lanelet_sequence, const std::optional<double> & s_left_bound,
  const std::optional<double> & s_right_bound);

/**
 * @brief Connect the path to the goal, ensuring the path is inside the lanelets.
 * @param path Input path.
 * @param lanelets Lanelets.
 * @param goal_pose Goal pose.
 * @param goal_lane_id Goal lane ID.
 * @param connection_section_length Length of connection section.
 * @param pre_goal_offset Offset for pre-goal.
 * @return A path connected to the goal. (std::nullopt if no valid path found)
 */
std::optional<experimental::trajectory::Trajectory<PathPointWithLaneId>>
connect_path_to_goal_inside_lanelets(
  const experimental::trajectory::Trajectory<PathPointWithLaneId> & path,
  const lanelet::ConstLanelets & lanelets, const geometry_msgs::msg::Pose & goal_pose,
  const lanelet::Id goal_lane_id, const double connection_section_length,
  const double pre_goal_offset);

/**
 * @brief Connect the path to the goal.
 * @param path Input path.
 * @param goal_pose Goal pose.
 * @param goal_lane_id Goal lane ID.
 * @param connection_section_length Length of connection section.
 * @param pre_goal_offset Offset for pre-goal.
 * @return A path connected to the goal.
 */
experimental::trajectory::Trajectory<PathPointWithLaneId> connect_path_to_goal(
  const experimental::trajectory::Trajectory<PathPointWithLaneId> & path,
  const geometry_msgs::msg::Pose & goal_pose, const lanelet::Id goal_lane_id,
  const double connection_section_length, const double pre_goal_offset);

/**
 * @brief Check if the pose is inside the lanelets.
 * @param pose Pose.
 * @param lanelets Lanelets.
 * @return True if the pose is inside the lanelets, false otherwise
 */
bool is_pose_inside_lanelets(
  const geometry_msgs::msg::Pose & pose, const lanelet::ConstLanelets & lanelets);

/**
 * @brief Check if the path is inside the lanelets.
 * @param path Path.
 * @param lanelets Lanelets.
 * @return True if the path is inside the lanelets, false otherwise
 */
bool is_path_inside_lanelets(
  const experimental::trajectory::Trajectory<PathPointWithLaneId> & path,
  const lanelet::ConstLanelets & lanelets);

/**
 * @brief get earliest turn signal based on turn direction specified for lanelets
 * @param path target path
 * @param planner_data planner data
 * @param current_pose current pose of ego vehicle
 * @param current_vel current longitudinal velocity of ego vehicle
 * @param search_distance base search distance
 * @param search_time time to extend search distance
 * @param angle_threshold_deg angle threshold for required end point determination
 * @param base_link_to_front distance from base link to front of ego vehicle
 * @return turn signal
 */
TurnIndicatorsCommand get_turn_signal(
  const PathWithLaneId & path, const PlannerData & planner_data,
  const geometry_msgs::msg::Pose & current_pose, const double current_vel,
  const double search_distance, const double search_time, const double angle_threshold_deg,
  const double base_link_to_front);

/**
 * @brief get required end point for turn signal activation
 * @param lanelet target lanelet
 * @param angle_threshold_deg  yaw angle difference threshold
 * @return required end point
 */

std::optional<lanelet::ConstPoint2d> get_turn_signal_required_end_point(
  const lanelet::ConstLanelet & lanelet, const double angle_threshold_deg);
}  // namespace utils
}  // namespace autoware::path_generator

#endif  // AUTOWARE__PATH_GENERATOR__UTILS_HPP_
